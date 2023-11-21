#include <asm/asm_macro.S.h>

/*
 * Fill in the buffer with character c (alignment handled by the hardware)
 *
 * Parameters:
 *    x0 - buf
 *    x1 - c
 *    x2 - n
 * Returns:
 *    x0 - buf
 */
    .global memset_asm

asmfunc memset_asm
  mov    x4, x0
  and    w1, w1, #0xff
  orr    w1, w1, w1, lsl #8
  orr    w1, w1, w1, lsl #16
  orr    x1, x1, x1, lsl #32
  subs   x2, x2, #8
  b.mi   2f
1:
  str    x1, [x4], #8
  subs   x2, x2, #8
  b.pl   1b
2:    
  adds   x2, x2, #4
  b.mi   3f
  sub    x2, x2, #4
  str    w1, [x4], #4
3:    
  adds   x2, x2, #2
  b.mi   4f
  sub    x2, x2, #2
  strh   w1, [x4], #2
4:    
  adds   x2, x2, #1
  b.mi   5f
  strb   w1, [x4]
5:    
  ret
endfunc memset_asm



.global current_core

asmfunc current_core
  mrs x0, mpidr_el1
  and x0, x0, #0xFF
  ret
endfunc current_core
