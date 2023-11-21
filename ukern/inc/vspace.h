
struct vspace {
	struct spinlock lock;
	struct pagetable *pgdp;
	uint16_t asid;
	int refcount;
};
