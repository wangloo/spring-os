#include <kernel.h>
#include <kmem.h>
#include <ramdisk.h>
#include <elf.h>
#include <lib/libdwarf.h>
#include <lib/dwarf.h>
#include <lib/libdwarf_ukern.h>
#include <slab.h>

#define TRUE 1
#define FALSE 0

enum {
  RET_OK = 0,
  RET_ERR = -1,
  RET_NOT_FOUND = -2,
};


// TODO
struct dbgi_var {
  char *name;
  char *typename;
  int size;

  // loc_type and loc_val decide location 
  // of the var together. For local var, 
  // loc_val might be offset of sp/fp.
  int loc_type;
  unsigned long loc_val;

  struct dbgi_var *next;
};

struct dbgi_func_param {
  char **name; // TODO
  int *size;
  int *offset;
  int count;
};
struct dbgi_func_loc {
  char *file;
  long line;
};

struct dbgi_func {
  unsigned long lowpc;
  unsigned long highpc;
  char *name;
  struct dbgi_func_loc *loc;
  struct dbgi_func_param *param;

  struct dbgi_func *next;
};

static struct dbgi_func func_head;
static struct dbgi_var  var_head; // TODO

struct ramdisk_file elf_file;
static Dwarf_Debug dbg;
static Dwarf_Cie *cie_data = NULL;
static Dwarf_Signed cie_element_count = 0;
static Dwarf_Fde *fde_data = NULL;
static Dwarf_Signed fde_element_count = 0;

// These are used when finding line number.
// Because using the frame of walk_all_die(),
// it's inconvenient to pass parameters in and out.
// Maybe better designed in future.
static int targetline;
static unsigned long targetpc;
static char *targetfile;
static int targetline_found;

typedef int (*die_handler)(Dwarf_Debug dbg, Dwarf_Die cudie, Dwarf_Die die);
typedef int (*cu_filter_t)(Dwarf_Debug dbg, Dwarf_Die cudie);

static inline void
alloc_copy_str(char **dst, char *src)
{
  int len = strlen(src);
  *dst = kalloc(len+1);
  strcpy(*dst, src);
}

