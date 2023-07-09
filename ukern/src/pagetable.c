#include <pagetable.h>
#include <barrier.h>
#include <assert.h>
#include <page.h>
#include <errno.h>
#include <string.h>
#include <addrspace.h> // 临时起名

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

static void pagetable_change_pgde(pte_t *pgdep, void *addr, u64 flags)
{
  u64 attr = 0;

  attr |= S1_DES_TABLE;
  if (flags & VM_KERN)
    attr |= S1_TABLE_UAP | S1_TABLE_UXN;

  set_pgde(pgdep, (pte_t)(vtop(addr) | attr));
}

static void pagetable_change_pude(pte_t *pudep, 
                void *addr, bool is_block, int flags)
{
  u64 attr = 0;

  if (!is_block) {
    attr |= S1_DES_TABLE;
    if (flags & VM_KERN)
      attr |= S1_TABLE_UAP | S1_TABLE_UXN;
  }
  else {
    switch (flags & VM_TYPE_MASK) {
    case __VM_NORMAL_NC:
      attr |= S1_BLOCK_NORMAL_NC;
      break;
    case __VM_IO:
      attr |= S1_BLOCK_DEVICE;
      break;
      break;
    case __VM_WT:
      attr |= S1_BLOCK_WT;
      break;
    default:
      attr |= S1_BLOCK_NORMAL;
      break;
    }

    if (flags & __VM_KERN) {
      if ((flags & VM_RW_MASK) == __VM_RO)
        attr |= S1_AP_RO;
      else
        attr |= S1_AP_RW;
    } else {
      attr |= S1_nG;
      if ((flags & VM_RW_MASK) == __VM_RO)
        attr |= S1_AP_RO_URO;
      else
        attr |= S1_AP_RW_URW;
    }

    if (!(flags & __VM_EXEC))
      attr |= (S1_XN | S1_PXN);

    if (flags & __VM_PFNMAP)
      attr |= S1_PFNMAP;

    if (flags & __VM_DEVMAP)
      attr |= S1_DEVMAP;

    if (flags & (__VM_SHARED | __VM_PMA))
      attr |= S1_SHARED;
  }

  set_pude(pudep, (pte_t)(vtop(addr) | attr));
}

static void pagetable_change_pmde(pte_t *pmdep, 
              void *addr, bool is_block, int flags)
{
  u64 attr = 0;

  if (!is_block) {
    attr |= S1_DES_TABLE;
    if (flags & VM_KERN)
      attr |= S1_TABLE_UAP | S1_TABLE_UXN;
  }
  else {
    switch (flags & VM_TYPE_MASK) {
    case __VM_NORMAL_NC:
      attr |= S1_BLOCK_NORMAL_NC;
      break;
    case __VM_IO:
      attr |= S1_BLOCK_DEVICE;
      break;
      break;
    case __VM_WT:
      attr |= S1_BLOCK_WT;
      break;
    default:
      attr |= S1_BLOCK_NORMAL;
      break;
    }

    if (flags & __VM_KERN) {
      if ((flags & VM_RW_MASK) == __VM_RO)
        attr |= S1_AP_RO;
      else
        attr |= S1_AP_RW;
    } else {
      attr |= S1_nG;
      if ((flags & VM_RW_MASK) == __VM_RO)
        attr |= S1_AP_RO_URO;
      else
        attr |= S1_AP_RW_URW;
    }

    if (!(flags & __VM_EXEC))
      attr |= (S1_XN | S1_PXN);

    if (flags & __VM_PFNMAP)
      attr |= S1_PFNMAP;

    if (flags & __VM_DEVMAP)
      attr |= S1_DEVMAP;

    if (flags & (__VM_SHARED | __VM_PMA))
      attr |= S1_SHARED;
  }

  set_pmde(pmdep, (pte_t)(vtop(addr) | attr));
}

static void pagetable_change_pte(pte_t *ptep, void *addr,  int flags)
{
  u64 attr = 0;

	switch (flags & VM_TYPE_MASK) {
	case __VM_NORMAL_NC:
		attr |= S1_PAGE_NORMAL_NC;
		break;
	case __VM_IO:
		attr |= S1_PAGE_DEVICE;
		break;
	case __VM_WT:
		attr |= S1_PAGE_WT;
		break;
	default:
		attr |= S1_PAGE_NORMAL;
		break;
	}

	if (flags & __VM_KERN) {
		if ((flags & VM_RW_MASK) == __VM_RO)
			attr |= S1_AP_RO;
		else
			attr |= S1_AP_RW;
	} else {
		attr |= S1_nG;
		if ((flags & VM_RW_MASK) == __VM_RO)
			attr |= S1_AP_RO_URO;
		else
			attr |= S1_AP_RW_URW;
	}

  if (!(flags & __VM_EXEC))
    attr |= (S1_XN | S1_PXN);

  if (flags & __VM_PFNMAP)
	  attr |= S1_PFNMAP;

	if (flags & __VM_DEVMAP)
	  attr |= S1_DEVMAP;

  if (flags & (__VM_SHARED | __VM_PMA))
	  attr |= S1_SHARED;

  set_pte(ptep, (pte_t)(vtop(addr) | attr));
}




static bool can_map_pud_huge(pte_t pude, 
              vaddr_t va, paddr_t pa, size_t size, int flags)
{
  if (!(flags & __VM_HUGE_1G) || !(pte_is_null(pude)))
    return false;
  if (!IS_1G_BLOCK_ALIGN(va) || !IS_1G_BLOCK_ALIGN(pa) || 
      !IS_1G_BLOCK_ALIGN(size))
    return false;

  return true;
}

