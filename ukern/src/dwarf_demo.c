#include <kernel.h>
#include <kmem.h>
#include <ramdisk.h>
#include <elf.h>
#include <lib/libdwarf.h>
#include <lib/dwarf.h>
#include <lib/libdwarf_ukern.h>

#define TRUE 1
#define FALSE 0


Dwarf_Debug dbg = 0;
Dwarf_Cie *cie_data = NULL;
Dwarf_Signed cie_element_count = 0;
Dwarf_Fde *fde_data = NULL;
Dwarf_Signed fde_element_count = 0;

// Eventually function define
// Call this recursive to unwind stack
int 
find_ra_dwarf(u64 curpc, u64 cursp, u64 *callerpc, u64 *callersp)
{
  Dwarf_Fde target_fde; // Fde about curpc
  Dwarf_Addr low_pc, high_pc; // Address range of fde
  Dwarf_Regtable3 regtable = {0}; // Register value about curpc
  Dwarf_Addr rowpc; // Start pc of row including curpc
  Dwarf_Error err;
  int oldrulecount = 0;

  // dbg,fde_data is global variable
  // Delete me if implement!!
  extern Dwarf_Fde *fde_data;
  extern Dwarf_Debug dbg;

  // Get targer fde about curpc
  if (dwarf_get_fde_at_pc(fde_data, curpc, &target_fde, 
          &low_pc, &high_pc, &err) != DW_DLV_OK) 
    return -1;
  // printf("lowpc: 0x%lx, highpc: 0x%lx\n", low_pc, high_pc);


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
    return -1;
  }

  u64 cfa, ra;
  cfa = *(u64 *)((s64)cursp + regtable.rt3_cfa_rule.dw_offset_or_block_len);
  ra = *(u64 *)((s64)cfa + regtable.rt3_rules[30].dw_offset_or_block_len);

  *callersp = cfa;
  *callerpc = ra;
  return ra;
}


// Output the function parameter information
// [die] Die of subprogram
static void
print_func_param(Dwarf_Die die)
{
  Dwarf_Die child; // Die of TAG_formal_paramter
  Dwarf_Half tag;  // Tag of child
  Dwarf_Error error, *errp;
  int i;

  errp = &error;

  if (dwarf_child(die, &child, errp) != DW_DLV_OK)
    return;

  do {
    if (dwarf_tag(child, &tag, errp) != DW_DLV_OK) {
      printf("Failed to get DIE tag");
      return;
    }

    if (tag != DW_TAG_formal_parameter)
      continue;

    // 获取参数名
    char *name;
    if (dwarf_diename(child, &name, errp) == DW_DLV_OK) {
      printf("Parameter Name: %s\n", name);
      dwarf_dealloc(dbg, name, DW_DLA_STRING);
    }

    // Get type of parameter
    Dwarf_Attribute type_attr;
    Dwarf_Off type_off;
    Dwarf_Die type_die;
    char *typename;
    if (dwarf_attr(child, DW_AT_type, &type_attr, errp) != DW_DLV_OK)
      return;
    if (dwarf_global_formref(type_attr, &type_off, errp) != DW_DLV_OK)
      return;

    if (dwarf_offdie(dbg, type_off, &type_die, errp) != DW_DLV_OK)
      return;

    if (dwarf_diename(type_die, &typename, errp) == DW_DLV_OK) {
      printf("Parameter Type: %s\n", typename);
      dwarf_dealloc(dbg, typename, DW_DLA_STRING);
    }

    // Get value of paramter
    Dwarf_Attribute loc_attr;
    Dwarf_Locdesc **locdesc;
    Dwarf_Signed locdesc_count;

    if (dwarf_attr(child, DW_AT_location, &loc_attr, errp) != DW_DLV_OK)
      return;
    if (dwarf_loclist_n(loc_attr, &locdesc, &locdesc_count, errp)
        != DW_DLV_OK)
      return;
    if (locdesc_count > 0 && locdesc != NULL) {
      for (Dwarf_Signed i = 0; i < locdesc[0]->ld_cents; ++i) {
        if (locdesc[0]->ld_s[i].lr_atom == DW_OP_fbreg) {
          printf("DW_OP_fbreg offset: %ld\n", locdesc[0]->ld_s[i].lr_number);
          break;
        }
      }
    }
    // 释放 locdesc
    for (i = 0; i < locdesc_count; ++i) {
      /*  Use llbuf[i]. Both Dwarf_Locdesc and the
          array of Dwarf_Loc it points to are
          defined in libdwarf.h: they are
          not opaque structs. */
      dwarf_dealloc(dbg, locdesc[i]->ld_s, DW_DLA_LOC_BLOCK);
      dwarf_dealloc(dbg, locdesc[i], DW_DLA_LOCDESC);
    }
    dwarf_dealloc(dbg, locdesc, DW_DLA_LIST);

    // 获取参数的类型信息等其他信息，你可以根据需要进行扩展
    printf("------------------------------\n");

    print_func_param(child);
  } while (dwarf_siblingof(dbg, child, &child, errp) == DW_DLV_OK);

}




