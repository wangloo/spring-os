#pragma once
#include <utils.h>

#define CPACR_EL1_FPEN    (UL(3) << 20)

#define SCTLR_EL1_UCI     (1 << 26) /* Traps EL0 execution of cache maintenance instructions to EL1, from AArch64 state only */
#define SCTLR_ELx_EE      (1 << 25) /* Endianness of data accesses at ELx */
#define SCTLR_EL1_E0E     (1 << 24) /* Endianness of data accesses at EL0 */
#define SCTLR_ELx_WXN     (1 << 19) /* Write permission implies XN (Execute-never) */
#define SCTLR_EL1_nTWE    (1 << 18) /* Traps EL0 execution of WFE instructions to EL1, from both Execution states */
#define SCTLR_EL1_nTWI    (1 << 16) /* Traps EL0 execution of WFI instructions to EL1, from both Execution states */
#define SCTLR_EL1_UCT     (1 << 15) /* Traps EL0 accesses to the CTR_EL0 to EL1, from AArch64 state only */
#define SCTLR_EL1_DZE     (1 << 14) /* Traps EL0 execution of DC ZVA instructions to EL1, from AArch64 state only */
#define SCTLR_ELx_I       (1 << 12) /* Instruction access Cacheability control, for accesses at EL0 and EL1 */
#define SCTLR_EL1_UMA     (1 << 9)  /* User Mask Access: Traps EL0 execution of MSR and MRS instructions that access the PSTATE.{D, A, I, F} */
#define SCTLR_EL1_nAA     (1 << 6)  /* Non-aligned access. This bit controls generation of Alignment faults at EL1 and EL0 under certain conditions */
#define SCTLR_EL1_CP15BEN (1 << 5)  /* System instruction memory barrier enable (AArch32) */
#define SCTLR_EL1_SA0     (1 << 4)  /* SP Alignment check enable for EL0 */
#define SCTLR_ELx_SA      (1 << 3)  /* SP Alignment check */
#define SCTLR_ELx_C       (1 << 2)  /* Cacheability control for data accesses */
#define SCTLR_ELx_A       (1 << 1)  /* Alignment check enable */
#define SCTLR_ELx_M       (1 << 0)  /* MMU enable for address translation */

/*
 * TCR flags.
 */
#define TCR_T0SZ_OFFSET   0
#define TCR_T1SZ_OFFSET   16
#define TCR_T0SZ(x)       ((UL(64) - (x)) << TCR_T0SZ_OFFSET)
#define TCR_T1SZ(x)       ((UL(64) - (x)) << TCR_T1SZ_OFFSET)
#define TCR_TxSZ(x)       (TCR_T0SZ(x) | TCR_T1SZ(x))
#define TCR_TxSZ_WIDTH    6
#define TCR_T0SZ_MASK     (((UL(1) << TCR_TxSZ_WIDTH) - 1) << TCR_T0SZ_OFFSET)

#define TCR_IRGN0_SHIFT   8
#define TCR_IRGN0_MASK    (UL(3) << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_NC      (UL(0) << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WBWA    (UL(1) << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WT      (UL(2) << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WBnWA   (UL(3) << TCR_IRGN0_SHIFT)

#define TCR_IRGN1_SHIFT   24
#define TCR_IRGN1_MASK    (UL(3) << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_NC      (UL(0) << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WBWA    (UL(1) << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WT      (UL(2) << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WBnWA   (UL(3) << TCR_IRGN1_SHIFT)

#define TCR_IRGN_NC       (TCR_IRGN0_NC | TCR_IRGN1_NC)
#define TCR_IRGN_WBWA     (TCR_IRGN0_WBWA | TCR_IRGN1_WBWA)
#define TCR_IRGN_WT       (TCR_IRGN0_WT | TCR_IRGN1_WT)
#define TCR_IRGN_WBnWA    (TCR_IRGN0_WBnWA | TCR_IRGN1_WBnWA)
#define TCR_IRGN_MASK     (TCR_IRGN0_MASK | TCR_IRGN1_MASK)

#define TCR_ORGN0_SHIFT   10
#define TCR_ORGN0_MASK    (UL(3) << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_NC      (UL(0) << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WBWA    (UL(1) << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WT      (UL(2) << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WBnWA   (UL(3) << TCR_ORGN0_SHIFT)

#define TCR_ORGN1_SHIFT   26
#define TCR_ORGN1_MASK    (UL(3) << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_NC      (UL(0) << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WBWA    (UL(1) << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WT      (UL(2) << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WBnWA   (UL(3) << TCR_ORGN1_SHIFT)

#define TCR_ORGN_NC       (TCR_ORGN0_NC | TCR_ORGN1_NC)
#define TCR_ORGN_WBWA     (TCR_ORGN0_WBWA | TCR_ORGN1_WBWA)
#define TCR_ORGN_WT       (TCR_ORGN0_WT | TCR_ORGN1_WT)
#define TCR_ORGN_WBnWA    (TCR_ORGN0_WBnWA | TCR_ORGN1_WBnWA)
#define TCR_ORGN_MASK     (TCR_ORGN0_MASK | TCR_ORGN1_MASK)

#define TCR_SH0_SHIFT     12
#define TCR_SH0_MASK      (UL(3) << TCR_SH0_SHIFT)
#define TCR_SH0_INNER     (UL(3) << TCR_SH0_SHIFT)

#define TCR_SH1_SHIFT     28
#define TCR_SH1_MASK      (UL(3) << TCR_SH1_SHIFT)
#define TCR_SH1_INNER     (UL(3) << TCR_SH1_SHIFT)
#define TCR_SHARED        (TCR_SH0_INNER | TCR_SH1_INNER)

#define TCR_TG0_SHIFT     14
#define TCR_TG0_MASK      (UL(3) << TCR_TG0_SHIFT)
#define TCR_TG0_4K        (UL(0) << TCR_TG0_SHIFT)
#define TCR_TG0_64K       (UL(1) << TCR_TG0_SHIFT)
#define TCR_TG0_16K       (UL(2) << TCR_TG0_SHIFT)

#define TCR_TG1_SHIFT     30
#define TCR_TG1_MASK      (UL(3) << TCR_TG1_SHIFT)
#define TCR_TG1_16K       (UL(1) << TCR_TG1_SHIFT)
#define TCR_TG1_4K        (UL(2) << TCR_TG1_SHIFT)
#define TCR_TG1_64K       (UL(3) << TCR_TG1_SHIFT)

#define TCR_IPS_SHIFT     32
#define TCR_IPS_48        (UL(5) << TCR_IPS_SHIFT)

#define TCR_IPS_MASK      (UL(7) << TCR_IPS_SHIFT)
#define TCR_A1            (UL(1) << 22)
#define TCR_ASID16        (UL(1) << 36)
#define TCR_TBI0          (UL(1) << 37)
#define TCR_HA            (UL(1) << 39)
#define TCR_HD            (UL(1) << 40)
#define TCR_NFD1          (UL(1) << 54)
