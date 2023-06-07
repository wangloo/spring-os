#include <asm/asm_macro.S.h>
#include <config/config.h>

.macro vector_def label
  .align	7
  b \label
.endm

/* Exception handler entries */
sync_cursp0:
  b   __bad_mode
irq_cursp0:
  b   __bad_mode
fiq_cursp0:
  b   __bad_mode
error_cursp0:
  b   __bad_mode

sync_curspx:
  b   __sync_from_current_el
irq_curspx:
  b   __irq_from_current_el
fiq_curspx:
  b   __bad_mode
error_curspx:
  b   __bad_mode

sync_low64:
  b   __sync_from_lower_el 
irq_low64:
  b   __irq_from_lower_el
fiq_low64:
  b   __bad_mode
error_low64:
  b   __bad_mode

sync_low32:
  b   __bad_mode
irq_low32:
  b   __bad_mode
fiq_low32:
  b   __bad_mode
error_low32:
  b   __bad_mode




.global elx_vectors
.section __elx_vectors, "ax"
.align 11
elx_vectors:
  vector_def    sync_cursp0
  vector_def    irq_cursp0
  vector_def    fiq_cursp0
  vector_def    error_cursp0

  vector_def    sync_curspx
  vector_def    irq_curspx
  vector_def    fiq_curspx
  vector_def    error_curspx
  
  vector_def    sync_low64
  vector_def    irq_low64
  vector_def    fiq_low64
  vector_def    error_low64

  vector_def    sync_low32
  vector_def    irq_low32
  vector_def    fiq_low32
  vector_def    error_low32
