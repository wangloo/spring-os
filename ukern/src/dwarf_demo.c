#include <kernel.h>
#include <kmem.h>
#include <ramdisk.h>
#include <lib/libdwarf.h>
#include <lib/libdwarf_ukern.h>




void test_libdwarf(void)
{
    Dwarf_Handler errhand = 0;
    Dwarf_Ptr errptr = 0;
    Dwarf_Error *errp = 0;
    Dwarf_Error error;
    Dwarf_Debug dbg = 0;
    int ret;
    struct ramdisk_file file;

    errp = &error;

    ramdisk_open("spring.elf", &file);
    ret = dwarf_init_rd(&file, DW_DLC_READ, errhand, errptr, &dbg, errp);

    assert(ret == 0);
}
