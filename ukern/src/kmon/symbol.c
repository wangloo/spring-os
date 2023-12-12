#include <kernel.h>
#include <kmem.h>
#include <ramdisk.h>
#include <elf.h>
#include <lib/libdwarf.h>
#include <lib/dwarf.h>
#include <lib/libdwarf_ukern.h>

#define TRUE 1
#define FALSE 0

enum {
  RET_OK = 0,
  RET_ERR = -1,
  RET_NOT_FOUND = -2,
};

static struct ramdisk_file elf_file;
static Dwarf_Debug dbg = 0;
static Dwarf_Cie *cie_data = NULL;
static Dwarf_Signed cie_element_count = 0;
static Dwarf_Fde *fde_data = NULL;
static Dwarf_Signed fde_element_count = 0;

static int
symbol_get_line(unsigned long pc, Dwarf_Die cu_die, unsigned int *line)
{


}

static int
get_inst_info_die(unsigned long pc, Dwarf_Die cu_die, 
            Dwarf_Die prog_die, char **func, char **file, int *line)
{
  Dwarf_Line *linebuf;
  Dwarf_Signed linecount=0;
  Dwarf_Die line_die;
  Dwarf_Unsigned lineno;
  Dwarf_Addr lineaddr;
  Dwarf_Error err;
  Dwarf_Error *errp=&err;
  char *progname, *filename;
  int ret=0;

  if (dwarf_diename(cu_die, &filename, &err) != DW_DLV_OK) {
    LOG_ERROR("Faild to get subprogram name\n");
    return RET_ERR;
  }

  if (dwarf_diename(prog_die, &progname, &err) != DW_DLV_OK) {
    LOG_ERROR("Faild to get die name\n");
    return RET_ERR;
  }

  // Handle line number last
  if (dwarf_srclines(cu_die, &linebuf, &linecount, errp) != DW_DLV_OK) {
    return RET_ERR;
  }
  // printf("linecount: %d\n", linecount);

  for (int i = 0; i < linecount; i++) {
    if (dwarf_lineaddr(linebuf[i], &lineaddr, errp) != DW_DLV_OK ||
        dwarf_lineno(linebuf[i], &lineno, errp) != DW_DLV_OK) {
      ret = -1;
      goto free_srcline;
    }

    if (lineaddr == pc) // Yeah, we find it!
      break;
  }

  *func = progname;
  *file = filename;
  *line = lineno;

free_srcline:
  dwarf_srclines_dealloc(dbg, linebuf, linecount);
  return ret;
}

static int
get_inst_info_cu(unsigned long pc, Dwarf_Die cu_die, 
                  char **func, char **file, int *line)
{
  Dwarf_Die child;
  Dwarf_Error err;
  Dwarf_Error *errp=&err;
  char *cuname;
  int ret;


  ret = dwarf_child(cu_die, &child, errp);
  if (ret == DW_DLV_NO_ENTRY) 
    return RET_NOT_FOUND;
  else if (ret != DW_DLV_OK)
    return RET_ERR;

  // For debug only
  if (dwarf_diename(cu_die, &cuname, &err) != DW_DLV_OK) {
    LOG_ERROR("Faild to get cudie name\n");
    return RET_ERR;
  }
  printf("cuname: %s\n", cuname);

  do {
    Dwarf_Addr lowpc, highpc;
    Dwarf_Half highpc_form, tag;
    enum Dwarf_Form_Class highpc_class;

    if (dwarf_tag(child, &tag, &err) != DW_DLV_OK) {
      LOG_ERROR("Failed to get DIE tag\n");
      return -1;
    }
    if (tag != DW_TAG_subprogram)
      continue;

    // For debug only
    // char *progname;
    // if (dwarf_diename(child, &progname, &err) != DW_DLV_OK) {
    //   LOG_ERROR("Faild to get die name\n");
    //   return RET_ERR;
    // }
    // printf("searching prog: %s\n", progname);

    if (dwarf_lowpc(child, &lowpc, &err) == DW_DLV_OK &&
        dwarf_highpc_b(child, &highpc, &highpc_form, &highpc_class, &err) == DW_DLV_OK) {

      if (highpc_form != DW_FORM_addr) 
        highpc = lowpc+highpc; // returned highpc if offset

      // printf("highpc_form: %d, highpc_class: %d\n", highpc_form, highpc_class);
      // printf("lowpc: 0x%lx, highpc:0x%lx\n", lowpc, highpc);
      if (lowpc <= pc && highpc >= pc) {
        printf("find!!\n\n");
        
        return get_inst_info_die(pc, cu_die, child, func, file, line);
      }
    }
  } while (dwarf_siblingof(dbg, child, &child, &err) == DW_DLV_OK);

  return RET_NOT_FOUND;
}


int
kmon_get_inst_info(unsigned long pc, 
                char **func, char **file, int *line)
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
    int cu_number = 0; // For debug only
    Dwarf_Error *errp  = 0;

    errp = &error;
    for(;;++cu_number) {
        Dwarf_Die no_die = 0;
        Dwarf_Die cu_die = 0;
        int res = DW_DLV_ERROR;

        res = dwarf_next_cu_header_d(dbg,is_info,&cu_header_length,
            &version_stamp, &abbrev_offset,
            &address_size, &offset_size,
            &extension_size,&signature,
            &typeoffset, &next_cu_header,
            &header_cu_type,errp);
        if  (res == DW_DLV_ERROR) {
            char *em = errp?dwarf_errmsg(error):"An error next cu her";
            printf("Error in dwarf_next_cu_header: %s\n",em);
            return RET_ERR;
        } else if (res == DW_DLV_NO_ENTRY) {
            /* Done. */
            return 0;
        }

        /* The CU will have a single sibling, a cu_die. */
        res = dwarf_siblingof_b(dbg,no_die,is_info,
            &cu_die,errp);
        if (res == DW_DLV_ERROR) {
            char *em = errp?dwarf_errmsg(error):"An error";
            printf("Error in dwarf_siblingof_b on CU die: %s\n",em);
            return -1;
        } if(res == DW_DLV_NO_ENTRY) {
            /* Impossible case. */
            printf("no entry! in dwarf_siblingof on CU die \n");
            return -1;
        }

        // A valid die of CU is found
        // Check if it contains the function we need
        res = get_inst_info_cu(pc, cu_die, func, file, line);
        dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
        if (res != RET_NOT_FOUND) {
          return res;
        }
    }
    return -1;
}

int 
kmon_symbol_init(void)
{
  Dwarf_Handler errhand = 0;
  Dwarf_Ptr errptr = 0;
  Dwarf_Error err;
  int ret;

  ramdisk_open("spring.elf", &elf_file);

  if (dwarf_init_rd(&elf_file, DW_DLC_READ, 
                errhand, errptr, &dbg, &err) != DW_DLV_OK)
    return -1;

  // if (dwarf_get_fde_list(dbg, &cie_data, &cie_element_count, 
  //                     &fde_data, &fde_element_count, &err) != DW_DLV_OK)
  //   return -1;

  LOG_DEBUG("cie_element_count: %d, fde_element_count: %d\n", 
                          cie_element_count, fde_element_count);
  return 0;
}