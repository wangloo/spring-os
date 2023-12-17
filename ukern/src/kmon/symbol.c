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
findloc_die(unsigned long pc, Dwarf_Die cu_die, 
            Dwarf_Die prog_die, char **func, char **file, int *line)
{
  Dwarf_Line *linebuf;
  Dwarf_Signed linecount=0;
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
findloc_cu(unsigned long pc, Dwarf_Die cu_die, 
                  char **func, char **file, int *line)
{
  Dwarf_Die child;
  Dwarf_Error err;
  Dwarf_Error *errp=&err;
  char *comp_path;
  int ret;


  ret = dwarf_child(cu_die, &child, errp);
  if (ret == DW_DLV_NO_ENTRY) 
    return RET_NOT_FOUND;
  else if (ret != DW_DLV_OK)
    return RET_ERR;

  // Exclude cu of thirdparty
  if (dwarf_die_text(cu_die, DW_AT_comp_dir, &comp_path, errp) != DW_DLV_OK) {
    LOG_ERROR("Faild to get comp_path\n");
    return RET_ERR;
  }
  if (strstr(comp_path, "/lib")) {
    return RET_NOT_FOUND;
  }

  // For debug only
  // char *cuname;
  // if (dwarf_diename(cu_die, &cuname, &err) != DW_DLV_OK) {
  //   LOG_ERROR("Faild to get cudie name\n");
  //   return RET_ERR;
  // }
  // printf("cuname: %s\n", cuname);

  do {
    Dwarf_Addr lowpc, highpc;
    Dwarf_Half highpc_form, tag;
    enum Dwarf_Form_Class highpc_class;

    if (dwarf_tag(child, &tag, &err) != DW_DLV_OK) {
      LOG_ERROR("Failed to get DIE tag\n");
      return -1;
    }

    // Exclude non-function
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
        
        return findloc_die(pc, cu_die, child, func, file, line);
      }
    }
  } while (dwarf_siblingof(dbg, child, &child, &err) == DW_DLV_OK);

  return RET_NOT_FOUND;
}


int
findloc(unsigned long pc, char **func, char **file, int *line)
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
    int found = 0;
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
            // All cu are walked through
            goto out;
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

        // Walk through all cu even already found,
        // to make first cu returned for each call of 
        // dwarf_next_cu_header_d()
        if (found) {
          dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
          continue;
        }

        // A valid die of CU is found
        // Check if it contains the function we need
        res = findloc_cu(pc, cu_die, func, file, line);
        dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
        if (res == RET_ERR) {
          return res;
        }
        if (res == RET_OK) found = 1;
    }

out:
    return (found == 1) ? 0 : -1;
}

// Give a function or global variable name,
// return info about it
int
kmon_get_name_info(char *name,
                unsigned long *addr, char **file, int *line)
{
  return 0;
}





// Eventually function define
// Call this recursive to unwind stack
int 
findcaller(u64 curpc, u64 cursp, u64 *callerpc, u64 *callersp)
{
  Dwarf_Fde target_fde; // Fde about curpc
  Dwarf_Addr low_pc, high_pc; // Address range of fde
  Dwarf_Regtable3 regtable = {0}; // Register value about curpc
  Dwarf_Addr rowpc; // Start pc of row including curpc
  Dwarf_Error err;
  int oldrulecount = 0;

  // Get targer fde about curpc
  if (dwarf_get_fde_at_pc(fde_data, curpc, &target_fde, 
          &low_pc, &high_pc, &err) != DW_DLV_OK) 
    return -1;
  printf("lowpc: 0x%lx, highpc: 0x%lx\n", low_pc, high_pc);


  // Set number of register in .debug_frame
  // This number is Arch-defined, 30 for AArch64 x0-x30
  oldrulecount = dwarf_set_frame_rule_table_size(dbg, 30);
  dwarf_set_frame_rule_table_size(dbg, oldrulecount);

  regtable.rt3_reg_table_size = oldrulecount;
  regtable.rt3_rules = kallocz(sizeof(struct Dwarf_Regtable_Entry3_s) * oldrulecount);
  if (!regtable.rt3_rules) {
    printf("Unable to malloc for %d rules\n", oldrulecount);
    return -1;
  }

  // Get all register value about curpc, fill regtable
  if (dwarf_get_fde_info_for_all_regs3(target_fde, 
        curpc, &regtable, &rowpc, &err) != DW_DLV_OK) {
    LOG_WARN("Register not changed\n");
    kfree(regtable.rt3_rules);
    return -1;
  }

  u64 cfa, ra;
  cfa = cursp + regtable.rt3_cfa_rule.dw_offset_or_block_len;
  // printf("cfa off: %d\n", regtable.rt3_cfa_rule.dw_offset_or_block_len);
  // printf("cfa: 0x%lx\n", cfa);
  // printf("ra off: %d\n", regtable.rt3_rules[30].dw_offset_or_block_len);
  ra = *(u64 *)((s64)cfa + regtable.rt3_rules[30].dw_offset_or_block_len);

  *callersp = cfa;
  *callerpc = ra;
  kfree(regtable.rt3_rules);
  return ra;
}

int 
kmon_symbol_init(void)
{
  Dwarf_Handler errhand = 0;
  Dwarf_Ptr errptr = 0;
  Dwarf_Error err;

  ramdisk_open("spring.elf", &elf_file);

  if (dwarf_init_rd(&elf_file, DW_DLC_READ, 
                errhand, errptr, &dbg, &err) != DW_DLV_OK)
    return -1;

  if (dwarf_get_fde_list(dbg, &cie_data, &cie_element_count, 
                      &fde_data, &fde_element_count, &err) != DW_DLV_OK)
    return -1;

  LOG_DEBUG("cie_element_count: %d, fde_element_count: %d\n", 
                          cie_element_count, fde_element_count);
  return 0;
}