#include <kernel.h>
#include <memlayout.h>
#include <proc.h>
#include <ramdisk.h>
#include <page.h>
#include <kmem.h>
#include <pagetable.h>
#include <elf.h>
#include <rootserv.h>


/* Symbolic values for the entries in the auxiliary table
   put on the initial stack */
#define AT_NULL   0     /* end of vector */
#define AT_IGNORE 1     /* entry should be ignored */
#define AT_EXECFD 2     /* file descriptor of program */
#define AT_PHDR   3     /* program headers for program */
#define AT_PHENT  4     /* size of program header entry */
#define AT_PHNUM  5     /* number of program headers */
#define AT_PAGESZ 6     /* system page size */
#define AT_BASE   7     /* base address of interpreter */
#define AT_FLAGS  8     /* flags */
#define AT_ENTRY  9     /* entry point of program */
#define AT_NOTELF 10    /* program is not ELF */
#define AT_UID    11    /* real uid */
#define AT_EUID   12    /* effective uid */
#define AT_GID    13    /* real gid */
#define AT_EGID   14    /* effective gid */
#define AT_PLATFORM 15  /* string identifying CPU for optimizations */
#define AT_HWCAP  16    /* arch dependent hints at CPU capabilities */
#define AT_CLKTCK 17    /* frequency at which times() increments */
/* AT_* values 18 through 22 are reserved */
#define AT_SECURE 23   /* secure mode boolean */
#define AT_BASE_PLATFORM 24     /* string identifying real platform, may
                                 * differ from AT_PLATFORM. */
#define AT_RANDOM 25    /* address of 16 random bytes */
#define AT_HWCAP2 26    /* extension of AT_HWCAP */

#define AT_EXECFN  31   /* filename of program */
/* dedicated for minos */
#define AT_SERVER_HANDLE 61/* for minos talk to root service */
#define AT_HEAP_BASE 62
#define AT_HEAP_END 63
#define AT_RAMDISK_BASE 64
#define AT_RAMDISK_END  65

typedef struct
{
	uint64_t a_type;              /* Entry type */
	uint64_t a_val;
} Elf64_auxv_t;

#define NEW_AUX_ENT(auxp, type, value)	\
	do {				\
		auxp--;			\
		auxp->a_type = type;	\
		auxp->a_val = value;	\
	} while (0)

static void *setup_auxv(void *va)
{
	Elf64_auxv_t *auxp = (Elf64_auxv_t *)va;

	NEW_AUX_ENT(auxp, AT_NULL, 0);
	NEW_AUX_ENT(auxp, AT_PAGESZ, PAGE_SIZE);
	NEW_AUX_ENT(auxp, AT_HWCAP, 0);		// TBD cpu feature.

	 // root service will have it own memory management
	 // service, kernel will handle the page fault for it.
	NEW_AUX_ENT(auxp, AT_HEAP_BASE, SYS_PROC_HEAP_BASE);
	NEW_AUX_ENT(auxp, AT_HEAP_END, SYS_PROC_HEAP_END);

  // root service load other service from ramdisk
  NEW_AUX_ENT(auxp, AT_RAMDISK_BASE, SYS_PROC_RAMDISK_BASE);
  NEW_AUX_ENT(auxp, AT_RAMDISK_END, SYS_PROC_RAMDISK_END);

	return (void *)auxp;
}

static void *
setup_argv(void *va, int argc, void *argvp)
{
    char **p = (char **)va;

    p -= (argc+1);
    memcpy(p, argvp, (argc+1)*sizeof(*p));
    *((uint64_t *)--p) = argc;
    return p;
}

static void *
setup_envp(void *va)
{
	va -= sizeof(unsigned long);
	*(char **)va = NULL;

	return va;
}