static bool can_map_pmd_huge(pte_t pmde, 
              vaddr_t va, paddr_t pa, size_t size, int flags)
{
  if (!(flags & __VM_HUGE_2M) || !(pte_is_null(pmde)))
    return false;
  if (!IS_2M_BLOCK_ALIGN(va) || !IS_2M_BLOCK_ALIGN(pa) || 
      !IS_2M_BLOCK_ALIGN(size))
    return false;

  return true;
}


static int map_pt_range(page_table_t *pt, 
              vaddr_t va, paddr_t pa, size_t size, int flags)
{
  pte_t *ptep;
  vaddr_t end;
  
  end = va + size;
  ptep = &pt->entry[pt_index(va)];
  do {
    assert(pte_is_null(*ptep));
    pagetable_change_pte(ptep, (void *)pa, flags);

    va += PAGE_SIZE;
    pa += PAGE_SIZE;
    ptep += 1;
  } while (va != end);

  return 0;
}

static int map_pmd_range(page_table_t *pmd, 
              vaddr_t va, paddr_t pa, size_t size, int flags, pgtable_alloc_func_t pgtable_alloc)
{
  pte_t *pmdep;
  void *pt;
  int ret;
  size_t map_size;

  pmdep = &pmd->entry[pmd_index(va)];
  do {
    if (can_map_pmd_huge(*pmdep, va, pa, size, flags)) {
      pagetable_change_pmde(pmdep, (void *)pa, true, flags);
      map_size = PMD_SIZE;
    }
    else {
      
      map_size = size > PMD_SIZE ? PMD_SIZE : size;
      if (pte_is_null(*pmdep)) {
        pt = pgtable_alloc();
        if (!pt)
          return -ENOMEM;
        memset(pt, 0, PAGE_SIZE);
        pagetable_change_pmde(pmdep, pt, false, flags);
      }
      else {
        pt = (void *)ptov(pte_table_addr(*pmdep));
      }
      printf("va: 0x%lx, size: 0x%lx, pt: 0x%lx\n",va, size, pt);
      ret = map_pt_range(pt, va, pa, map_size, flags);
      if (ret)
        return ret;
    }

    // after finishing one map, update info
    va += map_size, pa += map_size;
    size -= map_size;

  } while (pmdep++, size > 0);
  return 0;
}

static int map_pud_range(page_table_t *pud, 
              vaddr_t va, paddr_t pa, size_t size, int flags, pgtable_alloc_func_t pgtable_alloc)
{
  pte_t *pudep;
  void *pmd;
  int ret;
  size_t map_size;

  pudep = &pud->entry[pud_index(va)];
  do {
    if (can_map_pud_huge(*pudep, va, pa, size, flags)) {
      assert(0); // TODO: unsupport 1G block now, no idea!
      pagetable_change_pude(pudep, (void *)pa, true, flags);
      map_size = PUD_SIZE;
    }
    else {
      map_size = size >= PUD_SIZE ? PUD_SIZE : size;
      if (pte_is_null(*pudep)) {
        pmd = pgtable_alloc();
        if (!pmd)
          return -ENOMEM;
        memset(pmd, 0, PAGE_SIZE);
        pagetable_change_pude(pudep, pmd, false, flags);
      }
      else {
        pmd = (void *)ptov(pte_table_addr(*pudep));
      }
      ret = map_pmd_range(pmd, va, pa, map_size, flags, pgtable_alloc);
      if (ret)
        return ret;

    } 

    // after finishing one map, update info
    va += map_size, pa += map_size;
    size -= map_size;
  } while (pudep++, size > 0);

  return 0;
}


static int map_pgd_range(page_table_t *pagetable, vaddr_t va, paddr_t pa, 
              size_t size, int flags, pgtable_alloc_func_t pgtable_alloc)
{
  pte_t *pgdep;
  void *pud;
  int ret;
  size_t map_size;

  pgdep = &pagetable->entry[pgd_index(va)];
  do {
    if (pte_is_null(*pgdep)) {
      pud = pgtable_alloc();
      if (!pud)
        return -ENOMEM;
      memset(pud, 0, PAGE_SIZE);
      pagetable_change_pgde(pgdep, pud, flags);
    }
    else {
      pud = (void *)ptov(pte_table_addr(*pgdep));
    }

    map_size = size > PGD_SIZE ? PGD_SIZE : size;
    ret = map_pud_range(pud, va, pa, map_size, flags, pgtable_alloc);
    if (ret) 
      return ret;

    // after finishing one map, update info
    va += map_size, pa += map_size;
    size -= map_size;
      
  } while (pgdep++, size > 0);

  return 0;
}
int pagetable_map(page_table_t *pagetable, vaddr_t va, paddr_t pa, 
      size_t size, int flags, pgtable_alloc_func_t pgtable_alloc)
{
  // 在这里检查后，所有的内部函数都不再检查合法性
  assert(size > 0);
  assert(va < VA_MAX && va+size < VA_MAX);
  assert(pa < PA_MAX);
  assert(IS_PAGE_ALIGN(va) && IS_PAGE_ALIGN(size) && IS_PAGE_ALIGN(pa));
  
  return map_pgd_range(pagetable, va, pa, size, flags, pgtable_alloc);
}

int pagetable_unmap(page_table_t *pagetable, 
      vaddr_t start, vaddr_t end, int flags)
{
  assert(0);
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
  // extern unsigned char __kernel_page_table;

  // return (page_table_t *)&__kernel_page_table;
  assert(0);
  return NULL;
}