// Try to find function info in one cu
// [die] Die of cu
// return 0 if found, < 0 for not found
static int
find_func_cu(Dwarf_Die die, u64 pc)
{
  Dwarf_Die child;
  Dwarf_Error err, *errp;
  char *cuname;

  errp = &err;

  if (dwarf_child(die, &child, errp) != DW_DLV_OK) 
    return -1;

  if (dwarf_diename(die, &cuname, &err) != DW_DLV_OK) {
    LOG_ERROR("Faild to get cudie name\n");
    return -1;
  }

  printf("cuname: %s\n", cuname);

  do {
    Dwarf_Addr lowpc, highpc;
    Dwarf_Half highpc_form, tag;
    enum Dwarf_Form_Class highpc_class;
    char *progname;

    if (dwarf_tag(child, &tag, &err) != DW_DLV_OK) {
      LOG_ERROR("Failed to get DIE tag\n");
      return -1;
    }
    if (tag != DW_TAG_subprogram)
      continue;

    if (dwarf_diename(child, &progname, &err) != DW_DLV_OK) {
      LOG_ERROR("Faild to get die name\n");
      return -1;
    }

    printf("searching prog: %s\n", progname);

    if (dwarf_lowpc(child, &lowpc, &err) == DW_DLV_OK &&
        dwarf_highpc_b(child, &highpc, &highpc_form, &highpc_class, &err) == DW_DLV_OK) {

      if (highpc_form != DW_FORM_addr) 
        highpc = lowpc+highpc; // returned highpc if offset

      // printf("highpc_form: %d, highpc_class: %d\n", highpc_form, highpc_class);
      // printf("lowpc: 0x%lx, highpc:0x%lx\n", lowpc, highpc);
      if (lowpc <= pc && highpc >= pc) {
        printf("find!!\n\n");
        print_func_param(child);
        return 0;
      }
    }
  } while (dwarf_siblingof(dbg, child, &child, &err) == DW_DLV_OK);

  return -1;
}

// Output information about function,
// Like name, paramater type, param position...
// [pc] must be in function
// Return -1 in error, 0 for success
int
find_func(unsigned long pc)
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
        if  (res == DW_DLV_ERROR) {
            char *em = errp?dwarf_errmsg(error):"An error next cu her";
            printf("Error in dwarf_next_cu_header: %s\n",em);
            return -1;
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

        if (find_func_cu(cu_die, pc) == 0) {
          dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
          return 0;
        }
        // Recycle memory for this cu_die
        dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
    }
    return -1;
}

// Init global variable
int 
init_libdwarf(void)
{
  Dwarf_Handler errhand = 0;
  Dwarf_Ptr errptr = 0;
  Dwarf_Error err;
  struct ramdisk_file file;

  ramdisk_open("spring.elf", &file);

  if (dwarf_init_rd(&file, DW_DLC_READ, errhand, errptr, &dbg, &err) != DW_DLV_OK)
    return -1;

  find_func(0xffff0000400274f0);
  
  // Exit qemu for test quickly
  exit();
  return 0;
}

// Free all memory allocated by dwarf_init()
int 
free_libdwarf(void)
{
  Dwarf_Error err;

  if (dwarf_finish_rd(dbg, &err) != DW_DLV_OK)
    return -1;
  return 0;
}


#if 0
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



