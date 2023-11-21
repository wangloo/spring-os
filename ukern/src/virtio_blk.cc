#include <virtio_blk.h>
#include <barrier.h>
#include <kernel.h>
#include <kmem.h>
#include <page.h>

struct virtio_blk_req {
#define VIRTIO_BLK_T_IN       0
#define VIRTIO_BLK_T_OUT      1
	uint32_t type;
	uint32_t reserved;
	uint64_t sector;
	/* NO DATA INCLUDED HERE! */
	uint8_t status;
} __attribute__((packed));

/* !arrange in increasing order of bits */
struct virtio_feat blk_feats[] = {
    { "VIRTIO_BLK_F_SIZE_MAX", 1, false,
        "Maximum size of any single segment is in size_max." },
    { "VIRTIO_BLK_F_SEG_MAX", 2, false,
        "Maximum number of segments in a request is in seg_max." },
    { "VIRTIO_BLK_F_GEOMETRY", 4, false,
        "Disk-style geometry specified in geometry." },
    { "VIRTIO_BLK_F_RO", 5, false, "Device is read-only." },
    { "VIRTIO_BLK_F_BLK_SIZE", 6, false,
        "Block size of disk is in blk_size." },
    { "VIRTIO_BLK_F_FLUSH", 9, false, "Cache flush command support." },
    { "VIRTIO_BLK_F_TOPOLOGY", 10, false,
        "Device exports information on optimal I/O alignment." },
    { "VIRTIO_BLK_F_CONFIG_WCE", 11, false,
        "Device can toggle its cache between writeback and "
            "writethrough modes." },
    VIRTIO_EXT_FEATS
};

// TODO: 封装块设备的操作函数
struct blkdev_ops virtio_blk_ops = {
    .alloc = NULL,
    .free = NULL,
    .submit = NULL,
    .status = NULL,
	// .alloc = virtio_blk_alloc,
	// .free = virtio_blk_free,
	// .submit = virtio_blk_submit,
	// .status = virtio_blk_status,
};


/**
 * @brief 执行virtio块设备的写入或者读取, 每次操作的单位都是一个sector(512B)
 * 
 * @param [in] blk 
 * @param [in] type    VIRTIO_BLK_T_IN  读取
 *                     VIRTIO_BLK_T_OUT 写入
 * @param [in] sector  要操作的扇区号
 * @param [in] data    写入情况下, 存放
 */
void virtio_blk_submit(struct virtio_blk *blk, uint32_t type, uint32_t sector, uint8_t *data)
{
    uint32_t d1, d2, d3, datamode = 0;
    struct virtio_blk_req *hdr = get_free_page(0); // FIXME: memory leak?

    hdr->type = type;
    hdr->sector = sector;

	d1 = virtq_alloc_desc(blk->virtq, hdr);
	blk->virtq->desc[d1].len = VIRTIO_BLK_REQ_HEADER_SIZE;
	blk->virtq->desc[d1].flags = VIRTQ_DESC_F_NEXT;
	
	if (type == VIRTIO_BLK_T_IN) /* if it's a read */
	    datamode = VIRTQ_DESC_F_WRITE; /* mark page writeable */

	d2 = virtq_alloc_desc(blk->virtq, data);
	blk->virtq->desc[d2].len = VIRTIO_BLK_SECTOR_SIZE;
	blk->virtq->desc[d2].flags = datamode | VIRTQ_DESC_F_NEXT;

	d3 = virtq_alloc_desc(blk->virtq, (void*)hdr + VIRTIO_BLK_REQ_HEADER_SIZE);
	blk->virtq->desc[d3].len = VIRTIO_BLK_REQ_FOOTER_SIZE;
	blk->virtq->desc[d3].flags = VIRTQ_DESC_F_WRITE;

	blk->virtq->desc[d1].next = d2;
	blk->virtq->desc[d2].next = d3;

	blk->virtq->avail->ring[blk->virtq->avail->idx] = d1;
	dmb();
	blk->virtq->avail->idx += 1;
	dmb();
	blk->regs->QueueNotify = 0;
}

