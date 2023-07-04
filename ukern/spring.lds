#include <config/config.h>
ENTRY(_start)

SECTIONS
{

  . = CONFIG_KERNEL_ENTRY_ADDR+CONFIG_KERNEL_ADDR_MASK;
  bootcode :AT(CONFIG_KERNEL_ENTRY_ADDR)
  {
    __start_boot_code = .;
    KEEP(*(__start_up))
    KEEP(*(__boot))
    KEEP(*(__asm_code))
    KEEP(*(__elx_vectors));
    . = ALIGN(4096);
    __stop_boot_code = .;
  }

  bootdata : 
  {
    __start_boot_data = .;
    KEEP(*(__boot_data))
    __stop_boot_data = .;
  }


  __start_code = .;
  .text : 
  {
    *(.text)
  }
  
  . = ALIGN(4096);
  __stop_code = .;

  . = ALIGN(4096);
  __start_data = .;
  .data : 
  {
    /* kerenl page table */
    . = ALIGN(4096);
    __kernel_page_table = .;
    . = . + 4096;

    *(.data)
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
