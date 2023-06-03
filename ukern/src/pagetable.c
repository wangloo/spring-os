#include <pagetable.h>
#include <assert.h>



paddr_t pagetable_va_to_pa(page_table_t *pagetable, vaddr_t va)
{
  pte_t pgde, pude, pmde, pte;
  page_table_t *next_table;

  pgde = pagetable->entry[pgd_index(va)];
  assert(pte_is_vaild(pgde));
  assert(pte_is_table(pgde));

  // pud
  next_table = pte_table_addr(pgde);
  pude = next_table->entry[pud_index(va)];
  assert(pte_is_vaild(pude));
  if (!pte_is_table(pude)) {
    return (paddr_t)pud_block_addr(pude);
  }

  // pmd
  next_table = pte_table_addr(pude);
  pmde = next_table->entry[pmd_index(va)];
  assert(pte_is_vaild(pmde));
  if (!pte_is_table(pmde)) {
    return (paddr_t)pmd_block_addr(pmde);
  }

  // pt
  next_table = pte_table_addr(pmde);
  pte = next_table->entry[pt_index(va)];
  assert(pte_is_vaild(pte));
  return (paddr_t)pt_page_addr(pte);
}


page_table_t *kernel_pgd_base(void)
{
  extern unsigned char __kernel_page_table;

  return (page_table_t *)&__kernel_page_table;
}
