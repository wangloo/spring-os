#include <asm/asm_macro.S.h>
#include <asm/context.S.h>

  .global load_ectx
func load_ectx
  mov sp, x0
  LOAD_ECTX
  eret
endfunc load_ectx