void
print_one_cie(Dwarf_Debug dbg, Dwarf_Cie cie,
    Dwarf_Unsigned cie_index, Dwarf_Half address_size)
{
  Dwarf_Unsigned cie_length = 0;
  Dwarf_Small version = 0;
  char* augmenter = "";
  Dwarf_Unsigned code_alignment_factor = 0;
  Dwarf_Signed data_alignment_factor = 0;
  Dwarf_Half return_address_register_rule = 0;
  Dwarf_Ptr initial_instructions = 0;
  Dwarf_Unsigned initial_instructions_length = 0;
  Dwarf_Off cie_off = 0;

  Dwarf_Error err = 0;
  int ret=0;


  ret = dwarf_get_cie_info(cie,
      &cie_length,
      &version,
      &augmenter,
      &code_alignment_factor,
      &data_alignment_factor,
      &return_address_register_rule,
      &initial_instructions,
      &initial_instructions_length, &err);
  if (ret == DW_DLV_ERROR) {
    LOG_ERROR("Get CIE info\n");
    return;
  }
  if (ret == DW_DLV_NO_ENTRY) {
    LOG_ERROR("Impossible DW_DLV_NO_ENTRY on cie %" DW_PR_DUu "\n");
    return;
  }

  printf("<%5u>\tversion\t\t\t\t%d\n", cie_index, version);
  ret = dwarf_cie_section_offset(dbg, cie, &cie_off, &err);
  if (ret == DW_DLV_OK) {
    printf("\tcie section offset\t\t%lu 0x%08lx\n",
           (Dwarf_Unsigned)cie_off, (Dwarf_Unsigned)cie_off);
  }

  // printf("\taugmentation\t\t\t%s\n", sanitized(augmenter));
  printf("\tcode_alignment_factor\t\t%lu\n", code_alignment_factor);
  printf("\tdata_alignment_factor\t\t%ld\n", data_alignment_factor);
  printf("\treturn_address_register\t\t%d\n", return_address_register_rule);

  printf("\tbytes of initial instructions\t%lu\n",
         initial_instructions_length);
  printf("\tcie length\t\t\t%lu\n", cie_length);
  /*  For better layout */
  printf("\tinitial instructions\n");
}

static void
print_fde_instrs(Dwarf_Debug dbg, Dwarf_Fde fde)
{
    int res;
    Dwarf_Addr lowpc = 0;
    Dwarf_Unsigned func_length = 0;
    Dwarf_Ptr fde_bytes;
    Dwarf_Unsigned fde_byte_length = 0;
    Dwarf_Off cie_offset = 0;
    Dwarf_Signed cie_index = 0;
    Dwarf_Off fde_offset = 0;
    Dwarf_Addr arbitrary_addr = 0;
    Dwarf_Addr actual_pc = 0;
    Dwarf_Regtable3 tab3;
    int oldrulecount = 0;
    Dwarf_Ptr outinstrs = 0;
    Dwarf_Unsigned instrslen = 0;
    Dwarf_Frame_Op * frame_op_list = 0;
    Dwarf_Signed frame_op_count = 0;
    Dwarf_Cie cie = 0;
    Dwarf_Error err;

    res = dwarf_get_fde_range(fde,&lowpc,&func_length,&fde_bytes,
        &fde_byte_length,&cie_offset,&cie_index,&fde_offset,&err);
    if(res != DW_DLV_OK) {
        printf("Problem getting fde range \n");
        exit(1);
    }

    arbitrary_addr = lowpc + (func_length/2);
    printf("function low pc 0x%lx"
        "  and length 0x%lx"
        "  and addr we choose 0x%lx"
        "\n",
        lowpc,func_length,arbitrary_addr);

}
void
print_frames(Dwarf_Debug dbg)
{
  Dwarf_Half address_size = 0;
  Dwarf_Error err = 0;
  int i,ret=0;

  ret = dwarf_get_address_size(dbg, &address_size, &err);
  if (ret != DW_DLV_OK) {
    LOG_ERROR("dwarf_get_address_size\n");
    return;
  }

  Dwarf_Cie *cie_data = NULL;
  Dwarf_Signed cie_element_count = 0;
  Dwarf_Fde *fde_data = NULL;
  Dwarf_Signed fde_element_count = 0;
  const char *frame_section_name = 0;
  int cie_count = 0;

  ret = dwarf_get_frame_section_name(dbg, &frame_section_name, &err);
  if (ret != DW_DLV_OK || !frame_section_name || !strlen(frame_section_name)) {
    LOG_WARN("Get none frame section name, set manually\n");
    frame_section_name = ".debug_frame";
  }
  LOG_DEBUG("address size: %d\n", address_size);
  LOG_DEBUG("frame section name: %s\n", frame_section_name);

  ret = dwarf_get_fde_list(dbg, &cie_data, &cie_element_count, &fde_data,
                            &fde_element_count, &err);
  if (ret != DW_DLV_OK) {
    return;
  }
  LOG_DEBUG("cie_element_count: %d, fde_element_count: %d\n", 
              cie_element_count, fde_element_count);

  printf("\nfde:\n");
  for (i = 0; i < fde_element_count; i++) {
    print_fde_instrs(dbg, fde_data[i]);
  }
  // printf("\ncie:\n");
  // for (i = 0; i < cie_element_count; i++) {
  //   // print_one_cie(dbg, cie_data[i], i, address_size);
  // }
}

#endif

