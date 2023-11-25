struct handle_desc {
	struct kobject *kobj;
	int right;
	int padding;
} __packed;

struct handle_table_desc {
	int index;
	int free;
	struct handle_desc *next;
} __packed;

void 
init_proc_handles(struct proc *proc);
int 
handle_free(handle_t handle, struct kobject **kobj, right_t *right);
int 
get_kobject(handle_t handle, struct kobject **kobj, right_t *right);
int 
put_kobject(struct kobject *kobj);