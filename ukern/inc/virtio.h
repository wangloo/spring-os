#pragma once
#include <types.h>

#define VIRTIO_MAGIC   0x74726976
#define VIRTIO_VERSION 0x2


// spec part 5.Device Types
#define VIRTIO_DEV_INVAILD 0x0
#define VIRTIO_DEV_NET     0x1
#define VIRTIO_DEV_BLK     0x2


#define VIRTIO_STATUS_ACKNOWLEDGE        (1)
#define VIRTIO_STATUS_DRIVER             (2)
#define VIRTIO_STATUS_FAILED             (128)
#define VIRTIO_STATUS_FEATURES_OK        (8)
#define VIRTIO_STATUS_DRIVER_OK          (4)
#define VIRTIO_STATUS_DEVICE_NEEDS_RESET (64)

#define HI32(val64) ((uint32_t)((0xFFFFFFFF00000000ULL & (val64)) >> 32))
#define LO32(val64) ((uint32_t)(0x00000000FFFFFFFFULL & (val64)))

typedef volatile struct __attribute__((packed)){
	uint32_t MagicValue;
	uint32_t Version;
	uint32_t DeviceID;
	uint32_t VendorID;
	uint32_t DeviceFeatures;
	uint32_t DeviceFeaturesSel;
	uint32_t _reserved0[2];
	uint32_t DriverFeatures;
	uint32_t DriverFeaturesSel;
	uint32_t _reserved1[2];
	uint32_t QueueSel;
	uint32_t QueueNumMax;
	uint32_t QueueNum;
	uint32_t _reserved2[2];
	uint32_t QueueReady;
	uint32_t _reserved3[2];
	uint32_t QueueNotify;
	uint32_t _reserved4[3];
	uint32_t InterruptStatus;
	uint32_t InterruptACK;
	uint32_t _reserved5[2];
	uint32_t Status;
	uint32_t _reserved6[3];
	uint32_t QueueDescLow;  // paddr of description table of virtqueue(lower 32)
	uint32_t QueueDescHigh; // paddr of description table of virtqueue(higher 32)
	uint32_t _reserved7[2];
	uint32_t QueueAvailLow;
	uint32_t QueueAvailHigh;
	uint32_t _reserved8[2];
	uint32_t QueueUsedLow;
	uint32_t QueueUsedHigh;
	uint32_t _reserved9[21];
	uint32_t ConfigGeneration;
	uint32_t Config[0];
} virtio_regs ;

/* feature record */
struct virtio_feat {
    char *name;
    uint32_t bit;
    bool support;
    char *help;
};

struct virtqueue_desc {
    uint64_t addr;   // physical addr of buf
    uint32_t len;    // size of buf(bytes)
    /* This marks a buffer as continuing via the next field. */
#define VIRTQ_DESC_F_NEXT 1
    /* This marks a buffer as device write-only (otherwise device read-only). */
#define VIRTQ_DESC_F_WRITE 2
    /* This means the buffer contains a list of buffer descriptors. */
#define VIRTQ_DESC_F_INDIRECT 4
    /* The flags as indicated above. */
    uint16_t flags;
    /* Next field if flags & NEXT */
    uint16_t next;
} __attribute__((packed));

struct virtqueue_avail {
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
    uint16_t flags;
    uint16_t idx;     // % virtq->len = free index of ring[]
    uint16_t ring[0];
} __attribute__((packed));

struct virtqueue_used_elem {
    uint32_t id;
    uint32_t len;
} __attribute__((packed));

struct virtqueue_used {
#define VIRTQ_USED_F_NO_NOTIFY 1
    uint16_t flags;
    uint16_t idx;
    struct virtqueue_used_elem ring[0];
} __attribute__((packed));

struct virtqueue {
    uint64_t phys;      // paddr of the full data structure
    uint32_t len;       // number of buffers
    uint32_t seen_used;
    uint32_t free_desc; // index of free description table

    volatile struct virtqueue_desc *desc;
    volatile struct virtqueue_avail *avail;
    volatile uint16_t *used_event;
    volatile struct virtqueue_used *used;
    volatile uint16_t *avail_event;
    void **desc_virt;
} __attribute__((packed));




/* extension feature bits for queue and negotiation */
#define VIRTIO_EXT_FEATS                                                  \
    { "VIRTIO_F_RING_INDIRECT_DESC", 28, false,                            \
        "Negotiating this feature indicates that the driver can use"         \
        " descriptors with the VIRTQ_DESC_F_INDIRECT flag set, as"           \
        " described in 2.4.5.3 Indirect Descriptors." },                     \
    { "VIRTIO_F_RING_EVENT_IDX", 29, false,                        \
        "This feature enables the used_event and the avail_event "   \
        "fields"                                                     \
        " as described in 2.4.7 and 2.4.8." },                       \
    { "VIRTIO_F_VERSION_1", 32, false,                             \
        "This indicates compliance with this specification, giving " \
        "a simple way to detect legacy devices or drivers." },

void virtio_init(void);
void virtio_feat_nego(virtio_regs *regs, struct virtio_feat *feats, uint32_t n);



struct virtqueue *virtq_alloc(u32 len);
void virtq_add_to_device(volatile virtio_regs *regs, struct virtqueue *virtq,
                         uint32_t queue_sel);
uint32_t virtq_alloc_desc(struct virtqueue *virtq, void *addr);
void virtq_free_desc(struct virtqueue *virtq, uint32_t desc);