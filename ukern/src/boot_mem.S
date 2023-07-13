#include <stage1.h>
#include <config/config.h>

    /*
     * map the code memory VA->PA, if need to using
     * dcache need to enable the MMU, first clear
     * the page table, below is the var defination
     * Note, this function will call memset, and memset
     * will use x0 - x4, so need pay attention for x0-x4
     * if there some value saved in these register
     */
page_table     .req    x24
ttb0_pgd       .req    x20
ttb0_pud       .req    x21
ttb0_pmd       .req    x22
ttb0_pt        .req    x27
 
vaddr          .req    x5
paddr          .req    x6
size           .req    x7
pte_attr       .req    x8
 
tmp_const      .req    x9
pud_tmp        .req    x10
pmd_tmp        .req    x12
tmp            .req    x13
pte_value      .req    x14
pte_index      .req    x15
entry_size     .req    x16
entry_align    .req    x17
entry_mask     .req    x18
pagetable_base .req    x23        // store the pagetable for page allocation

.section __asm_code, "ax"
.balign 4

#define HOST_TABLE_DES    (S1_DES_TABLE | S1_TABLE_UAP | S1_TABLE_UXN)
#define HOST_1G_BLK_ATTR  (S1_DES_BLOCK | S1_ATTR_IDX(MT_NORMAL))

    .global prepare_map
    .global do_map_kern
    .global do_map_id


/**
 * @brief 建立恒等映射页表和内核页表之前的准备
 *
 */
prepare_map:
    /* get the base address of pagetable pool */
    ldr    pagetable_base, =__pagetable_base

    ret

/**
 * @brief 内核页表映射的建立
 *
 * 建立的映射包括：
 * 1. kernel的代码段、数据段、bss段等
 * 2. boot data段，因为要访问内核页表和页表池
 * 3. 串口
 * 4. bootmem_base 之后的内存会以2M的大块映射，但是bootmem_base
 *    不一定是2M对齐的，于是中间的4K也是在这时被填充上映射
 */
do_map_kern:
    /* save the lr register */
    mov    x26, x30

    /* kernel pgd is defined in lds */
    mov    x1, #0
    mov    x2, #4096
    ldr    page_table, = __kernel_page_table
    mov    x0, page_table
    bl     memset

    // 预留0-4G的页表(只填了2M block), 其他的页表项需要自己填
    bl     build_page_table

    // map the code section rx
__map_kcode_section:
    ldr    vaddr, =__start_code
    ldr    tmp, =__stop_code
    sub    size, tmp, vaddr
    ldr    tmp, =CONFIG_KERNEL_ADDR_MASK // tmp reg is free now!
    sub    vaddr, vaddr, tmp
    mov    paddr, vaddr
    ldr    pte_attr, =BOOTMEM_CODE_ATTR
    bl     build_normal_pte_table

    // map the data section
__map_kdata_section:
    ldr    vaddr, =__start_data
    ldr    tmp, =__stop_data
    sub    size, tmp, vaddr
    ldr    tmp, =CONFIG_KERNEL_ADDR_MASK // tmp reg is free now!
    sub    vaddr, vaddr, tmp
    mov    paddr, vaddr
    ldr    pte_attr, =BOOTMEM_DATA_ATTR
    bl     build_normal_pte_table

    // map the RO data section like symbol and string
    // RO section is the last section in the dts, so the
    // the range is from __rodata_start to kernel_bootmem_base
__map_krodata_section:
    ldr    vaddr, =__start_rodata
    adr    tmp, kernel_bootmem_base
    ldr    tmp, [tmp]
    sub    size, tmp, vaddr
    ldr    tmp, =CONFIG_KERNEL_ADDR_MASK // tmp reg is free now!
    sub    vaddr, vaddr, tmp
    mov    paddr, vaddr
    ldr    pte_attr, =BOOTMEM_DATA_RO_ATTR
    bl     build_normal_pte_table

    // map the boot data, 主要是为了访问内核的页表和页表池
    ldr    vaddr, =__start_boot_data
    ldr    tmp, =__stop_boot_data
    sub    size, tmp, vaddr
    mov    paddr, vaddr
    ldr    pte_attr, =BOOTMEM_DATA_ATTR
    bl     build_normal_pte_table

    // map the UART memory for early-print
    ldr    vaddr, =CONFIG_UART_BASE
    mov    paddr, vaddr
    ldr    size, =CONFIG_UART_IO_SIZE
    ldr    pte_attr, =BOOTMEM_IO_BLK_ATTR
    bl     build_io_pte_table

    // map the left 4K pages
    adr    vaddr, kernel_bootmem_base
    ldr    vaddr, [vaddr]
    ldr    tmp, =CONFIG_KERNEL_ADDR_MASK // tmp reg is free now!
    sub    vaddr, vaddr, tmp
    mov    tmp, vaddr
    ldr    entry_mask, =0x1fffff
    add    tmp, tmp, entry_mask
    mvn    entry_align, entry_mask
    and    tmp, tmp, entry_align
    mov    paddr, vaddr
    sub    size, tmp, vaddr
    ldr    pte_attr, =BOOTMEM_DATA_ATTR
    bl     build_normal_pte_table

    ret    x26


