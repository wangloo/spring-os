#pragma once
#include <asm/arm64_common.h>
#include <asm/arm64_mair.h>

#define ARM64_SPSR_VALUE	0x1c5

#define ARM64_CPACR_VALUE   CPACR_EL1_FPEN

#define ARM64_SCTLR_VALUE	\
	SCTLR_EL1_UCI | SCTLR_EL1_UCT | SCTLR_EL1_DZE


/*
 * 0x00 - MT_DEVICE_NGNRNE
 * 0x04 - MT_DEVICE_NGNRE
 * 0x0c - MT_DEVICE_GRE
 * 0x44 - MT_NORMAL_NC
 * 0xff - MT_NORMAL
 * 0xbb - MT_NORMAL_WT
 */
#define ARM64_MAIR_VALUE           \
	MAIR(0x00, MT_DEVICE_nGnRnE) | \
	MAIR(0x04, MT_DEVICE_nGnRE)  | \
	MAIR(0x0c, MT_DEVICE_GRE)    | \
	MAIR(0x44, MT_NORMAL_NC)     | \
	MAIR(0xff, MT_NORMAL)        | \
	MAIR(0xbb, MT_NORMAL_WT)


// VA[55] == 0 : TCR_ELx.TBI0 determines whether address tags are used.
// VA[55] == 1 : TCR_ELx.TBI1 determines whether address tags are used.
#define ARM64_TCR_VALUE	\
	TCR_T0SZ(48) | TCR_T1SZ(48) | TCR_IRGN0_WBWA | TCR_IRGN1_WBWA |		\
	TCR_ORGN0_WBWA | TCR_ORGN1_WBWA | TCR_SH0_INNER | TCR_SH1_INNER |	\
	 TCR_TG0_4K | TCR_ASID16 | TCR_IPS_48

// CINOS:   0x1535103510
// Current: 0x1535103510