struct vma {
  unsigned long start;
  unsigned long end;
  struct list_head list;
  int perm; // Permission
  int anon; // Anonymous Mapping?
};