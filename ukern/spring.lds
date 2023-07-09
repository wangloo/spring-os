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
    KEEP(*(__elx_vectors));
    . = ALIGN(4096);
    __stop_boot_code = .;
  }

  bootdata : 
  {
    __start_boot_data = .;

    /* kerenl page table */
    . = ALIGN(4096);
    __kernel_page_table = .;
    . = . + 4096;
    __identity_page_table = .;
    . = . + 4096;
    KEEP(*(__boot_data))
    __stop_boot_data = .;
  }

  . = ALIGN(0x20000);
  . = . + CONFIG_KERNEL_ADDR_MASK;
  __start_code = .;
  .text : AT(ADDR(.text)-CONFIG_KERNEL_ADDR_MASK)
  {
    *(.text)
  }
  
  . = ALIGN(4096);
  __stop_code = .;

  . = ALIGN(4096);
  __start_data = .;
  .data : 
  {
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