static inline int
install_func_pc(struct dbgi_func *df, 
            Dwarf_Debug dbg, Dwarf_Die cudie, Dwarf_Die funcdie)
{

  Dwarf_Addr lowpc, highpc;
  Dwarf_Half highpc_form;
  enum Dwarf_Form_Class highpc_class;
  Dwarf_Error err;

  // TODO: Support record inline function
  // Dwarf_Attribute inline_attr;
  // Dwarf_Signed inline_value;
  // if (dwarf_attr(funcdie, DW_AT_byte_size, &inline_attr, errp) == DW_DLV_OK) {
  //   if (dwarf_formudata(inline_attr, (Dwarf_Unsigned *)&inline_value, errp) != DW_DLV_OK)
  //     return -1;
  //   if ((inline_value == DW_INL_inlined) || 
  //       (inline_value == DW_INL_declared_inlined)) {
  //     df->highpc = df->lowpc = -1;
  //     return 0;
  //   }
  // }

  if (dwarf_lowpc(funcdie, &lowpc, &err) == DW_DLV_OK && 
      dwarf_highpc_b(funcdie, &highpc, &highpc_form, &highpc_class, &err) == DW_DLV_OK) {
    if (highpc_form != DW_FORM_addr)
      highpc = lowpc + highpc; // returned highpc if offset

    // printf("highpc_form: %d, highpc_class: %d\n", highpc_form, highpc_class);
    // printf("lowpc: 0x%lx, highpc:0x%lx\n", lowpc, highpc);
    df->highpc = highpc;
    df->lowpc = lowpc;
  } else {
    LOG_ERROR("Can't get address region\n");
    kfree(df);
    return -1;
  }
  return 0;
}
static inline int
install_func_param(struct dbgi_func *df, 
            Dwarf_Debug dbg, Dwarf_Die cudie, Dwarf_Die funcdie)
{
  Dwarf_Die paramdie = 0; // Die of TAG_formal_paramter
  Dwarf_Half tag = 0;
  Dwarf_Error err;
  Dwarf_Error *errp=&err;
  int *poff, *psize;
  int i, pcount=0;

  poff = kalloc(8 * sizeof(unsigned int *));
  psize = kalloc(8 * sizeof(unsigned int *));
  if (dwarf_child(funcdie, &paramdie, errp) != DW_DLV_OK) {
    // LOG_WARN("Func has no parameter\n");
    goto out;
  }

  do {
    if (dwarf_tag(paramdie, &tag, errp) != DW_DLV_OK) 
      return -1;

    if (tag != DW_TAG_formal_parameter)
      continue;

    // 获取参数名
    // char *param_name;
    // if (dwarf_diename(paramdie, &param_name, errp) == DW_DLV_OK) {
    //   printf("Parameter Name: %s\n", param_name);
    //   dwarf_dealloc(dbg, param_name, DW_DLA_STRING);
    // }

    // Get type of parameter
    Dwarf_Attribute type_attr, size_attr;
    Dwarf_Half type_tag;
    Dwarf_Off type_off;
    Dwarf_Die type_die;
    // Dwarf_Unsigned type_size;
    if (dwarf_attr(paramdie, DW_AT_type, &type_attr, errp) != DW_DLV_OK) {
      return -1;
    }
    if (dwarf_global_formref(type_attr, &type_off, errp) != DW_DLV_OK) {
      return -1;
    }
    if (dwarf_offdie(dbg, type_off, &type_die, errp) != DW_DLV_OK) {

      return -1;
    }

    // For paramter which type are not directly,
    // only the original type has 'AT_bytesize' attribute
    if (dwarf_tag(type_die, &type_tag, errp) != DW_DLV_OK) {
      dwarf_dealloc(dbg, type_die, DW_DLA_DIE);
      return -1;
    }
    while (type_tag == DW_TAG_typedef || type_tag == DW_TAG_const_type
           || type_tag == DW_TAG_restrict_type) {
      if (dwarf_attr(type_die, DW_AT_type, &type_attr, errp) != DW_DLV_OK)
        return -1;
      if (dwarf_global_formref(type_attr, &type_off, errp) != DW_DLV_OK)
        return -1;
      if (dwarf_offdie(dbg, type_off, &type_die, errp) != DW_DLV_OK)
        return -1;

      if (dwarf_tag(type_die, &type_tag, errp) != DW_DLV_OK) {
        return -1;
      }
    }

    // For debug
    // char *typename;
    // if (dwarf_diename(type_die, &typename, errp) == DW_DLV_OK) {
    //   printf("Parameter Type: %s\n", typename);
    //   dwarf_dealloc(dbg, typename, DW_DLA_STRING);
    // }

    if (dwarf_attr(type_die, DW_AT_byte_size, &size_attr, errp) != DW_DLV_OK)
      return -1;
    if (dwarf_formudata(size_attr, (Dwarf_Unsigned *)&psize[pcount], errp) != DW_DLV_OK)
      return -1;
    dwarf_dealloc(dbg, type_die, DW_DLA_DIE);
    // printf("====>\n");

    // Get location of paramter
    Dwarf_Attribute loc_attr;
    Dwarf_Locdesc **locdesc;
    Dwarf_Signed locdesc_count;

    if (dwarf_attr(paramdie, DW_AT_location, &loc_attr, errp) != DW_DLV_OK) 
      return -1;
    if (dwarf_loclist_n(loc_attr, &locdesc, &locdesc_count, errp) != DW_DLV_OK)
      return -1;
    if (locdesc_count > 0 && locdesc != NULL) {
      for (Dwarf_Signed i = 0; i < locdesc[0]->ld_cents; ++i) {
        if (locdesc[0]->ld_s[i].lr_atom == DW_OP_fbreg) {
          // printf("DW_OP_fbreg offset: %ld\n", locdesc[0]->ld_s[i].lr_number);
          // psize[pcount] = 4;
          poff[pcount] = locdesc[0]->ld_s[i].lr_number;
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
    // printf("------------------------------\n");

    pcount += 1;
  } while (dwarf_siblingof(dbg, paramdie, &paramdie, errp) == DW_DLV_OK);

out:
  df->param = kalloc(sizeof(*(df->param)));
  df->param->count = pcount;
  df->param->offset = poff;
  df->param->size = psize;
  return 0;
}

static inline int
install_func_loc(struct dbgi_func *df, 
            Dwarf_Debug dbg, Dwarf_Die cudie, Dwarf_Die funcdie)
{
  Dwarf_Error err;
  char *progname, *filename;

  if (dwarf_diename(cudie, &filename, &err) != DW_DLV_OK) {
    LOG_ERROR("Faild to get subprogram name\n");
    return RET_ERR;
  }
  if (dwarf_diename(funcdie, &progname, &err) != DW_DLV_OK) {
    LOG_ERROR("Faild to get die name\n");
    return RET_ERR;
  }

  // For debug only
  // printf("searching prog: %s\n", progname);

  df->loc = kalloc(sizeof(*(df->loc)));
  alloc_copy_str(&df->name, progname);
  alloc_copy_str(&(df->loc->file), filename);
  return 0;
}

int
func_install_handler(Dwarf_Debug dbg, Dwarf_Die cudie, Dwarf_Die funcdie)
{
  Dwarf_Attribute attr;
  Dwarf_Signed attr_value;
  Dwarf_Half tag=0;
  Dwarf_Error err, *errp=&err;
  struct dbgi_func *df;

  // For debug
  // Dwarf_Unsigned offset;
  // dwarf_dieoffset(funcdie, &offset, errp);
  // LOG_DEBUG("Now handle tag <%x>\n", offset);


  if (dwarf_tag(funcdie, &tag, &err) != DW_DLV_OK) {
    LOG_ERROR("Failed to get DIE tag\n");
    return -1;
  }

  // Exclude non-function
  if (tag != DW_TAG_subprogram)
    return 0;
  
  // Exclude function declaration
  if (dwarf_attr(funcdie, DW_AT_declaration, &attr, errp) == DW_DLV_OK)
    return 0;
  // Exclude abstrace function
  if (dwarf_attr(funcdie, DW_AT_abstract_origin, &attr, errp) == DW_DLV_OK)
    return 0;  

  // Exclude inline function
  if (dwarf_attr(funcdie, DW_AT_inline, &attr, errp) == DW_DLV_OK) {
    if (dwarf_formudata(attr, (Dwarf_Unsigned *)&attr_value, errp) != DW_DLV_OK) {
      LOG_ERROR("Has inline attr, but get value err\n");
      return -1;
    }
    if ((attr_value == DW_INL_inlined) || 
        (attr_value == DW_INL_declared_inlined))
      return 0;
  }

  df = kallocz(sizeof(*df));

  // printf("Handling location...\n");
  // Fill func's location
  if (install_func_loc(df, dbg, cudie, funcdie) < 0)
    return -1;

  // printf("Handing instruction region...\n");
  // Fill func's instruction region
  if (install_func_pc(df, dbg, cudie, funcdie) < 0)
    return -1;

  // printf("Handing paramaters...\n");
  // Fill function paramters
  if (install_func_param(df, dbg, cudie, funcdie) < 0)
    return -1;

  df->next = func_head.next;
  func_head.next = df;
  return 0;
}


static int
walk_all_die_cu(Dwarf_Debug dbg, Dwarf_Die cu_die, die_handler handler)
{
  Dwarf_Die child;
  Dwarf_Error err;
  Dwarf_Error *errp=&err;
  int ret;

  ret = dwarf_child(cu_die, &child, errp);
  if (ret == DW_DLV_NO_ENTRY) 
    return RET_NOT_FOUND;
  else if (ret != DW_DLV_OK)
    return RET_ERR;

  // Exclude cu of thirdparty
  char *cuname;
  if (dwarf_diename(cu_die, &cuname, &err) != DW_DLV_OK) {
    LOG_ERROR("Faild to get cudie name\n");
    return RET_ERR;
  }
  if (strstr(cuname, "/lib") || strstr(cuname, "/kmon") || 
      strstr(cuname, "/test")) {
    dwarf_dealloc(dbg,cuname, DW_DLA_STRING);
    return RET_NOT_FOUND;
  }
  dwarf_dealloc(dbg,cuname, DW_DLA_STRING);

  // For debug only
  // printf("cuname: %s\n", cuname);

  do {
    if (handler(dbg, cu_die, child) < 0) {
      return -1;
    }
  } while (dwarf_siblingof(dbg, child, &child, &err) == DW_DLV_OK);

  return 0;
}

static int
walk_all_die(Dwarf_Debug dbg, die_handler handler, cu_filter_t cu_filter)
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
            // All cu are walked through
            goto out;
        }

        /* The CU will have a single sibling, a cu_die. */
        res = dwarf_siblingof_b(dbg,no_die,is_info, &cu_die,errp);
        if (res == DW_DLV_ERROR) {
            char *em = errp?dwarf_errmsg(error):"An error";
            printf("Error in dwarf_siblingof_b on CU die: %s\n",em);
            return -1;
        } if(res == DW_DLV_NO_ENTRY) {
            /* Impossible case. */
            printf("no entry! in dwarf_siblingof on CU die \n");
            return -1;
        }

        if (cu_filter && cu_filter(dbg, cu_die)) {
          dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
          continue;
        }

        res = walk_all_die_cu(dbg, cu_die, handler);
        dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
        if (res == RET_ERR) return res;
    }

out:
  return RET_OK;
}


