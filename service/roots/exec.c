
#include <stdio.h>
#include <string.h>
#include <elf.h>

#include <minos/service.h>
#include <minos/types.h>
#include <minos/utils.h>
#include <minos/page.h>
#include <minos/list.h>
#include <minos/debug.h>
#include <minos/kobject_uapi.h>
#include <ramdisk.h>
#include <proc.h>
#include <vmm.h>
#include <halloc.h>

int
exec(char *path, char **argv)
{
  Elf64_Ehdr elf;
  Elf64_Phdr ph;
  struct ramdisk_file file;
  int i, off;
  unsigned long elf_load_vbase = (unsigned long)-1;
  unsigned long elf_load_vend = 0;
  unsigned long elf_load_align = 0;
  unsigned long elf_map_size;

  if (!argv || !argv[0])
    return -1; 

  if (ramdisk_open(path, &file) < 0) {
    return -1;
  }

  if (ramdisk_read(&file, &elf, sizeof(elf), 0) != sizeof(elf)) {
    goto bad;
  }
  
  if (elf.e_ident[0] != ELFMAG0 || elf.e_ident[1] != ELFMAG1 ||
      elf.e_ident[2] != ELFMAG2 || elf.e_ident[3] != ELFMAG3) {
    pr_err("ELF format error\n");
    goto bad;
  }

  // load and map all loadable segments
  for (i = 0, off = elf.e_phoff; i < elf.e_phnum; i++, off += sizeof(ph)) {
    if (ramdisk_read(&file, &ph, sizeof(ph), off) != sizeof(ph))
      goto bad;
    if (ph.p_type != PT_LOAD)
      continue;
    if (ph.p_memsz < ph.p_filesz)
      goto bad;
    if (ph.p_vaddr + ph.p_memsz < ph.p_vaddr)
      goto bad;

    if (ph.p_vaddr < elf_load_vbase)
      elf_load_vbase = ph.p_vaddr;
    if (ph.p_vaddr+ph.p_memsz > elf_load_vend)
      elf_load_vend = ph.p_vaddr+ph.p_memsz;
    if (ph.p_align > elf_load_align)
      elf_load_align = ph.p_align;
      

    // u64 a = pgtbl_walk(p->pagetable, ph.vaddr);
    // if (ramdisk_read(&file, (void *)ptov(a), ph.filesz, ph.off) < 0) {
    //   goto bad;
    // }
  }

  elf_map_size = align_page_up(elf_load_vend-elf_load_vbase);
  pr_debug("elf_load_vbase: 0x%lx\n", elf_load_vbase);
  pr_debug("elf_load_vend : 0x%lx\n", elf_load_vend);
  pr_debug("elf_map_size : 0x%lx\n", elf_map_size);

  // Create process control block
  struct proc *p;
  if ((p = create_new_proc("sh", NULL, elf.e_entry)) == NULL) {
    goto bad;
  }

  if (vspace_init(p, elf_load_vbase, elf_map_size) < 0) {
    goto bad;
  }
  

  // Allocate memory for all loadable segments
  // And attach it to new process
  struct vma *vma, *vma_roots;


  // vma = halloc(sizeof(*vma));
  // vma_init(vma, elf_load_vbase, elf_map_size, KOBJ_RIGHT_RWX, 0);
  

  // Create mapping with same pma for roots
  // Read segments to it





  return 0;

bad:
  return -1;
}