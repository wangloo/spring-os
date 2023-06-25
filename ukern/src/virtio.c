#include <virtio_blk.h>
#include <virtio.h>
#include <barrier.h>
#include <utils.h>
#include <kmem.h>
#include <addrspace.h>

static u32 virtio_irq_base = CONFIG_VIRTIO_IRQ;
static volatile virtio_regs *virtio_regs_base = 
                                (volatile virtio_regs *)CONFIG_VIRTIO_BASE;
static int virtio_dev_init(volatile virtio_regs *regs, uint32_t irq)
{

    if ((regs->MagicValue) != VIRTIO_MAGIC) {
        printf("virtio at 0x%x had wrong magic value 0x%x, expected 0x%x\n",
                regs, regs->MagicValue, VIRTIO_MAGIC);
        return -1;
    }
    if ((regs->Version) != VIRTIO_VERSION) {
        printf("virtio at 0x%x had wrong version 0x%x, expected 0x%x\n",
                regs, regs->Version, VIRTIO_VERSION);
        return -1;
    }
    if ((regs->DeviceID) == 0) {
        // On QEMU, this is pretty common, don't print a message
        printf("virtio at 0x%x has DeviceID=0, skipping\n", regs);
        return -1;
    }
    /* First step of initialization: reset */
    regs->Status = 0;
    dmb();

    regs->Status |= VIRTIO_STATUS_ACKNOWLEDGE;
    regs->Status |= VIRTIO_STATUS_DRIVER;
    dmb();

    switch (regs->DeviceID) {
    case VIRTIO_DEV_BLK:
        printf("find a virtio-blk device, init...\n");
        return virtio_blk_init(regs, irq);
    default:
        /* 
         * 这并不是一个错误，只是现在只涉及到virtio块设备，其他的
         * virtio设备没有做支持，所以是一个TODO
         */
        panic("unsupported virtio device id 0x%x\n", regs->DeviceID);
    }


    return 0;
}


/**
 * @brief 协商一个virtio设备支持的特性
 * 
 * @param [in] regs  virtio设备寄存器
 * @param [in] feats 期望这个设备满足的特性集合
 * @param [in] n     feats数组长度
 */
void virtio_feat_nego(virtio_regs *regs, struct virtio_feat *feats, uint32_t n)
{
    uint32_t bank = 0;
    uint32_t driver = 0;
    uint32_t device;
    uint32_t i;

    /* negotiate feature in bank0 first */
    regs->DeviceFeaturesSel = bank;
    dmb();
    device = regs->DeviceFeatures;

    for (i = 0; i < n; i++) {
        // negotiate next bank of features
        if ((feats[i].bit / 32) != bank) {
            regs->DriverFeaturesSel = bank;
            dmb();
            regs->DriverFeatures = driver;
            if (device) {
                printf("device supports unknown bits 0x%x in bank %d\n", device, bank);
            }
            bank = feats[i].bit / 32;
            regs->DriverFeaturesSel = bank;
            dmb();
            device = regs->DriverFeatures;
        }
        if (device & feats[i].bit) {
            if (feats[i].support == true) {
                driver |= feats[i].bit;
            } else {
                printf("driver don't support feature: %s\n", 
                        feats[i].name);
            }
            // clear negotiated bit
            device &= ~(feats[i].bit);
        }
    }

    // write negotiated driver feature
    regs->DriverFeaturesSel = bank;
    dmb();
    regs->DriverFeatures = driver;
    if (device) {
        printf("device supports unknown bits 0x%x in bank %d\n", device, bank);
    }
}

/**
 * @brief 为一个virtio设备添加一个queue
 * 
 * @param [in] regs 
 * @param [in] virtq 
 * @param [in] queue_sel 
 */
