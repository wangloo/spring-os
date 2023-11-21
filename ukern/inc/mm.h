#pragma once


extern unsigned long kernel_start;
extern unsigned long kernel_bootmem_base;
extern unsigned long kernel_stack_top;
extern unsigned long kernel_stack_bottom;
extern unsigned long kernel_end;


// This not work!!
// Because use `adr` instruction to load them in C,
// But across kernel space and userspace
// So adr is limited, ldr peseudo is work
// extern unsigned long __start_boot_code;
// extern unsigned long __stop_boot_code;
// extern unsigned long __start_boot_data;
// extern unsigned long __kernel_page_table;
// extern unsigned long __identity_page_table;
// extern unsigned long __pagetable_base;
// extern unsigned long __stop_boot_data;
// extern unsigned long __start_kernel;
// extern unsigned long __start_code;
// extern unsigned long __stop_code;
// extern unsigned long __start_kobject_desc;
// extern unsigned long __stop_kobject_desc;
// extern unsigned long __start_data;
// extern unsigned long __start_bss;
// extern unsigned long __stop_bss;
// extern unsigned long __start_rodata;
// extern unsigned long __stop_kernel;


void 
init_mm(void);