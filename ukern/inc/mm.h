#pragma once


extern unsigned long kernel_start;
extern unsigned long kernel_bootmem_base;
extern unsigned long kernel_stack_top;
extern unsigned long kernel_stack_bottom;
extern unsigned long kernel_end;



void mm_init(void);