int
dgbinfo_get_func_loc(unsigned long pc, char **name, char **file, int *line)
{
  struct dbgi_func *df = func_head.next;
  struct dbgi_func_loc *dfl;

  while (df) {
    if (pc >= df->lowpc && pc < df->highpc) {
      dfl = df->loc;
      if (name) *name = df->name;
      if (file) *file = dfl->file;
      if (line) *line = dfl->line;
      return 0;
    }
    df = df->next;
  }
  return -1;
}

int
dbgsym_func_name2addr(char *name, unsigned long *pc)
{
  struct dbgi_func *df = func_head.next;
  struct dbgi_func_loc *dfl;

  while (df) {
    if (df->name && strcmp(df->name, name)==0) {
      *pc = df->lowpc;
      return 0;
    }
    df = df->next;
  }
  return -1;
}

int
dgbinfo_get_func_param(unsigned long pc, int *argc, int **offset, int **size)
{
  struct dbgi_func *df = func_head.next;
  struct dbgi_func_param *dfp;

  while (df) {
    if (pc >= df->lowpc && pc < df->highpc) {
      dfp = df->param;
      if (!dfp) 
        return -1;
      *argc = dfp->count;
      *offset = dfp->offset;
      *size = dfp->size;
      return 0;
    }
    df = df->next;
  }
  return -1;
}



