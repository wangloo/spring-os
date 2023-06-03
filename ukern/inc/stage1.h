#pragma once
#include <utils.h>


/*
 * stage 1 VMSAv8-64 Table Descriptors
 */
#define S1_DES_FAULT     (0b00 << 0)
#define S1_DES_BLOCK     (0b01 << 0) /* level 1/2 */
#define S1_DES_TABLE     (0b11 << 0) /* level 0/1/2 */
#define S1_DES_PAGE      (0b11 << 0) /* level 3 */

#define S1_TABLE_NS      (UL(1) << 63)
#define S1_TABLE_AP      (0)
#define S1_TABLE_XN      (UL(1) << 60)

#define S1_TABLE_UAP     (UL(1) << 61)
#define S1_TABLE_UXN     (UL(1) << 60)

#define S1_CONTIGUOUS    (UL(1) << 52)
#define S1_PXN           (UL(1) << 53)
#define S1_XN            (UL(1) << 54)

#define S1_PFNMAP        (UL(1) << 55) // 55 - 58 is for software
#define S1_DEVMAP        (UL(1) << 56) // 55 - 58 is for software
#define S1_SHARED        (UL(1) << 57) // 55 - 58 is for software

#define S1_NS            (1 << 5)

#define S1_AP_RW         (0b00 << 6) // for EL2 ap[2] is valid
#define S1_AP_RW_URW     (0b01 << 6)
#define S1_AP_RO         (0b10 << 6)
#define S1_AP_RO_URO     (0b11 << 6)

#define S1_SH_NON        (0b00 << 8)
#define S1_SH_OUTER      (0b10 << 8)
#define S1_SH_INNER      (0b11 << 8)

#define S1_AF            (1 << 10)
#define S1_nG            (1 << 11)

#define S1_ATTR_IDX(n)   ((n & 0xf) << 2)

#define MT_DEVICE_nGnRnE 0
#define MT_DEVICE_nGnRE  1
#define MT_DEVICE_GRE    2
#define MT_NORMAL_NC     3
#define MT_NORMAL        4
#define MT_NORMAL_WT     5

#define S1_PAGE_NORMAL       (S1_DES_PAGE | S1_AF | S1_NS | S1_SH_INNER | S1_ATTR_IDX(MT_NORMAL))
#define S1_PAGE_NC           (S1_DES_PAGE | S1_AF | S1_NS | S1_SH_INNER | S1_ATTR_IDX(MT_DEVICE_nGnRE))
#define S1_PAGE_DEVICE       (S1_DES_PAGE | S1_AF | S1_NS | S1_SH_INNER | S1_ATTR_IDX(MT_DEVICE_nGnRnE))
#define S1_PAGE_NORMAL_NC    (S1_DES_PAGE | S1_AF | S1_NS | S1_SH_INNER | S1_ATTR_IDX(MT_NORMAL_NC))
#define S1_PAGE_WT           (S1_DES_PAGE | S1_AF | S1_NS | S1_SH_INNER | S1_ATTR_IDX(MT_NORMAL_WT))

#define S1_BLOCK_NORMAL      (S1_DES_BLOCK | S1_AF | S1_NS | S1_SH_INNER | S1_ATTR_IDX(MT_NORMAL))
#define S1_BLOCK_NC          (S1_DES_BLOCK | S1_AF | S1_NS | S1_SH_INNER | S1_ATTR_IDX(MT_DEVICE_nGnRE))
#define S1_BLOCK_DEVICE      (S1_DES_BLOCK | S1_AF | S1_NS | S1_SH_INNER | S1_ATTR_IDX(MT_DEVICE_nGnRnE))
#define S1_BLOCK_NORMAL_NC   (S1_DES_BLOCK | S1_AF | S1_NS | S1_SH_INNER | S1_ATTR_IDX(MT_NORMAL_NC))
#define S1_BLOCK_WT          (S1_DES_BLOCK | S1_AF | S1_NS | S1_SH_INNER | S1_ATTR_IDX(MT_NORMAL_WT))

#define BOOTMEM_CODE_ATTR     (S1_ATTR_IDX(MT_NORMAL) | S1_DES_PAGE | S1_NS | S1_AP_RO | S1_SH_INNER | S1_AF | S1_XN)
#define BOOTMEM_DATA_ATTR     (S1_ATTR_IDX(MT_NORMAL) | S1_DES_PAGE | S1_NS | S1_AP_RW | S1_SH_INNER | S1_AF | S1_XN | S1_PXN)
#define BOOTMEM_DATA_RO_ATTR  (S1_ATTR_IDX(MT_NORMAL) | S1_DES_PAGE | S1_NS | S1_AP_RO | S1_SH_INNER | S1_AF | S1_XN | S1_PXN)
#define BOOTMEM_INIT_ATTR     (S1_ATTR_IDX(MT_NORMAL) | S1_DES_PAGE | S1_NS | S1_AP_RW | S1_SH_INNER | S1_AF | S1_XN)
#define BOOTMEM_IO_ATTR       (S1_ATTR_IDX(MT_DEVICE_nGnRnE) | S1_DES_PAGE | S1_NS | S1_AP_RW | S1_SH_INNER | S1_AF | S1_XN | S1_PXN)
#define BOOTMEM_IO_BLK_ATTR   (S1_ATTR_IDX(MT_DEVICE_nGnRnE) | S1_DES_BLOCK | S1_NS | S1_AP_RW | S1_SH_INNER | S1_AF | S1_XN | S1_PXN)
#define BOOTMEM_DATA_BLK_ATTR (S1_ATTR_IDX(MT_NORMAL) | S1_DES_BLOCK | S1_NS | S1_AP_RW | S1_SH_INNER | S1_AF | S1_XN | S1_PXN)