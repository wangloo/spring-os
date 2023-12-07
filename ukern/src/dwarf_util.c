#include <kernel.h>
#include <kmem.h>
#include <ramdisk.h>
#include <elf.h>
#include <lib/libdwarf.h>
#include <lib/dwarf.h>
#include <lib/libdwarf_ukern.h>


#define UNDEF_VAL 2000
#define SAME_VAL 2001
#define CFA_VAL 2002


static void
print_reg(int r)
{
   switch(r) {
   case SAME_VAL:
        printf(" %d SAME_VAL ",r);
        break;
   case UNDEF_VAL:
        printf(" %d UNDEF_VAL ",r);
        break;
   case CFA_VAL:
        printf(" %d (CFA) ",r);
        break;
   default:
        printf(" r%d ",r);
        break;
   }
}

static void
print_one_regentry(const char *prefix,
    struct Dwarf_Regtable_Entry3_s *entry)
{
    int is_cfa = !strcmp("cfa",prefix);
    printf("%s ",prefix);
    printf("type: %d %s ",
        entry->dw_value_type,
        (entry->dw_value_type == DW_EXPR_OFFSET)? "DW_EXPR_OFFSET":
        (entry->dw_value_type == DW_EXPR_VAL_OFFSET)? "DW_EXPR_VAL_OFFSET":
        (entry->dw_value_type == DW_EXPR_EXPRESSION)? "DW_EXPR_EXPRESSION":
        (entry->dw_value_type == DW_EXPR_VAL_EXPRESSION)?
            "DW_EXPR_VAL_EXPRESSION":
            "Unknown");
    switch(entry->dw_value_type) {
    case DW_EXPR_OFFSET:
        print_reg(entry->dw_regnum);
        printf(" offset_rel? %d ",entry->dw_offset_relevant);
        if(entry->dw_offset_relevant) {
            printf(" offset  %ld " ,
                entry->dw_offset_or_block_len);
            if(is_cfa) {
                printf("defines cfa value");
            } else {
                printf("address of value is CFA plus signed offset");
            }
            if(!is_cfa  && entry->dw_regnum != CFA_VAL) {
                printf(" compiler botch, regnum != CFA_VAL");
            }
        } else {
            printf("value in register");
        }
        break;
    case DW_EXPR_VAL_OFFSET:
        print_reg(entry->dw_regnum);
        printf(" offset  %" DW_PR_DSd " " ,
            entry->dw_offset_or_block_len);
        if(is_cfa) {
            printf("does this make sense? No?");
        } else {
            printf("value at CFA plus signed offset");
        }
        if(!is_cfa  && entry->dw_regnum != CFA_VAL) {
            printf(" compiler botch, regnum != CFA_VAL");
        }
        break;
    case DW_EXPR_EXPRESSION:
        print_reg(entry->dw_regnum);
        printf(" offset_rel? %d ",entry->dw_offset_relevant);
        printf(" offset  %" DW_PR_DSd " " ,
            entry->dw_offset_or_block_len);
        printf("Block ptr set? %s ",entry->dw_block_ptr?"yes":"no");
        printf(" Value is at address given by expr val ");
        /* printf(" block-ptr  0x%" DW_PR_DUx " ",
            (Dwarf_Unsigned)entry->dw_block_ptr); */
        break;
    case DW_EXPR_VAL_EXPRESSION:
        printf(" expression byte len  %" DW_PR_DSd " " ,
            entry->dw_offset_or_block_len);
        printf("Block ptr set? %s ",entry->dw_block_ptr?"yes":"no");
        printf(" Value is expr val ");
        if(!entry->dw_block_ptr) {
            printf("Compiler botch. ");
        }
        /* printf(" block-ptr  0x%" DW_PR_DUx " ",
            (Dwarf_Unsigned)entry->dw_block_ptr); */
        break;
    }
    printf("\n");
}


void
print_regtable(Dwarf_Regtable3 *tab3)
{
    int r;
    /* We won't print too much. A bit arbitrary. */
    int max = 31;
    if(max > tab3->rt3_reg_table_size) {
        max = tab3->rt3_reg_table_size;
    }
    print_one_regentry("cfa",&tab3->rt3_cfa_rule);

    for(r = 0; r < max; r++) {
        char rn[30];
        snprintf(rn,sizeof(rn),"reg %d",r);
        print_one_regentry(rn,tab3->rt3_rules+r);
    }
}