int
dbginfo_get_caller(u64 curpc, u64 cursp, u64 *callerpc, u64 *callersp)
{
  Dwarf_Fde target_fde;       // Fde about curpc
  Dwarf_Addr low_pc, high_pc; // Address range of fde
  Dwarf_Regtable3 regtable = {0}; // Register value about curpc
  Dwarf_Addr rowpc; // Start pc of row including curpc
  Dwarf_Error err;
  int oldrulecount = 0;

  // Get targer fde about curpc
  if (dwarf_get_fde_at_pc(fde_data, curpc, &target_fde, 
          &low_pc, &high_pc, &err) != DW_DLV_OK) 
    return -1;
  // For debug
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
    LOG_WARN("Register not changed\n");
    kfree(regtable.rt3_rules);
    return -1;
  }

  u64 cfa_off, ra_off;
  cfa_off = regtable.rt3_cfa_rule.dw_offset_or_block_len;
  ra_off = regtable.rt3_rules[30].dw_offset_or_block_len;
  
  // printf("cfa off: %d\n", cfa_off);
  // printf("ra off: %d\n", ra_off);
  *callersp = cursp + cfa_off;
  if (!ra_off)
    *callerpc = 0;
  else
    *callerpc = *(u64 *)((s64)(*callersp) + ra_off);
  kfree(regtable.rt3_rules);
  return 0;
}


