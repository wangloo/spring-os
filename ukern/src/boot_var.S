#include <asm/asm_macro.S.h>

.macro DEFINE_GET_LDS_VAR name
  .global __get\name
func __get\name
  ldr x0, =\name
  ret
endfunc __get\name
.endm


DEFINE_GET_LDS_VAR __start_boot_code
DEFINE_GET_LDS_VAR __stop_boot_code
DEFINE_GET_LDS_VAR __start_boot_data
DEFINE_GET_LDS_VAR __kernel_page_table
DEFINE_GET_LDS_VAR __identity_page_table
DEFINE_GET_LDS_VAR __pagetable_base
DEFINE_GET_LDS_VAR __stop_boot_data
DEFINE_GET_LDS_VAR __start_kernel
DEFINE_GET_LDS_VAR __start_code
DEFINE_GET_LDS_VAR __stop_code
DEFINE_GET_LDS_VAR __start_kobject_desc
DEFINE_GET_LDS_VAR __stop_kobject_desc
DEFINE_GET_LDS_VAR __start_data
DEFINE_GET_LDS_VAR __start_bss
DEFINE_GET_LDS_VAR __stop_bss
DEFINE_GET_LDS_VAR __start_rodata
DEFINE_GET_LDS_VAR __stop_kernel



  .global __get_kernel_end
  .global __get_kernel_stack_top
func __get_kernel_end
    ldr x0, =kernel_end
    ldr x0, [x0]
    ret
endfunc __get_kernel_end

func __get_kernel_stack_top
    ldr x0, =kernel_stack_top
    ldr x0, [x0]
    ret
endfunc __get_kernel_stack_top
