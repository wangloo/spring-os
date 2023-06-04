#include <asm/arm64_default.h>
#include <config/config.h>

.global _start
.section __start_up, "ax"

.macro clear_ttbr0
    //msr     ttbr0_el1, xzr
    //tlbi    vmalle1
.endm

_start:
  bl     current_core
  cbz    x0, _primary

_secondary:
  wfi    // wait for being waken up

_primary:
  mrs    x0, CurrentEL
  mov    x1, x0, lsr #2
  and    x1, x1, #3

  // only support booting on EL1
  cmp    x1, #3
  b.eq   boot_panic
  cmp    x1, #2
  b.eq   boot_panic

_on_el1:
  adr    x1, _start  // entry address must 4k align
  and    x0, x1, #0xffff
  cbnz   x0, boot_panic

  // kernel_start to __code_start is the reserve memory
  // for minos
  ldr     x3, =0xffffffffffe00000
  and     x1, x1, x3
  adr     x2, kernel_start
  str     x1, [x2]


  /* using current EL stack register */
  msr    spsel, #1
  dsb    nsh
  isb

  /* setup exception vector */
  ldr    x1, =elx_vectors
  msr    vbar_el1, x1

  /* enable Abort now for early system abort */
  msr    daifclr, #4
  dsb    nsh
  isb

  /* invalid the dcache and flush the tlb */
  bl     inv_dcache_all
  dsb    sy
  isb

  ldr    x1, =ARM64_SCTLR_VALUE
  msr    sctlr_el1, x1
  dsb    nsh
  isb

  ldr    x1, =ARM64_MAIR_VALUE
  msr    mair_el1, x1

  ldr    x1, =ARM64_TCR_VALUE
  msr    tcr_el1, x1            // set the TCR_ELx.
  dsb    sy
  isb

  // disable EL1 FPU traps.
  // otherwise, accessing register qx will cause trap.
  ldr    x1, =ARM64_CPACR_VALUE
  msr    cpacr_el1, x1

  mov    x1, #ARM64_SPSR_VALUE
  msr    spsr_el1, x1
  dsb    nsh

  ldr    x0, =__stop_kernel
  adr    x1, kernel_stack_top
  adr    x2, kernel_bootmem_base
  adr    x3, kernel_stack_bottom

  // store the bootmem base
  // [x2] = align_up(x0, 4096)
  add    x0, x0, #4095
  mov    x4, #4095
  mvn    x5, x4
  and    x0, x0, x5
  str    x0, [x2]

  // store the minos stack bottom
  str    x0, [x3]

  // store the minos stack top
  mov    x4, #CONFIG_TASK_STACK_SIZE
  mov    x5, #CONFIG_NR_CPUS
  mul    x5, x4, x5
  add    x0, x0, x5
  str    x0, [x1]

  sub    x0, x0, x19, lsl #CONFIG_TASK_STACK_SHIFT
  mov    sp, x0

  // clean kernel bss
  ldr    x0, =__start_bss
  mov    x1, #0
  ldr    x2, =__stop_bss
  sub    x2, x2, x0
  bl     memset

  // map the boot memory when booting
  bl     map_boot_mem
  dsb    ishst
  isb



  /* setup the el1 page table 
   TODO: why ttbr 0? */
  ldr    x1, = __kernel_page_table
  msr    ttbr0_el1, x1
  msr    ttbr1_el1, x1
  dsb    nsh
  isb

dbg_map_boot_mem:

  ldr    x26, =mmu_on

  // enable the mmu and disable the aligment check
  mrs    x1, sctlr_el1
  orr    x1, x1, #SCTLR_ELx_M
  orr    x1, x1, #SCTLR_ELx_C
  orr    x1, x1, #SCTLR_ELx_I
  bic    x1, x1, #SCTLR_ELx_SA
  bic    x1, x1, #SCTLR_ELx_A
  msr    sctlr_el1, x1
  dsb    sy
  isb

  br     x26

mmu_on:
  ic     ialluis
  dsb    sy
  isb

  // clear_ttbr0
  dsb    sy
  isb

  bl    kernel_init    // never return.
dead_loop:
  b     dead_loop

boot_panic:
  nop
  b     boot_panic


/*
 * 相当于定义五个变量, 存储一些地址
 */
  .data
  .global kernel_start
  .global kernel_bootmem_base
  .global kernel_stack_top
  .global kernel_stack_bottom
  .global kernel_end
  .balignl 16, 0xdeadbeef

kernel_start:         .quad  0x0
kernel_bootmem_base:  .quad  0x0
kernel_stack_top:     .quad  0x0
kernel_stack_bottom:  .quad  0x0
kernel_end:           .quad  0x0

/*
  ------------------------------- kernel_start /
                                  __start_kernel

        .text, .data, .bss
        .rodata, ...

  ------------------------------- __stop_kernel
        ALIGN(4K)
  ------------------------------- kernel_bootmem_base /
                                  kernel_stack_bottom
           STACK

  ------------------------------- kernel_stack_top

        BOOT PAGETABLEs  
    (uncertain size, 4K aligned)

  ------------------------------- kernel_end

        KERNEL FREE MEM

  ------------------------------- CONFIG_KERNEL_RAM_SIZE


 */
