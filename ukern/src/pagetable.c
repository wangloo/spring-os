#include <pagetable.h>
#include <barrier.h>
#include <assert.h>
#include <page.h>

#define VA_MAX (1ul << 48)
#define PA_MAX (1ul << 48)

#define WRITE_ONCE(x, y) (x) = (y)
static inline void set_pte(pte_t *ptep, pte_t new_pte)
{
  WRITE_ONCE(*ptep, new_pte);
  __dsb(ishst);
}
static inline void set_pmde(pte_t *pmdep, pte_t new_pmde)
{
  WRITE_ONCE(*pmdep, new_pmde);
  __dsb(ishst);
}
static inline void set_pude(pte_t *pudep, pte_t new_pude)
{
  WRITE_ONCE(*pudep, new_pude);
  __dsb(ishst);
}
static inline void set_pgde(pte_t *pgdep, pte_t new_pgde)
{
  WRITE_ONCE(*pgdep, new_pgde);
  __dsb(ishst);
}


static int _map_pgd_range(page_table_t *pagetable, 
        vaddr_t va, paddr_t pa, size_t size, int flags)
{
  // pte_t pgde;

  assert(0);
  return 0;
}

int pagetable_map(page_table_t *pagetable, 
      vaddr_t va, paddr_t pa, size_t size, int flags)
{
  // 在这里检查后，所有的内部函数都不再检查合法性
  assert(size > 0);
  assert(va < VA_MAX && va+size < VA_MAX);
  assert(pa < PA_MAX);
  assert(IS_PAGE_ALIGN(va) && IS_PAGE_ALIGN(size) && IS_PAGE_ALIGN(pa));
  
  return _map_pgd_range(pagetable, va, pa, size, flags);
}


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
