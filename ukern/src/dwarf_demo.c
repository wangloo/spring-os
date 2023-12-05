#include <kernel.h>
#include <kmem.h>
#include <ramdisk.h>
#include <elf.h>
#include <lib/libdwarf.h>
#include <lib/dwarf.h>
#include <lib/libdwarf_ukern.h>

#define TRUE 1
#define FALSE 0


static void
print_die_data(Dwarf_Debug dbg, Dwarf_Die print_me, int in_level)
{
    char *name = 0;
    Dwarf_Error error = 0;
    Dwarf_Error *errp = 0;
    int res = 0;

    res = dwarf_diename(print_me,&name,errp);
    if(res == DW_DLV_ERROR) {
        printf("Error in dwarf_diename  %d \n");
        exit(1);
    }
    if(res == DW_DLV_NO_ENTRY) {
        printf("No name\n");
        return;
    }  

    printf("level %d, diename: %s\n", in_level,  name);
}

static void
get_die_and_siblings(Dwarf_Debug dbg, Dwarf_Die in_die,
    int is_info,int in_level)
{
    int res = DW_DLV_ERROR;
    Dwarf_Die cur_die=in_die;
    Dwarf_Die child = 0;
    Dwarf_Error error = 0;
    Dwarf_Error *errp = 0;

    errp = &error;


    print_die_data(dbg, cur_die, in_level);

#if 0   // Too many print, open it later
    for(;;) {
        Dwarf_Die sib_die = 0;
        res = dwarf_child(cur_die,&child,errp);
        if(res == DW_DLV_ERROR) {
            printf("Error in dwarf_child , level %d \n",in_level);
            exit(1);
        }
        if(res == DW_DLV_OK) {
            get_die_and_siblings(dbg,child,is_info,in_level+1);
        }
        /* res == DW_DLV_NO_ENTRY */
        res = dwarf_siblingof_b(dbg,cur_die,is_info,&sib_die,errp);
        if(res == DW_DLV_ERROR) {
            char *em = errp?dwarf_errmsg(error):"Error siblingof_b";
            printf("Error in dwarf_siblingof_b , level %d :%s \n",
                in_level,em);
            exit();
        }
        if(res == DW_DLV_NO_ENTRY) {
            /* Done at this level. */
            break;
        }
        /* res == DW_DLV_OK */
        if(cur_die != in_die) {
            dwarf_dealloc(dbg,cur_die,DW_DLA_DIE);
        }
        cur_die = sib_die;
        // print_die_data(dbg,cur_die,in_level);
    }
#endif
    return;
}

static void
read_cu_list(Dwarf_Debug dbg)
{
    Dwarf_Unsigned cu_header_length = 0;
    Dwarf_Unsigned abbrev_offset = 0;
    Dwarf_Half     address_size = 0;
    Dwarf_Half     version_stamp = 0;
    Dwarf_Half     offset_size = 0;
    Dwarf_Half     extension_size = 0;
    Dwarf_Sig8     signature;
    Dwarf_Unsigned typeoffset = 0;
    Dwarf_Unsigned next_cu_header = 0;
    Dwarf_Half     header_cu_type = DW_UT_compile;
    Dwarf_Bool     is_info = TRUE;
    Dwarf_Error error;
    int cu_number = 0;
    Dwarf_Error *errp  = 0;

    errp = &error;
    for(;;++cu_number) {
        // printf("=> CU: %d\n", cu_number);
        Dwarf_Die no_die = 0;
        Dwarf_Die cu_die = 0;
        int res = DW_DLV_ERROR;

        res = dwarf_next_cu_header_d(dbg,is_info,&cu_header_length,
            &version_stamp, &abbrev_offset,
            &address_size, &offset_size,
            &extension_size,&signature,
            &typeoffset, &next_cu_header,
            &header_cu_type,errp);
        if(res == DW_DLV_ERROR) {
            char *em = errp?dwarf_errmsg(error):"An error next cu her";
            printf("Error in dwarf_next_cu_header: %s\n",em);
            exit(1);
        }
        if(res == DW_DLV_NO_ENTRY) {
            /* Done. */
            return;
        }

        /* The CU will have a single sibling, a cu_die. */
        res = dwarf_siblingof_b(dbg,no_die,is_info,
            &cu_die,errp);
        if(res == DW_DLV_ERROR) {
            char *em = errp?dwarf_errmsg(error):"An error";
            printf("Error in dwarf_siblingof_b on CU die: %s\n",em);
            exit();
        }
        if(res == DW_DLV_NO_ENTRY) {
            /* Impossible case. */
            printf("no entry! in dwarf_siblingof on CU die \n");
            exit();
        }
        get_die_and_siblings(dbg,cu_die,is_info,0);
        dwarf_dealloc(dbg,cu_die,DW_DLA_DIE);
        // resetsrcfiles(dbg,&sf);
    }
}

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

  printf("==========READ CU LIST====\n");
  read_cu_list(dbg);

  ret = dwarf_finish_rd(dbg, &error);
  assert(ret == 0);
}
