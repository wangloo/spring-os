
struct proc {
  int handle;   // Used to control the process
  struct list_head vma_free;
  struct list_head vma_used;


  // heap area
  unsigned long brk_end;
  unsigned long brk_start;
  unsigned long brk_cur;

  struct list_head list;
};


struct proc *
create_new_proc(char *name, struct proc *parent, unsigned long entry);