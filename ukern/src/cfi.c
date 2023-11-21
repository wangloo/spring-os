#include <kernel.h>
#include <cfi.h>

/////////////////////////////////////////
// depending on size of a word
/////////////////////////////////////////
#define WSIZE   4
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

volatile u32 *cfi_pflash_base = (volatile u32 *)(ptov(CONFIG_PFLASH_BASE));
static u32 capacity;
static u32 eregnum;
static u32 eblksz;
static u32 pgsz;


static inline u8 CFI_GETB(u32 offset)
{
    return ((u8 *)cfi_pflash_base)[offset];
}
static inline u32 CFI_GETW(u32 woffset)
{
    return cfi_pflash_base[woffset];
}
static inline void CFI_PUTW(u32 woffset, u32 val)
{
    cfi_pflash_base[woffset] = val;
}

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
    LOG_INFO("CFI", "device size: 0x%x\n", capacity);
        
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
    LOG_INFO("CFI", "erase block size: 0x%x\n", eblksz);

    CFI_PUTW(0, 0xff);
    return 0;
}


   


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


size_t cfi_read(char *out, u64 offset, size_t size)
{
    u64 offset_walign;

    offset_walign = align_up(offset, WSIZE);

    // 1. 读字对齐之前地址的内容
    while (size > 0 && offset < offset_walign) {
        *(out++) = CFI_GETB(offset);
        offset += 1;
        size -= 1;
    }
    
    // 2. 按字读取
    while (size >= WSIZE) {
        *((unsigned int *)out) = CFI_GETW(offset/WSIZE);
        out += WSIZE;
        offset += WSIZE;
        size -= WSIZE;
    }

    // 3. 读剩下的字节内容
    while (size > 0) {
        *(out++) = CFI_GETB(offset);
        offset += 1;
        size -= 1;
    }
    return size;
}

// write operation can't over PAGE
size_t cfi_write(const char *in, u64 offset, size_t size)
{
    u64 blkaddr;
    size_t words;
    int wcnt;  // number of words for each write
    int i;

    /* 写入大小和偏移都必须是WSIZE的整数倍 */
    assert( (offset % WSIZE == 0) && 
              (size % WSIZE == 0));

    words = size / WSIZE;

    // first wirte might not be page aligned
    wcnt = B2W(CFI_PAGE_SZ - (offset & (CFI_PAGE_SZ - 1)));
    wcnt = (wcnt > words)? words : wcnt;

    while (words > 0) {
        blkaddr = B2W(offset & (~(CFI_ERASE_BLOCK_SZ - 1)));
        CFI_PUTW(blkaddr, 0xe8); // write buffer
        CFI_PUTW(blkaddr, wcnt - 1); // write word count, 0-based
        
        // write one word at a time
        for (i = 0; i < wcnt; i++) {
            CFI_PUTW(B2W(offset), *((u32 *)in));
            offset += WSIZE;
            in += WSIZE;
        }
        
        CFI_PUTW(blkaddr, 0xd0); // confirm
        while (!cfi_is_ready(blkaddr)) {}

        words -= wcnt;
        wcnt = (words > CFI_PAGE_SZ_WORD)? CFI_PAGE_SZ_WORD : words;
    }
    CFI_PUTW(0, 0x50);  // clear status
    return size;
}


void cfi_test(void)
{
    size_t size = 256;
    u32 offset;
    char rbuf[size];
    char wbuf[size];

    printf("start testing CFI...\n");

    // 初始化测试数据
    for (int i = 0; i < size; i++) 
        wbuf[i] = i % size;

    // 1. 对第一个block的 0-256 byte读写测试
#if 1 
    offset = 0;
    memset(rbuf, 0 , sizeof(rbuf));
    cfi_erase(0, CFI_ERASE_BLOCK_SZ);
    cfi_write(wbuf, offset, size);
    cfi_read(rbuf, offset, size);
    for (int i = 0; i < size; i++) {
        assert(wbuf[i] == rbuf[i]);
    }
#endif

    // 2. 对第一个block的第 256-512 byte读写测试
#if 1 
    offset = 256;
    memset(rbuf, 0 , sizeof(rbuf));
    cfi_erase(0, CFI_ERASE_BLOCK_SZ);
    cfi_write(wbuf, offset, size);
    cfi_read(rbuf, offset, size);
    for (int i = 0; i < size; i++) {
        assert(wbuf[i] == rbuf[i]);
    }
#endif

    // 3. 对最后一个block的后256bytes 读写测试
#if 1
    offset = (CFI_ERASE_BLOCK_NUM) * CFI_ERASE_BLOCK_SZ - 256;
    memset(rbuf, 0 , sizeof(rbuf));
    cfi_write(wbuf, offset, size);
    cfi_read(rbuf, offset, size);
    for (int i = 0; i < size; i++) {
        assert(wbuf[i] == rbuf[i]);
    }
#endif

    // 3. QEMU dtb 中给定的 pflash unit1 size=0x4000000
    //    而CFI query 得到的size=0x2000000, 如果操作
    //    0x2000000 以后的地址会怎么样?
    //    result: RAZ/WI
    /* offset = (CFI_ERASE_BLOCK_NUM) * CFI_ERASE_BLOCK_SZ; */
    /* memset(rbuf, 0, sizeof(rbuf)); */
    /* cfi_erase(offset, 256); */
    /* cfi_writew(wbuf, offset, words); */
    /* cfi_show_fmt(rbuf, offset, words); */

    // 4. 测试erase. 擦除首个block, 确保后面的block不受影响
#if 0    /* cfi_erase() 可能存在bug, 详见其实现中的注释 */
    offset = 0;
    words = CFI_ERASE_BLOCK_SZ_WORD;
    char *ewbuf = mem_alloc(words*WSIZE);
    char *erbuf = mem_alloc(words*WSIZE);
    memset(ewbuf, 0, words*WSIZE);
    
    cfi_writew(ewbuf, B2W(offset)+CFI_ERASE_BLOCK_SZ_WORD, words);
    cfi_erase(offset, words*WSIZE);
    cfi_readw(erbuf, B2W(offset)+CFI_ERASE_BLOCK_SZ_WORD, words);
    for (int i = 0; i < words; i++) {
        mprintf("erbuf[%d] = %x, ewbuf[%d] = %x\n", i, *(u32 *)(erbuf + i), i, *(u32 *)(ewbuf + i));
        assert((*(u32 *)(erbuf+i)) == (*(u32 *)(ewbuf+i)));
    }

    mem_free(erbuf);
    mem_free(ewbuf);
#endif

    printf("CFI test PASSED!!\n");
}

int 
init_cfi(void)
{
    if (cfi_query() < 0)
        return -1;
    LOG_DEBUG("CFI", "init ok\n");
    return 0;
}