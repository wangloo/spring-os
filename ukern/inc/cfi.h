#pragma once

#include <types.h>

#define CFI_QUERY_CMD              0x98
#define CFI_QUERY_QRY              0x10
#define CFI_QUERY_BASE             0x55
#define CFI_QUERY_VENDOR           0x13
#define CFI_QUERY_SIZE             0x27
#define CFI_QUERY_PAGE_BITS        0x2A
#define CFI_QUERY_ERASE_REGION     0x2C
#define CFI_QUERY_BLOCKS           0x2D
#define CFI_QUERY_BLOCK_SIZE       0x2F

void cfi_test(void);
int cfi_init(void);
int cfi_writew(const char *buf, u64 offset, int words);
int cfi_readw(char *buf, u64 offset, int words);
int cfi_readb(char *buf, u64 offset, int bytes);
