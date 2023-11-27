
struct proc {
  struct list_head vma_free;
  struct list_head vma_used;


  // heap area
  unsigned long brk_end;
  unsigned long brk_start;
  unsigned long brk_cur;

  struct list_head list;
};