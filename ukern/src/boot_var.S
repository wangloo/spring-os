#include <asm/asm_macro.S.h>

/*
 * 本文件中的函数是提供给内核调用，访问boot阶段定义的这些变量中的值
 * ，注意不是返回变量的地址
 */

    .global __get_kernel_end
    .global __get_kernel_stack_top


kfunc __get_kernel_end
    ldr x0, =kernel_end
    ldr x0, [x0]
    ret
endkfunc __get_kernel_end

kfunc __get_kernel_stack_top
    ldr x0, =kernel_stack_top
    ldr x0, [x0]
    ret
endkfunc __get_kernel_stack_top