/**
 * @brief boot阶段恒等映射页表的建立
 *
 */

do_map_id:
    /* save the lr register */
    mov    x26, x30

    /* identity pgd is defined in lds */
    mov    x1, #0
    mov    x2, #4096
    ldr    page_table, = __identity_page_table
    mov    x0, page_table
    bl     memset

    /* 申请恒等映射页表的其他级子页表，并填入子页表的地址到上级,
       仅预留了2M的空间，映射boot内容应该是足够了的 */
    bl     build_id_page_table

__map_boot_code:
    ldr    vaddr, =__start_boot_code
    ldr    tmp, =__stop_boot_code
    sub    size, tmp, vaddr
    mov    paddr, vaddr
    ldr    pte_attr, =BOOTMEM_CODE_ATTR
    bl     build_normal_pte_table

__map_boot_data:
    ldr    vaddr, = __start_boot_data
    ldr    tmp, =__stop_boot_data
    sub    size, tmp, vaddr
    mov    paddr, vaddr
    ldr    pte_attr, =BOOTMEM_DATA_ATTR
    bl     build_normal_pte_table
    ret    x26

build_normal_pmd_table:
    mov    x25, x30
    ldr    entry_mask, =0x1fffff
    ldr    entry_align, =0xffe00000

    ret    x25

