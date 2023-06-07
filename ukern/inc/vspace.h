#pragma once
#include <pagetable.h>
#include <types.h>

struct vspace {
	page_table_t *pgdp;
	// spinlock_t lock;
	uint16_t asid;
};