static void virtio_blk_handle_used(struct virtio_blk *dev, uint32_t usedidx) {
	struct virtqueue *virtq = dev->virtq;
	uint32_t desc1, desc2, desc3;
	struct virtio_blk_req *req;
	uint8_t *data;

    desc1 = virtq->used->ring[usedidx].id;
    if (!(virtq->desc[desc1].flags & VIRTQ_DESC_F_NEXT))
        goto bad_desc;
    desc2 = virtq->desc[desc1].next;
    if (!(virtq->desc[desc2].flags & VIRTQ_DESC_F_NEXT))
        goto bad_desc;
    desc3 = virtq->desc[desc2].next;
	if (virtq->desc[desc1].len != VIRTIO_BLK_REQ_HEADER_SIZE
			|| virtq->desc[desc2].len != VIRTIO_BLK_SECTOR_SIZE
			|| virtq->desc[desc3].len != VIRTIO_BLK_REQ_FOOTER_SIZE)
		goto bad_desc;

	req = virtq->desc_virt[desc1];
	data = virtq->desc_virt[desc2];
	if (req->status != VIRTIO_BLK_S_OK)
		goto bad_status;

	if (req->type == VIRTIO_BLK_T_IN) {
        /* TODO: 目前只对读取做了判断，未来可以扩展到写入。而且读取的数据没有
         * 传出，可以在这里打印data检查结果。
         */
        data = data;
	}

	virtq_free_desc(virtq, desc1);
	virtq_free_desc(virtq, desc2);
	virtq_free_desc(virtq, desc3);

    return;
bad_desc:
    printf("virtio-blk received malformed descriptors\n");
    return;
bad_status:
    printf("virtio-blk: error in command response\n");
}

// TODO: dynamic allocate
struct virtio_blk vdev;
void virtio_blk_isr(uint32_t intid)
{
    struct virtio_blk *dev = &vdev;
    u32 len = dev->virtq->len;
    int i;

    dev->regs->InterruptACK = dev->regs->InterruptStatus;

	for (i = dev->virtq->seen_used; i != dev->virtq->used->idx; i = wrap(i + 1, len)) {
		virtio_blk_handle_used(dev, i);
	}
	dev->virtq->seen_used = dev->virtq->used->idx;
}

int virtio_blk_init(virtio_regs *regs, u32 irq)
{
    volatile struct virtio_blk_config *conf = (struct virtio_blk_config *)&regs->Config;
    struct virtqueue *virtq;
    uint32_t genbefore, genafter;

    virtio_feat_nego(regs, blk_feats, nelem(blk_feats));
    regs->Status |= VIRTIO_STATUS_FEATURES_OK;
    dmb();
    if (!(regs->Status & VIRTIO_STATUS_FEATURES_OK)) {
        panic("virtio-blk did not accept our features\n");
        return -1;
    }
    
    /* dump some configuration of blk device.
     * for now, all features are disable in driver, so the only meaningful
     * info in virtio_blk_config is capacity */
    printf("feature negotiation OK!\n");
    printf("- virtio-blk has 0x%x 0x%x sectors\n", HI32(conf->capacity), LO32(conf->capacity));
    printf("- virtio-blk queue number max: %d\n", regs->QueueNumMax);
    printf("- virtio-blk status: %d\n", regs->Status);
    printf("- virtio-blk interrupt status: %d\n", regs->InterruptStatus);
    
    virtq = virtq_alloc(64);
    virtq_add_to_device(regs, virtq, 0); // 0 for first queue

    vdev.regs = regs;
    vdev.intid = irq;
    vdev.virtq = virtq;
    vdev.config = conf;
	vdev.blkdev.ops = &virtio_blk_ops;
	vdev.blkdev.blksiz = VIRTIO_BLK_SECTOR_SIZE;

    /* TODO: config read can't guarantee atomic  */
	do {
		genbefore = vdev.regs->ConfigGeneration;
		vdev.blkdev.blkcnt = vdev.config->capacity;
		genafter = vdev.regs->ConfigGeneration;
	} while (genbefore != genafter);
	snprintf(vdev.blkdev.name, sizeof(vdev.blkdev.name), "vblk%d",
	         vdev.intid);
    
    
    /* TODO: 这里应该根据系统的实现来注册中断
     * handler是 virtio_blk_isr(), 中断号是参数irq，
     * 不同系统的实现可能不相同，这里暂时未实现
     */
    
    regs->Status |= VIRTIO_STATUS_DRIVER_OK;
    dmb();

    return 0;
}