int
exec(char *path, char **argv)
{
    int i, off;
    u64 pa, mapva, mapsz, sp, size, stackbase, stacksize;
    struct ramdisk_file file;
    struct elfhdr elf;
    struct proghdr ph;
    struct proc *p = cur_proc();

    if (!argv || !argv[0]) {
        return -1;
    }
    

    if (ramdisk_open(path, &file) < 0) {
        return -1;
    }
    if (ramdisk_read(&file, &elf, sizeof(elf), 0) != sizeof(elf)) {
        goto bad;
    }
    if (elf.magic != ELF_MAGIC)
        goto bad;

    // load and map all loadable segments
    for (i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)) {
        if (ramdisk_read(&file, &ph, sizeof(ph), off) != sizeof(ph)) 
            goto bad;
        if (ph.type != ELF_PROG_LOAD)
            continue;
        if (ph.memsz < ph.filesz)
            goto bad;
        if (ph.vaddr + ph.memsz < ph.vaddr)
            goto bad;
        
        mapva = align_page_down(ph.vaddr);
        mapsz = align_page_up(ph.memsz + (ph.vaddr-mapva));
        pa = vtop(kalloc(mapsz));
        // LOG_DEBUG("va: 0x%lx, pa: 0x%lx, size: 0x%lx\n", mapva, pa, mapsz);

        if (pagetable_map(p->pagetable, mapva, pa, mapsz, VM_RWX) < 0) {
            kfree((void *)ptov(pa));
            goto bad;
        }
        u64 a = pgtbl_walk(p->pagetable, ph.vaddr);
        if (ramdisk_read(&file, (void *)ptov(a), ph.filesz, ph.off) < 0) {
            goto bad;
        }
    }

    pa = (u64)vtop(page_allocn(2));
    sp = 0xF0002000;
    stacksize = 2*PAGE_SIZE;
    stackbase = sp - stacksize;
    if (pagetable_map(p->pagetable, stackbase, pa, stacksize, VM_RW)) {
        kfree((void *)ptov(pa));
        goto bad;
    }

    size = SYS_PROC_HEAP_SIZE;
    pa = (u64)vtop(kalloc(align_page_up(size)));
    if (pagetable_map(p->pagetable, SYS_PROC_HEAP_BASE, pa, size, VM_RW)) {
        kfree((void *)ptov(pa));
        goto bad;
    }
    
    size = RAMDISK_SIZE;
    pa = RAMDISK_BASE;
    if (pagetable_map(p->pagetable, SYS_PROC_RAMDISK_BASE, pa, size, VM_RW)) {
        kfree((void *)ptov(pa));
        goto bad;
    }


    // Put argc,argv,envp.. to user stack
    // C Runtime will decode them
    // So sp_el0 is lower than 0xF0002000
    void *va = (void *)(ptov(pgtbl_walk(p->pagetable, (vaddr_t)stackbase) + stacksize));
    void *origin = va;
    void *argvp[MAXARG+1];
    int argc;

    for (argc = 0; argv[argc]; argc++) {
        size = strlen(argv[argc])+1;
        va -= size; 
        memcpy(va, argv[argc], size+1); // Add '\0'
        argvp[argc] = (void *)sp - (origin-va);
    }    
    argvp[argc] = 0;

    va = (void *)align_down((u64)va, 8);
    va = setup_auxv(va);
    va = setup_envp(va);
    va = setup_argv(va, argc, argvp);
    proc_set_context(p, (void *)elf.entry, sp - (origin-va));


    proc_ready(p);
    LOG_DEBUG("NEW PROCESS!!\n");
    LOG_DEBUG("name: %s\n",p->name);
    LOG_DEBUG("kstack base: 0x%lx\n",p->stack_base);
    LOG_DEBUG("ctx->spel0: 0x%lx\n",proc_ectx(p)->ctx.sp0);
    LOG_DEBUG("ctx->elr: 0x%lx\n",proc_ectx(p)->ctx.elr);
    LOG_DEBUG("ctx->spsr: 0x%lx\n",proc_ectx(p)->ctx.spsr);
    LOG_DEBUG("sizeof(struct econtext): 0x%x\n", sizeof(struct econtext));
    LOG_DEBUG("addr of ctx: %p\n", proc_ectx(p));
    LOG_DEBUG("addr of ctx->sp_el0: %p\n", &(proc_ectx(p)->ctx.sp0));
    LOG_DEBUG("addr of ctx->elr: %p\n", &(proc_ectx(p)->ctx.elr));
    LOG_DEBUG("addr of ctx->spsr: %p\n", &(proc_ectx(p)->ctx.spsr));
    return 0;
bad:
    return -1;
}