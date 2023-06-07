#include <asm/asm_macro.S.h>
#include <asm/context.S.h>
#include <config/config.h>

.global __bad_mode
.global __sync_from_lower_el
.global __sync_from_current_el
.global __irq_from_lower_el
.global __irq_from_current_el


func __bad_mode
    b    .
endfunc __bad_mode



func __sync_from_current_el
  SAVE_GP_REGS
  SAVE_OTHER_REGS
  mov x0, sp   // function paramter
  bl sync_from_current_el

  LOAD_GP_REGS
  eret
endfunc __sync_from_current_el




func __sync_from_lower_el
  SAVE_GP_REGS
  mov x0, sp   // function paramter

  LOAD_GP_REGS
  eret
endfunc __sync_from_lower_el




func __irq_from_current_el
  SAVE_GP_REGS
  mov x0, sp
  bl irq_from_current_el
  LOAD_GP_REGS
  eret
endfunc __irq_from_current_el





func __irq_from_lower_el
  SAVE_GP_REGS
  mov x0, sp

  LOAD_GP_REGS
  eret
endfunc __irq_from_lower_el

