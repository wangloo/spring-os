#pragma once
#include <virtio.h>
#include <block.h>


#define VIRTIO_BLK_SECTOR_SIZE     512

#define VIRTIO_BLK_REQ_HEADER_SIZE 16
#define VIRTIO_BLK_REQ_FOOTER_SIZE 1

#define VIRTIO_BLK_S_OK            0
#define VIRTIO_BLK_S_IOERR         1 // device or driver error
#define VIRTIO_BLK_S_UNSUPP        2 // request unsupported by device

/* description of a virtio block device */
struct virtio_blk {
    virtio_regs *regs;
    volatile struct virtio_blk_config *config;
    struct virtqueue *virtq;
    uint32_t intid;
    struct list_head list;
    struct blkdev blkdev;
};


/* The capacity of the device (expressed in 512-byte sectors) 
is always present. The availability of the others
all depend on various feature negotiation. */
struct virtio_blk_config {
    uint64_t capacity;
    uint32_t size_max;
    uint32_t seg_max;
    struct {
        uint16_t cylinders;
        uint8_t heads;
        uint8_t sectors;
    } geometry;
    uint32_t blk_size; // sector size
    struct {
        uint8_t physical_block_exp;
        uint8_t alignment_offset;
        uint16_t min_io_size;
        uint32_t opt_io_size;
    } topology;
    uint8_t writeback;
} __attribute__((packed));


int virtio_blk_init(virtio_regs *regs, uint32_t intid);
void virtio_blk_submit(struct virtio_blk *blk, uint32_t type, uint32_t sector, uint8_t *data);