build_io_pte_table:
    mov    x25, x30

    ldr    entry_size, =0x200000
    ldr    entry_mask, =0x1fffff
    mvn    entry_align, entry_mask

    // alloc one page to map 2M IO pmd
    /*
    mov    x0, pagetable_base
    add    pagetable_base, pagetable_base, #4096
    mov    pmd_tmp, x0
    mov    x1, #0
    mov    x2, #4096
    bl    memset
    */


    // x1 = align_up(vaddr, entry_size)
    add    x1, vaddr, size
    add    x1, x1, entry_mask
    and    x1, x1, entry_align
    // vaddr = paddr = align_down(vaddr, entry_size)
    and    vaddr, vaddr, entry_align
    and    paddr, paddr, entry_align
    sub    size, x1, vaddr

    ubfx   x0, vaddr, #30, #9
    ldr    x1, =HOST_TABLE_DES
    ldr    pmd_tmp, [ttb0_pud, x0, lsl #3]
    bic    pmd_tmp, pmd_tmp, x1
    //
    // orr    x1, pmd_tmp, x1
    // str    x1, [ttb0_pud, x0, lsl #3]

    lsr    x2, vaddr, #21
    orr    paddr, paddr, pte_attr

loop_io_pte:
    str    paddr, [pmd_tmp, x2, lsl #3]
    sub    size, size, entry_size
    add    paddr, paddr, entry_size
    add    x2, x2, #1
    cbnz   size, loop_io_pte
    ret    x25



build_normal_pte_table:
    ldr    entry_size, =0x1000
    ldr    entry_mask, =0xfff
    ldr    entry_align, =0xfffffffffffff000

    // the va and pa must page align, so the info
    // from lds must correct
    and    vaddr, vaddr, entry_align
    and    paddr, paddr, entry_align
    and    size, size, entry_align

    // get the va offset in bootmem
    adr    x0, kernel_start
    ldr    x0, [x0]
    sub    x0, vaddr, x0
    ubfx   pte_index, x0, #12, #20
    mov    pmd_tmp, ttb0_pt
    add    pmd_tmp, pmd_tmp, pte_index, lsl #3

    bic    paddr, paddr, entry_mask
    bic    paddr, paddr, #0xffff000000000000
    orr    paddr, paddr, pte_attr

loop_normal_pte:
    cbz    size, exit_loop
    str    paddr, [pmd_tmp]
    sub    size, size, entry_size
    add    pmd_tmp, pmd_tmp, #8
    add    paddr, paddr, entry_size
    b      loop_normal_pte
exit_loop:
    ret


build_id_page_table:
    mov    x25, x30

    // identify map 2M is enough
    // so need 3 pages(1 pud and 1 pmd and 1pt)
    mov    x3, #3
    mov    ttb0_pud, pagetable_base
    ldr    x2, =(4096 * 3)  // 3 pages
    add    pagetable_base, pagetable_base, x2
    mov    x0, ttb0_pud
    mov    x1, #0
    bl     memset // clear 3 pages

    // pgd has 1 entry(idx: 0)
    mov    tmp, ttb0_pud
    mov    pte_index, 0
    ldr    x1, =HOST_TABLE_DES

    orr    pte_value, tmp, x1
    str    pte_value, [page_table, pte_index, lsl #3]

    // pud has 1 entry(idx: 1)
    add    ttb0_pmd, ttb0_pud, #0x1000
    mov    tmp, ttb0_pmd
    mov    pte_index, 1
    ldr    x1, =HOST_TABLE_DES

    orr    pte_value, tmp, x1
    str    pte_value, [ttb0_pud, pte_index, lsl #3]

    // pmd has 1 entry(idx: 0)
    add    ttb0_pt, ttb0_pmd, #0x1000
    mov    tmp, ttb0_pt
    mov    pte_index, 0
    ldr    x1, =HOST_TABLE_DES
    orr    pte_value, tmp, x1
    str    pte_value, [ttb0_pmd, pte_index, lsl #3]


    ret x25


// 4-level pagetable:
//   pgd => pud => pmd => pte
build_page_table:
    mov    x25, x30

    // map first 4GB for kernel memory, here
    // need 5 pages(1 pud and 4 pmd)
    mov    x3, #5
    mov    ttb0_pud, pagetable_base
    ldr    x2, =(4096 * 5)  // 5 pages
    add    pagetable_base, pagetable_base, x2
    mov    x0, ttb0_pud
    mov    x1, #0
    bl     memset // clear 5 pages

    // pgd has 1 entry(idx: 0)
    mov    tmp, ttb0_pud
    mov    pte_index, 0
    ldr    x1, =HOST_TABLE_DES

    orr    pte_value, tmp, x1
    str    pte_value, [page_table, pte_index, lsl #3]

    // pud has 4 entry(0-4G)
    add    ttb0_pmd, ttb0_pud, #0x1000
    mov    vaddr, #0x0
    mov    entry_size, #4
    mov    tmp, ttb0_pmd
    mov    pte_index, 0
    ldr    x1, =HOST_TABLE_DES
loop_fill_pud:
    orr    pte_value, tmp, x1
    str    pte_value, [ttb0_pud, pte_index, lsl #3]
    add    pte_index, pte_index, #1
    add    tmp, tmp, #0x1000
    sub    entry_size, entry_size, #1
    cbnz   entry_size, loop_fill_pud // fill 4 entries in PUD


    // count how many memory is mapped as 4K page
    // other memory will mapped as 2M block
    adr    vaddr, kernel_bootmem_base
    ldr    vaddr, [vaddr]
    ldr    tmp, =CONFIG_KERNEL_ADDR_MASK
    sub    vaddr, vaddr, tmp
    mov    tmp, vaddr
    ldr    entry_mask, =0x1fffff  // 2M mask
    add    tmp, tmp, entry_mask
    mvn    entry_align, entry_mask
    and    tmp, tmp, entry_align  // below: tmp = align_up(vaddr, 2M)
    adr    pte_value, kernel_start
    ldr    pte_value, [pte_value]

    // size record the 4K page size
    // tmp_const record the 2M block start address
    mov    tmp_const, tmp
    sub    size, tmp, pte_value


dbg_cal_size:
    lsr    x3, size, #21  // x3: number of entry in pmd for this 4K pages
    lsl    x2, x3, #12    // x2: number of pt for this 4k pages

    // need x2 pages for pt(level-3 pagetable)
    mov    ttb0_pt, pagetable_base
    add    pagetable_base, pagetable_base, x2
    mov    x0, ttb0_pt
    mov    x1, #0
    bl     memset

    // kernel_start address 2M align kernel_start
    // will set at boot stage correctly
    adr    vaddr, kernel_start
    ldr    vaddr, [vaddr]

    lsr    pte_index, vaddr, #21
    mov    tmp, ttb0_pt
    ldr    x1, =HOST_TABLE_DES
loop_fill_pmd_tbl:
    orr    pte_value, tmp, x1
    str    pte_value, [ttb0_pmd, pte_index, lsl #3]
    add    pte_index, pte_index, #1
    add    tmp, tmp, #4096
    sub    x3, x3, #1
    cbnz    x3, loop_fill_pmd_tbl  // fill entries in PMD for 4K pages

    // continue map the 2M block memory
    ldr    x2, =CONFIG_KERNEL_RAM_SIZE
    sub    size, x2, size
    lsr    size, size, #21  // size: number of pmd entry for 2M block
    ldr    entry_mask, =0x200000 // 这里不是

    mov    paddr, tmp_const
    ldr    pte_attr, =BOOTMEM_DATA_BLK_ATTR
loop_fill_pmd_blk:
    orr    pte_value, paddr, pte_attr
    str    pte_value, [ttb0_pmd, pte_index, lsl #3]
    add    pte_index, pte_index, #1
    add    paddr, paddr, entry_mask
    sub    size, size, #1
    cbnz    size, loop_fill_pmd_blk

    ret    x25
