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
  adr    x4, kernel_end

  // store the bootmem base
  // [x2] = align_up(x0, 4096)
  add    x0, x0, #4095
  mov    x5, #4095
  mvn    x6, x5
  and    x0, x0, x6
  str    x0, [x2]

  // store the minos stack bottom
  str    x0, [x3]

  // store the minos stack top
  mov    x5, #CONFIG_TASK_STACK_SIZE
  mov    x6, #CONFIG_NR_CPUS
  mul    x6, x5, x6
  add    x0, x0, x6
  str    x0, [x1]

  /* kernel_end == stack top, no other data */
  str    x0, [x4]

  sub    x0, x0, x19, lsl #CONFIG_TASK_STACK_SHIFT
  mov    sp, x0

  // map boot and kernel memory
  bl     prepare_map
  bl     do_map_id
  bl     do_map_kern
  dsb    ishst
  isb

  ldr    x1, = __kernel_page_table
  ldr    x2, = __identity_page_table
  msr    ttbr0_el1, x2
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

  /* setup exception vector */
  ldr    x1, =elx_vectors
  msr    vbar_el1, x1

  /* enable Abort now for early system abort */
  msr    daifclr, #4
  dsb    nsh
  isb


  // clean kernel bss
  ldr    x0, =__start_bss
  mov    x1, #0
  ldr    x2, =__stop_bss
  sub    x2, x2, x0
  bl     memset

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
  .section __boot_data, "aw"
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
  是时候对整个系统的内存规划做一个描述了， 不然天天看到这些之前定义的变量头痛。

  (1) 从链接脚本开始说起，__start_kernel是一个分水岭，之前的代码都属于boot阶段，
  之后的代码都属于真正的kernel。它们之间的区别是boot阶段的代码的LMA==VMA，也就是
  在disable MMU的情况下能够直接访问（当然只要你编译的代码没有绝对的地址访问，
  即便VMA!=LMA的情况也能正常的执行代码，他会顺序的从LMA开始执行，然后使用相对地址访问
  来做跳转等指令， 很多系统就是这么做的，比如minos。所以此时你有了uboot后，你想加载到
  哪里开始执行都可以，只是这样做不好调试因为符号信息是错误的。），而kernel的VMA是高地址的
  ，MASK为0xffff000000000000. 所以造成的情况是: boot和kernel的代码LMA连续，但是VMA
  相隔千万里。

  (2) 再从启动开始说起。启动后，-kernel下，qemu会执行boot段里的代码，这很顺利。
  直到启用MMU之前（别问我为什么要启用MMU），启用MMU也是在boot阶段做的事情，所以
  之前必须准备好恒等映射的页表，做好开启MMU时的过渡。
  
  (3) 准备好页表。boot阶段需要准备好两张页表，一张是boot时使用的自己的页表
  (不然开启MMU之后的boot指令都没法访问了），另一张是kernel的页表，因为boot最终是
  要bl到kernel中去的，而kernel又在高地址，ARMv8规定高地址和低地址使用两张不同的页表
  所以一共需要两张页表。 系统采用的是四级页表的方式，boot和kernel的顶级页表均在链接
  脚本中预留的空间，请看__kernel_page_table和__identity_page_table。

  (4) 其他级别的页表池。 四级页表，链接脚本中预留的4K显然不够，而另外页表的申请是从
  __pagetable_base 地址开始预留的。因为反正__start_kerenl是2M对齐，boot code
  和data又用不了那么多，中间一定存在20页的空洞来放pagetable的池子。之后就开始建立映射
  ，map_id和map_kern分别是建立boot段页表和内核页表，映射的详情可以见这两个函数的注释
  ，在这里就不多介绍了。
  (5) 映射的过程中，与地址规划相关的几个变量需要着重介绍，在 boot data 段中共定义了
  五个重要的变量：
  * kernel_start
  * kernel_bootmem_base
  * kernel_stack_top
  * kernel_stack_bottom
  * kernel_end
  它们存储的值都非常有用，规定了一些范围界限，以至于后面kernel启动的时候也需要用到
  这些界限值，下面我们来解释下它们中都存了什么值。
  * kernel_start: 将__start_code对齐，对齐到2M，这样映射的时候比较方便，
    但是目前我们链接脚本中定义的起始LMA/VMA也都是2M对齐的。
    FIXME: 这个起名错误啊，或者说不应该从__start_code开始！
  * kernel_bootmem_base: 虚拟地址，以__stop_kerenl为基准，向上对齐到4K，
    到kernel_end为止，里面包含的内容是boot期间设置的kernel内存，目前包括
    kernel stack。另外从这之后开始的内存都属于 normal memory，映射的时候
    可以使用一个属性，所以之后尽量映射为2M block。
  * kernel_stack_bottom: 虚拟地址，kernel stack底，kernel stack是挨着
    kernel_bootmem_base的。
  * kernel_stack_top: 虚拟地址，bottom和top之间的空间就是内核的栈，为每个
    CPU 分配的单独的。
  * kernel_end: 虚拟地址，这是在建立映射之后赋值的，从kernel_stack_top
    之后先是boot页表和kernel页表的其他级页表用的池子，然后 kernel_end
    就标定了这个池子用到了哪里。 从kernel_end之后的内存就是由内核来管理了，
    完全的自由，内核一共可以掌管 CONFIG_KERNEL_RAM_SIZE 这么多内存。
  

  (5) boot阶段的结束。开启MMU后，其实就可以进入kernel的世界了，kernel
  管理的normal内存从 kernel_end 开始，这些内存可以用来建立分配器，进一步
  的划分就是之后的事情了， 这里暂时就介绍到这。



  各个变量在地址上的视角：

  ------------------------------- kernel_start / __start_boot_code
        BOOT CODE
  ------------------------------- __stop_boot_code
        ALIGN(4K)        
  ------------------------------- __start_boot_data
        BOOT DATA
  ------------------------------- __stop_boot_data
        ALIGN(2M)               

  ------------------------------- __start_kernel

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
