struct handle_desc {
	struct kobject *kobj;  // kobject attatched
	int right;             // Right of kobject
	int padding;
} __packed;

struct handle_table_desc {
	int index;  // Handle start number in table
	int free;   
	struct handle_desc *next; // Support for extension
} __packed;

void 
init_proc_handles(struct proc *proc);
handle_t
handle_alloc(struct kobject *kobj, right_t right);
int 
handle_free(handle_t handle, struct kobject **kobj, right_t *right);
int 
get_kobject(handle_t handle, struct kobject **kobj, right_t *right);
int 
put_kobject(struct kobject *kobj);