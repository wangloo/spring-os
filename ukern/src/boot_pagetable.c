#include <kernel.h>
#include <page.h>
#include <pagetable.h>

#define BOOTDATA  __section(__boot_data)
BOOTDATA struct pagetable id_pgtable;

#if 0
#define EARLY_PGTABLE_MAX 10  // enough?
#define BOOT   

static BOOT struct pagetable early_pgtable_mem[EARLY_PGTABLE_MAX];  
static BOOT int early_pgtable_index;

void * early_pgtable_alloc(void)
{
    if (early_pgtable_index == EARLY_PGTABLE_MAX)
        return NULL;
    return &early_pgtable_mem[early_pgtable_index++];
}

void map_boot_identify(void)
{
    extern unsigned long __start_boot_code;
    extern unsigned long __stop_boot_code;
    extern unsigned long __stop_code;
    extern unsigned long __start_data;
    extern unsigned long __stop_data;
    u64 vbase, pbase, vend, size;

    vbase = (u64)&__start_boot_code;
    pbase = (u64)&__start_boot_code;
    vend  = (u64)&__stop_code;
    size = vend - vbase;

    pagetable_map(&id_pgtable, vbase, pbase, size, VM_RO | VM_KERN |__VM_EXEC, early_pgtable_alloc);

    // vbase = (u64)&__start_data;
    // pbase = (u64)&__start_data;
    // vend  = (u64)&__stop_data;
    // size = vend - vbase;
    // pagetable_map(&id_pgtable, vbase, pbase, size, BOOTMEM_DATA_ATTR, early_pgtable_alloc);
    // DBG_pagetable(&id_pgtable);


}

#endif