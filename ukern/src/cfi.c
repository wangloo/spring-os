#include <cfi.h>
#include <config/config.h>
#include <string.h>
#include <assert.h>

#define CFI_BASE  ((vaddr_t)CONFIG_PFLASH_BASE)

/////////////////////////////////////////
// depending on size of a word
/////////////////////////////////////////
#define WSIZE   4
#define CFI_GETB(offset)      (*(unsigned char *)(CFI_BASE+offset))
#define CFI_PUTB(offset, val) (*(unsigned char *)(CFI_BASE+offset) = (val))
#define CFI_GETW(offset)      (*(unsigned int *) (CFI_BASE+offset))
#define CFI_PUTW(offset, val) (*(unsigned int *) (CFI_BASE+offset) = (val))
#define W2B(word)  ((word) << 2)
#define B2W(byte)  ((byte) >> 2)

/////////////////////////////////////////
// QEMU pflash expected value
/////////////////////////////////////////
#define QEMU_EXPECTED_ERASE_REGION_NUM  (1)
#define QEMU_EXPECTED_ERASE_BLOCK_NUM   (256)
#define QEMU_EXPECTED_ERASE_BLOCK_SZ    (512 * 256) // 128K
#define QEMU_EXPECTED_PAGE_SZ           (1 << 11)   // 2k




/////////////////////////////////////////
// 
/////////////////////////////////////////
#define CFI_ERASE_BLOCK_NUM       QEMU_EXPECTED_ERASE_BLOCK_NUM
#define CFI_ERASE_BLOCK_SZ        QEMU_EXPECTED_ERASE_BLOCK_SZ
#define CFI_ERASE_BLOCK_SZ_WORD  (CFI_ERASE_BLOCK_SZ / WSIZE)
#define CFI_PAGE_SZ               QEMU_EXPECTED_PAGE_SZ
#define CFI_PAGE_SZ_WORD         (CFI_PAGE_SZ / WSIZE)
static u32 capacity;
static u32 eregnum;
static u32 eblksz;
static u32 pgsz;

// todo: abstract
struct cfi_flash_t {
    u32 cf_blksz;
};

// offset: in word
static inline int cfi_is_ready(int offset)
{
    CFI_PUTW(offset, 0x70);
    return CFI_GETW(offset) & 0x80; // 0x80 is MASK
}

static int cfi_query_QRY(void)
{
    if (CFI_GETB(W2B(0x10)) != 'Q') 
        return -1;
    if (CFI_GETB(W2B(0x11)) != 'R') 
        return -1;
    if (CFI_GETB(W2B(0x12)) != 'Y') 
        return -1;

    return 0;
}

static inline u8 cfi_query_1b(int offset)
{
    return CFI_GETB(W2B(offset));
}

static inline u16 cfi_query_2b(int offset)
{
    return (CFI_GETB(W2B(offset + 1)) << 8) | (CFI_GETB(W2B(offset)));
}

int cfi_query(void)
{
    int ret;

    CFI_PUTW(0x55, 0x98);

    // verify CFI-capable
    if ((ret = cfi_query_QRY()) < 0) 
        return ret;
    
    // query device size
    capacity = 1u << cfi_query_1b(0x27);
    printf("device size: 0x%x\n", capacity);
        
    // query page bit(max number of bytes in buffer write)
    pgsz = 1u << cfi_query_2b(0x2A);
    assert(pgsz == QEMU_EXPECTED_PAGE_SZ);

    // query number of earse region
    eregnum = cfi_query_1b(0x2c);
    assert(eregnum == QEMU_EXPECTED_ERASE_REGION_NUM);

    // query number of Erase Blocks of identical size within region
    int eblknum;
    eblknum = cfi_query_2b(0x2d) + 1;
    assert(eblknum == QEMU_EXPECTED_ERASE_BLOCK_NUM);

    // query earse block size withon region
    eblksz = cfi_query_2b(0x2f);
    eblksz *= 256;
    assert(eblksz == QEMU_EXPECTED_ERASE_BLOCK_SZ);
    printf("erase block size: 0x%x\n", eblksz);

    CFI_PUTW(0, 0xff);
    return 0;
}

int cfi_init(void)
{
    if (cfi_query() < 0)
        return -1;
    return 0;
}
   


// addr: in bytes
int cfi_erase(u64 addr, int bytes) 
{
/* 擦除操作可能存在bug? 每次擦除的单位变成了8个block
 * 故在此将其注释掉
 */
#if 0
    u64 blkaddr, cnt, i;
    
    blkaddr = B2W((addr) & (~(CFI_ERASE_BLOCK_SZ - 1)));
    cnt = (B2W(addr + bytes - 1) - blkaddr) / CFI_ERASE_BLOCK_SZ_WORD + 1;

    printf("erase block addr: 0x%lx, count: %d\n", W2B(blkaddr), cnt);
    for (i = 0; i < cnt; i++) {
        CFI_PUTW(blkaddr, 0x20); // erase 
        CFI_PUTW(blkaddr, 0xd0); // confirm
        while (!cfi_is_ready(blkaddr)) {}
        blkaddr += CFI_ERASE_BLOCK_SZ_WORD;
    }
    CFI_PUTW(0, 0x50); // clear status
#endif
    return 0;
}

// offset: in byte
int cfi_readb(char *buf, u64 offset, int bytes)
{
    int i;
    for (i = 0; i < bytes; i++) {
        buf[i] = CFI_GETB(offset + i);
    }
    return 0;
}

// offset:  in word
int cfi_readw(char *buf, u64 offset, int words)
{
    u32 *p = (u32 *)buf;
    int i;

    for (i = 0; i < words; i++) {
        p[i] = CFI_GETW(offset + i);
    }
    return 0;
}

// write operation can't over PAGE
// offset:  in word
int cfi_writew(const char *buf, u64 offset, int words)
{
    const u32 *p = (const u32 *)buf;
    u64 blkaddr;
    int cnt;  // number of words for each write
    int i;

    // first wirte might not be page aligned
    cnt = CFI_PAGE_SZ_WORD - (offset & (CFI_PAGE_SZ_WORD - 1));
    cnt = (cnt > words)? words : cnt;

    while (words) {
        blkaddr = offset & (~(CFI_ERASE_BLOCK_SZ_WORD - 1));
        CFI_PUTW(blkaddr, 0xe8); // write buffer
        CFI_PUTW(blkaddr, cnt - 1); // write word count, 0-based

        // write one word at a time
        for (i = 0; i < cnt; i++) {
            CFI_PUTW(offset++, *p++);
        }
        // confirm
        CFI_PUTW(blkaddr, 0xd0);
        while (!cfi_is_ready(blkaddr)) {}

        words -= cnt;
        cnt = (words > CFI_PAGE_SZ_WORD)? CFI_PAGE_SZ_WORD : words;
    }
    
    CFI_PUTW(0, 0x50);  // clear status
    return 0;
}

// offset: in bytes
static void cfi_show_fmt(const char *buf, u64 offset, int words)
{
    int i, j;

    for (i = 0; i < words; i++) {
        printf("0x%x:", offset + i*WSIZE);
        for (j = 0; j < WSIZE; j++) {
            printf("%x ", buf[i*WSIZE + j]);
        }
        printf("\n");
    }
    printf("\n");
}

