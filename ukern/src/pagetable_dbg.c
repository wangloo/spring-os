#include <kernel.h>
#include <page.h>
#include <pagetable.h>



static inline vaddr_t 
pack_va(int idx0, int idx1, int idx2, int idx3)
{
  return  ptov(((vaddr_t)idx0 << PGD_SHIFT) |
                ((vaddr_t)idx1 << PUD_SHIFT) |
                ((vaddr_t)idx2 << PMD_SHIFT) |
                ((vaddr_t)idx3 << PAGE_SHIFT));
}


static void _DBG_pagetable_level3(struct pagetable *table, int idxL0, int idxL1, int idxL2)
{
  pte_t entry;
  int idx;


    for (idx = 0; idx < PTP_ENTRIES; idx++) {
      entry = table->entry[idx];
      if (pte_is_null(entry)) continue;
      assert(pte_is_vaild(entry));
      print_space(2*4);
      printf("- page(4K), va: 0x%lx, pa: 0x%lx\n", 
                pack_va(idxL0, idxL1, idxL2, idx), pt_page_addr(entry));
    }
}

static void 
DBG_pagetable_level2(struct pagetable *table, int idxL0, int idxL1)
{
  struct pagetable *next_tab;
  pte_t entry;
  int idx;

  for (idx = 0; idx < PTP_ENTRIES; idx++) {
    entry = table->entry[idx];
    if (pte_is_null(entry)) continue;
    assert(pte_is_vaild(entry));

    if (pte_is_table(entry)) {
      next_tab = pte_table_addr(entry);
      print_space(2*3);
      printf("- L3 table, table addr: %p\n", next_tab);
      _DBG_pagetable_level3((struct pagetable *)(ptov(next_tab)), idxL0, idxL1, idx);
    }
    else {
      print_space(2*2);
      printf("- L3 block(2M), va: 0x%lx, pa: 0x%lx\n", 
                  pack_va(idxL0, idxL1, idx, 0), pmd_block_addr(entry));
    }
  }
}

static void 
DBG_pagetable_level1(struct pagetable *table, int idxL0)
{
  struct pagetable *next_tab;
  pte_t entry;
  int idx;

  for (idx = 0; idx < PTP_ENTRIES; idx++) {
    entry = table->entry[idx];
    if (pte_is_null(entry)) continue;
    assert(pte_is_vaild(entry));

    if (pte_is_table(entry)) {
      next_tab = pte_table_addr(entry);
      print_space(2*2);
      printf("- L2 table, table addr: %p\n", next_tab);
      DBG_pagetable_level2((struct pagetable *)(ptov(next_tab)), idxL0, idx);
    }
    else {
      print_space(2*1);
      printf("- L2 block(1G), va: 0x%lx, pa: 0x%lx\n", 
                  pack_va(idxL0, idx, 0, 0), pmd_block_addr(entry));
    }
  }
}

static void 
DBG_pagetable_level0(struct pagetable *table)
{
  struct pagetable *next_tab;
  pte_t entry;
  int idx;

  for (idx = 0; idx < PTP_ENTRIES; idx++) {
    entry = table->entry[idx];
    if (pte_is_null(entry)) continue;
    assert(pte_is_vaild(entry));
    assert (pte_is_table(entry));

    next_tab = pte_table_addr(entry);
    print_space(2*1);
    printf("- L1 table, table addr: %p\n", next_tab);
    DBG_pagetable_level1((struct pagetable *)(ptov(next_tab)), idx);
  }
}


// pagetable: virtual address of pagetable
void DBG_pagetable(struct pagetable *pagetable)
{
  printf("L0 table addr: %lx\n", (vaddr_t)pagetable);
  DBG_pagetable_level0(pagetable);
}
