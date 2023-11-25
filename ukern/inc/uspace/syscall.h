// syscall func

extern int 
sys_kobject_connect(char __user *path, right_t right);
extern int 
sys_kobject_close(int handle);
extern int 
sys_kobject_open(handle_t handle);
extern handle_t 
sys_kobject_create(int type, unsigned long data);
extern ssize_t 
sys_kobject_recv(handle_t handle, void __user *data, size_t data_size,
                  size_t *actual_data, void __user *extra, size_t extra_size,
                  size_t *actual_extra, uint32_t timeout);
extern ssize_t 
sys_kobject_send(handle_t handle, void __user *data, size_t data_size,
                  void __user *extra, size_t extra_size, uint32_t timeout);
extern int 
sys_kobject_reply(handle_t handle, long token,
                    long err_code, handle_t fd, right_t fd_right);
extern int 
sys_kobject_munmap(handle_t handle);
extern int 
sys_kobject_mmap(handle_t handle, void **addr, unsigned long *msize);
extern long 
sys_kobject_ctl(handle_t handle, int req, unsigned long data);