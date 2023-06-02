#include <asm/asm_macro.S.h>
#include <config/config.h>

.global __bad_mode


func __bad_mode
lb:
    b    lb
endfunc __bad_mode