void virtq_add_to_device(volatile virtio_regs *regs, struct virtqueue *virtq,
                         uint32_t queue_sel)
{
    paddr_t desc_addr = virtq->phys + ((void *)virtq->desc - (void *)virtq);
    paddr_t avail_addr = virtq->phys + ((void *)virtq->avail - (void *)virtq);
    paddr_t used_addr = virtq->phys + ((void *)virtq->used - (void *)virtq);

	regs->QueueSel = queue_sel;
	dmb();
	regs->QueueNum = virtq->len;
	regs->QueueDescLow = LO32(desc_addr);
	regs->QueueDescHigh = HI32(desc_addr);
	regs->QueueAvailLow = LO32(avail_addr);
	regs->QueueAvailHigh = HI32(avail_addr);
	regs->QueueUsedLow = LO32(used_addr);
	regs->QueueUsedHigh = HI32(used_addr);
	dmb();
	regs->QueueReady = 1;
}



/**
 * @brief 申请一个virtqueue
 * 
 * @param [in] len the length of virtqueue
 * @return     struct virtqueue* 
 */
struct virtqueue *virtq_alloc(u32 len)
{
    struct virtqueue *virtq;
    void  *page;
    int i;


	/* compute offsets */
	u32 off_desc = align_up(sizeof(struct virtqueue), 16);
	u32 off_avail = align_up(off_desc + len * sizeof(struct virtqueue_desc), 2);
	u32 off_used_event = (off_avail + sizeof(struct virtqueue_avail) +
	                           len * sizeof(uint16_t));
	u32 off_used = align_up(off_used_event + sizeof(uint16_t), 4);
	u32 off_avail_event = (off_used + sizeof(struct virtqueue_used) +
	                            len * sizeof(struct virtqueue_used_elem));
	u32 off_desc_virt = align_up(off_avail_event + sizeof(uint16_t), sizeof(void *));
	u32 memsize = off_desc_virt + len * sizeof(void *);

	if (memsize > PAGE_SIZE) {
		printf("virtq_create: virtqueue matedata is too big for a page\n", 
                memsize);
		return NULL;
	}

    // allocate a page to store metadata and virtqueue
    page = get_free_page(0);

    // fill items in virtqueue
    virtq = (struct virtqueue *)page;
    virtq->phys = vtop(page);
    virtq->len = len;

	virtq->desc = (struct virtqueue_desc *)(page + off_desc);
	virtq->avail = (struct virtqueue_avail *)(page + off_avail);
	virtq->used_event = (uint16_t *)(page + off_used_event);
	virtq->used = (struct virtqueue_used *)(page + off_used);
	virtq->avail_event = (uint16_t *)(page + off_avail_event);
	virtq->desc_virt = (void **)(page + off_desc_virt);

	virtq->avail->idx = 0;
	virtq->used->idx = 0;
	virtq->seen_used = virtq->used->idx;
	virtq->free_desc = 0;

	for (i = 0; i < len; i++) {
		virtq->desc[i].next = i + 1;
	}
    return virtq;
}

uint32_t virtq_alloc_desc(struct virtqueue *virtq, void *addr)
{
	uint32_t desc = virtq->free_desc;
	uint32_t next = virtq->desc[desc].next;
	if (desc == virtq->len)
		panic("ERROR: ran out of virtqueue descriptors\n");
	virtq->free_desc = next;

	virtq->desc[desc].addr = vtop(addr);
	virtq->desc_virt[desc] = addr;
	return desc;
}

void virtq_free_desc(struct virtqueue *virtq, uint32_t desc)
{
	virtq->desc[desc].next = virtq->free_desc;
	virtq->free_desc = desc;
	virtq->desc_virt[desc] = NULL;
}



/**
 * @brief 初始化所有的virtio设备
 */
void virtio_init()
{
    volatile virtio_regs *regs = virtio_regs_base;

    /* query all virtio devices and initialize existing  */
	for (int i = 0; i < 32; i++) {
		virtio_dev_init(regs, virtio_irq_base + i);

        /* 为每个设备保留寄存器大小为 0x200 */
        regs = (volatile virtio_regs *)((vaddr_t)regs + 0x200);
    }
}
