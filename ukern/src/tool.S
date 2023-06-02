#include <asm/asm_macro.S.h>

.global current_core

func current_core
  mrs x0, mpidr_el1
  and x0, x0, #0xFF
  ret
endfunc current_core
