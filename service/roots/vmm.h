struct vma {
  unsigned long start;
  unsigned long end;
  int perm;        // Permission
  int anon;        // Anonymous Mapping?
  int pma_handle;  // handle of mapped pysical mem area
  struct list_head list;
};

static inline void
vma_init(struct vma *vma, unsigned long start, unsigned long size,
          int perm, int anon)
{
  vma->start = start;
  vma->end = start+size;
  vma->perm = perm;
  vma->anon = anon;
}

int 
vspace_init(struct proc *proc, unsigned long elf_start, unsigned long elf_end);