static int
func_get_line_handler(Dwarf_Debug dbg, Dwarf_Die cudie, Dwarf_Die funcdie)
{
  enum Dwarf_Form_Class highpc_class;
  Dwarf_Addr lowpc, highpc;
  Dwarf_Half highpc_form;
  Dwarf_Half tag=0;
  Dwarf_Line *linebuf;
  Dwarf_Signed linecount=0;
  Dwarf_Unsigned lineno;
  Dwarf_Addr lineaddr, endaddr;
  Dwarf_Error err;
  Dwarf_Error *errp=&err;
  int i;

  if (dwarf_tag(funcdie, &tag, &err) != DW_DLV_OK) {
    LOG_ERROR("Failed to get DIE tag\n");
    return -1;
  }

  // Exclude non-function
  if (tag != DW_TAG_subprogram)
    return 0;
  // Exclude die which pc not inside
  if (dwarf_lowpc(funcdie, &lowpc, &err) == DW_DLV_OK && 
      dwarf_highpc_b(funcdie, &highpc, &highpc_form, &highpc_class, &err) == DW_DLV_OK) {
    if (highpc_form != DW_FORM_addr)
      highpc = lowpc + highpc; // returned highpc if offset
    if (targetpc < lowpc || targetpc >= highpc)
      return 0;
  } else {
    TODO();
  }

  // Handle line number last
  if (dwarf_srclines(cudie, &linebuf, &linecount, errp) != DW_DLV_OK) {
    return RET_ERR;
  }
  for (i = 0; i < linecount; i++) {
    if (dwarf_lineaddr(linebuf[i], &lineaddr, errp) != DW_DLV_OK ||
        dwarf_lineno(linebuf[i], &lineno, errp) != DW_DLV_OK) {
      dwarf_srclines_dealloc(dbg, linebuf, linecount);
      return RET_ERR;
    }

    // Get the end address of the line, if available
    if (i + 1 < linecount) {
        if (dwarf_lineaddr(linebuf[i + 1], &endaddr, errp) != DW_DLV_OK) {
            dwarf_srclines_dealloc(dbg, linebuf, linecount);
            return RET_ERR;
        }
    } else {
        // TODO: In last row, how to caculate end addr?
        endaddr = lineaddr + 4; 
    }

    // printf("lineaddr: %lx, endaddr: %lx, lineno: %d\n", lineaddr, endaddr, lineno);
    if (targetpc >= lineaddr && targetpc < endaddr) { // Yeah, we find it!
      targetline_found = 1;
      break;
    }
  }
  dwarf_srclines_dealloc(dbg, linebuf, linecount);

  if (i != linecount) {
    targetline = lineno;
  }
  return 0;
}

// If 1 is returned, won't go through all dies
int
func_get_line_cu_filter(Dwarf_Debug dbg, Dwarf_Die cudie)
{
  Dwarf_Error err;
  char *cuname;

  if (targetline_found) return 1;

  // The filter to only walk cu of target file
  dwarf_diename(cudie, &cuname, &err);
  if (!strstr(cuname, targetfile)) return 1;


  return 0;
}

// Ugly design!!
// For now, line number info is not stored in struct dbgi_func,
// because I haven't figued out how to store them.
// So we must walk all die to find target pc's line number.
// Fortunitely, [file] is provided so we can use filter
// to avoid walk all die, which is too waste of time.
int
dbginfo_get_func_lineno(unsigned long pc, char *file, int *line)
{
  targetpc = pc;
  targetfile = file;
  targetline_found = 0;
  if (walk_all_die(dbg, func_get_line_handler, func_get_line_cu_filter) < 0)
    return -1;

  if (!targetline_found) {
    LOG_ERROR("targetpc: %lx of %s not found lineno\n", targetpc, targetfile);
    return -1;
  }
  *line = targetline;
  return 0;
}


int
dgbinfo_init_all_func(Dwarf_Debug dbg)
{
  if (walk_all_die(dbg, func_install_handler, NULL) < 0)
    return -1;
  return 0;
}

int
init_dbginfo(void)
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
  
  if (dgbinfo_init_all_func(dbg) < 0)
    return -1;

  // if (dwarf_finish_rd(dbg, &err) != DW_DLV_OK)
  //   return -1;

  return 0;
}