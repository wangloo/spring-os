#pragma once
#include <memattr.h>
#include <stage1.h>
#include <page.h>


#define PGD_SHIFT            (39)
#define PUD_SHIFT            (30)
#define PMD_SHIFT            (21)
// #define PAGE_SHIFT           (12)

#define PGD_SIZE             (1UL << PGD_SHIFT) // 顶级页表 一项能存的大小
#define PUD_SIZE             (1UL << PUD_SHIFT)
#define PMD_SIZE             (1UL << PMD_SHIFT)
// #define PAGE_SIZE            (1UL << PAGE_SHIFT)

/* The number of entries in one page table page */
#define PTP_ENTRIES          (1UL << (PAGE_SHIFT - 3))

#define IS_1G_BLOCK_ALIGN(x) (!((unsigned long)(x) & (PUD_SIZE-1)))
#define IS_2M_BLOCK_ALIGN(x) (!((unsigned long)(x) & (PMD_SIZE-1)))

#define pgd_index(va)        (((va) >> PGD_SHIFT) & (PTP_ENTRIES - 1))
#define pud_index(va)        (((va) >> PUD_SHIFT) & (PTP_ENTRIES - 1))
#define pmd_index(va)        (((va) >> PMD_SHIFT) & (PTP_ENTRIES - 1))
#define pt_index(va)         (((va) >> PAGE_SHIFT) & (PTP_ENTRIES - 1))

#define pud_block_addr(pude) ((pude).l1_block.pfn << PUD_SHIFT) // 1G
#define pmd_block_addr(pmde) ((pmde).l2_block.pfn << PMD_SHIFT) // 2M
#define pt_page_addr(pte)    ((pte).l3_page.pfn << PAGE_SHIFT)  // 4K

#define pte_is_vaild(entry)  ((entry).table.is_valid)
#define pte_is_null(entry)   ((entry).pte == 0)
#define pte_is_table(entry)  ((entry).table.is_table)

#define pte_table_addr(pte)                                                    \
  (page_table_t *)((paddr_t)((pte).table.next_table_addr) << PAGE_SHIFT)


/* table format */
typedef union {
  struct {
    uint64_t is_valid : 1, is_table : 1, ignored1 : 10, next_table_addr : 36,
        reserved : 4, ignored2 : 7,
        PXNTable : 1, // Privileged Execute-never for next level
        XNTable : 1,  // Execute-never for next level
        APTable : 2,  // Access permissions for next level
        NSTable : 1;
  } table;            // L0~L2页表项描述符的结构
  struct {
    uint64_t is_valid : 1, is_table : 1,
        attr_index : 3,              // Memory attributes index
        NS : 1,                      // Non-secure
        AP : 2,                      // Data access permissions
        SH : 2,                      // Shareability
        AF : 1,                      // Accesss flag
        nG : 1,                      // Not global bit
        reserved1 : 4, nT : 1, reserved2 : 13, pfn : 18, reserved3 : 2, GP : 1,
        reserved4 : 1, DBM : 1,      // Dirty bit modifier
        Contiguous : 1, PXN : 1,     // Privileged execute-never
        UXN : 1,                     // Execute never
        soft_reserved : 4, PBHA : 4; // Page based hardware attributes
  } l1_block;                        // L1块描述符的结构
  struct {
    uint64_t is_valid : 1, is_table : 1,
        attr_index : 3,              // Memory attributes index
        NS : 1,                      // Non-secure
        AP : 2,                      // Data access permissions
        SH : 2,                      // Shareability
        AF : 1,                      // Accesss flag
        nG : 1,                      // Not global bit
        reserved1 : 4, nT : 1, reserved2 : 4, pfn : 27, reserved3 : 2, GP : 1,
        reserved4 : 1, DBM : 1,      // Dirty bit modifier
        Contiguous : 1, PXN : 1,     // Privileged execute-never
        UXN : 1,                     // Execute never
        soft_reserved : 4, PBHA : 4; // Page based hardware attributes
  } l2_block;                        // L2块描述符的结构
  struct {
    uint64_t is_valid : 1, is_page : 1,
        attr_index : 3,                  // Memory attributes index
        NS : 1,                          // Non-secure
        AP : 2,                          // Data access permissions
        SH : 2,                          // Shareability
        AF : 1,                          // Accesss flag
        nG : 1,                          // Not global bit
        pfn : 36, reserved : 3, DBM : 1, // Dirty bit modifier
        Contiguous : 1, PXN : 1,         // Privileged execute-never
        UXN : 1,                         // Execute never
        soft_reserved : 4, PBHA : 4,     // Page based hardware attributes
        ignored : 1;
  } l3_page;                             // L3页表描述符
  u64 pte;                               // 页表项
} pte_t;

typedef struct _page_table {
  pte_t entry[PTP_ENTRIES];
} page_table_t __attribute__((aligned(PAGE_SIZE)));

typedef void *(*pgtable_alloc_func_t)(void);

int pagetable_map(page_table_t *pagetable, 
      vaddr_t va, paddr_t pa, size_t size, int flags, pgtable_alloc_func_t pgtable_alloc);
int pagetable_unmap(page_table_t *pagetable, 
      vaddr_t start, vaddr_t end, int flags);
paddr_t pagetable_va_to_pa(page_table_t *pagetable, vaddr_t va);
void DBG_pagetable(page_table_t *pagetable);
