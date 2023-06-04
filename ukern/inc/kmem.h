#pragma once
#include <page.h>
#include <slab.h>

void *kalloc(size_t size);
void kfree(void *addr);