#include <pagetable.h>
#include <assert.h>



// paddr_t pagetable_va_to_pa(page_table_t *pagetable, vaddr_t va)
// {
//   pte_t pud, pmd, pte;
//   paddr_t pa;

//   // get pud and check block or table
//   pud = pagetable->entry[pgd_index(va)];
//   assert(is_vaild_entry(pud));
//   if (pud_is_block(pud)) { // 2M
//     pa = pud.l1_block.pfn << PUD_SHIFT;
//     return pa;
//   }

//   // get pmd and check block or table
//   pmd = (pud_table_addr(pud))->entry[pud_index(va)];
//   assert(is_vaild_entry(pmd));
//   if (pmd_is_block(pmd)) {  // 64K
//     pa = pmd.l2_block.pfn << PMD_SHIFT;
//     return pa;
//   }

//   // get pte and return pa
//   pte = (pmd_table_addr(pud))->entry[pmd_index(va)];
//   assert(is_vaild_entry(pte));
//   pa = pte.l3_page.pfn << PAGE_SHIFT;
//   return pa;
// }


vaddr_t kernel_pgd_base(void)
{
  extern unsigned char __kernel_page_table;

  return (vaddr_t)&__kernel_page_table;
}
