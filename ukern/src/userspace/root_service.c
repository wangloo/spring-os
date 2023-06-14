#include <ramdisk.h>
#include <panic.h>
#include <errno.h>
#include <string.h>
#include <page.h>
#include <uspace/elf.h>
#include <uspace/proc.h>
struct elf_ctx {
	Elf_Addr base_load_vbase;
	Elf_Addr base_load_vend;
	Elf_Addr memsz;
	Elf_Addr align;
	Elf_Ehdr ehdr;
	Elf_Off  dynoff;
	Elf_Addr dynsize;
};

enum {
	EL_OK         = 0,
	EL_EIO,
    	EL_ENOMEM,
    	EL_NOTELF,
    	EL_WRONGBITS,
    	EL_WRONGENDIAN,
    	EL_WRONGARCH,
    	EL_WRONGOS,
    	EL_NOTEXEC,
    	EL_NODYN,
    	EL_BADREL,
};

#define EL_PHOFF(ctx, num) (((ctx)->ehdr.e_phoff + (num) * (ctx)->ehdr.e_phentsize))
#define EL_SHOFF(ctx, num) (((ctx)->ehdr.e_shoff + (num) * (ctx)->ehdr.e_shentsize))

int elf_findphdr(struct ramdisk_file *file, struct elf_ctx *ctx,
		Elf_Phdr *phdr, uint32_t type, unsigned *i)
{
	int rv = EL_OK;

    	for (; *i < ctx->ehdr.e_phnum; (*i)++) {
		rv = ramdisk_read(file, phdr, sizeof(Elf_Phdr), EL_PHOFF(ctx, *i));
		if (rv)
			return rv;
		
		if (phdr->p_type == type)
			return rv;
    	}

    	*i = -1;
    	return rv;
}


int elf_init(struct ramdisk_file *file, struct elf_ctx *ctx)
{
	Elf_Phdr ph;
	int rv = EL_OK;
	unsigned i = 0;

	memset(ctx, 0, sizeof(struct elf_ctx));
	
	if ((rv = ramdisk_read(file, &ctx->ehdr, sizeof(ctx->ehdr), 0)))
		return rv;
	
	if (!IS_ELF(ctx->ehdr))
		return EL_NOTELF;
	
	if (ctx->ehdr.e_ident[EI_CLASS] != ELFCLASS)
	        return EL_WRONGBITS;
	
	if (ctx->ehdr.e_ident[EI_DATA] != ELFDATATHIS)
	        return EL_WRONGENDIAN;
	
	if (ctx->ehdr.e_ident[EI_VERSION] != EV_CURRENT)
	        return EL_NOTELF;
	
	if (ctx->ehdr.e_type != ET_EXEC || ctx->ehdr.e_type == ET_DYN)
	        return EL_NOTEXEC;

	if (ctx->ehdr.e_machine != EM_THIS)
	        return EL_WRONGARCH;
	
	if (ctx->ehdr.e_version != EV_CURRENT)
	        return EL_NOTELF;
	
	/*
	 * calculate how many memory is needed for this elf file, the
	 * memory will allocated together.
	 */
	ctx->base_load_vbase = (unsigned long)-1;

	for(;;) {
		if ((rv = elf_findphdr(file, ctx, &ph, PT_LOAD, &i)))
			return rv;
	
	        if (i == (unsigned) -1)
			break;

		if (ph.p_vaddr < ctx->base_load_vbase)
			ctx->base_load_vbase = ph.p_vaddr;
	
	        Elf_Addr phend = ph.p_vaddr + ph.p_memsz;
	        if (phend > ctx->base_load_vend)
			ctx->base_load_vend = phend;
	
	        if (ph.p_align > ctx->align)
			ctx->align = ph.p_align;
		i++;
	}

	ctx->memsz = align_page_up(ctx->base_load_vend - ctx->base_load_vbase);
	
	return rv;
}


long elf_load_process(struct process *proc, struct ramdisk_file *file)
{
	struct elf_ctx ctx;

	if (elf_init(file, &ctx))
		return -EBADF;
	
	// if (elf_load(proc, file, &ctx))
	// 	return -EIO;

	return ctx.ehdr.e_entry;
}


/*
 * root service will create the first user space
 * process, then response for the memory management
 * for all the task
 */
int load_root_service(void)
{
    int ret;
    long entry;
    struct ramdisk_file file;
    struct process *proc = 0;

    ret = ramdisk_open("roots.elf", &file);
    if (ret) {
        printf("open roots.elf failed, errno: %d\n", ret);
        goto failed;
    }
    
    entry = elf_load_process(proc, &file);
    assert(entry > 0);
    return 0;
failed:
	panic("load root service fail\n");
	return -EFAULT;
}