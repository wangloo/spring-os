#include <config/config.h>
ENTRY(_start)

SECTIONS
{

  . = CONFIG_KERNEL_ENTRY_ADDR;
  bootcode :
  {
    __start_boot_code = .;
    KEEP(*(__start_up))
    KEEP(*(__boot))
    KEEP(*(__asm_code))
    . = ALIGN(4096);
    __stop_boot_code = .;
  }

  bootdata : 
  {
    __start_boot_data = .;
    KEEP(*(__boot_data))

    /* level-0 pagetable */
    . = ALIGN(4096);
    __kernel_page_table = .;
    . = . + 4096;
    __identity_page_table = .;
    . = . + 4096;

    /* 因为 __start_kernel 需要对齐要2M，所以不出意外的话，
       这里会有一片相当大的空洞，可以作为 pagetable pool用 */
    __pagetable_base = .;
    /* 为boot和kernel的页表预留20页应该够了 */
    . = . + 4096 * 20;   
    __stop_boot_data = .;
  }

  . = ALIGN(0x20000);
  . = . + CONFIG_KERNEL_ADDR_MASK;
  __start_kernel = .;
  __start_code = .;
  .text : AT(ADDR(.text)-CONFIG_KERNEL_ADDR_MASK)
  {
    *(.text)
    . = ALIGN(4096);
    KEEP(*(__elx_vectors));
  }
  
  . = ALIGN(4096);
  __stop_code = .;

  __start_initcall = .;
	__init_func_0_start = .;
	.__init_func_0 : {
		*(.__init_func_0)
	}
	__init_func_1_start = .;
	.__init_func_1 : {
		*(.__init_func_1)
	}
	__init_func_2_start = .;
	.__init_func_2 : {
		*(.__init_func_2)
	}
	__init_func_3_start = .;
	.__init_func_3 : {
		*(.__init_func_3)
	}
	__init_func_4_start = .;
	.__init_func_4 : {
		*(.__init_func_4)
	}
	__init_func_5_start = .;
	.__init_func_5 : {
		*(.__init_func_5)
	}
	__init_func_6_start = .;
	.__init_func_6 : {
		*(.__init_func_6)
	}
	__init_func_7_start = .;
	.__init_func_7 : {
		*(.__init_func_7)
	}
	__init_func_8_start = .;
	.__init_func_8 : {
		*(.__init_func_8)
	}
	__init_func_9_start = .;
	.__init_func_9 : {
		*(.__init_func_9)
	}

  . = ALIGN(4096);
  __stop_initcall = .;
  __start_data = .;
  .data : 
  {
    *(.data)

    __start_kobject_desc = .;
    *(.__kobject_desc)
    __stop_kobject_desc = .;
  }

  . = ALIGN(8);
  __start_bss = .;
  .bss : 
  {
    *(.bss)
  }
  __stop_bss = .;

  . = ALIGN(4096);
  __stop_data = .;

  __start_rodata = .;
  .rodata : 
  {
    KEEP(*(.rodata))
  }

  __stop_kernel = .;
}
