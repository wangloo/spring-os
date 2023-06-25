#pragma once
#include <types.h>
#include <list.h>

struct blkdev {
	struct list_head blklist;
	struct blkdev_ops *ops;
	uint64_t blkcnt;
	uint32_t blksiz;
	char name[16];
};


/**
 * Block device operations. Each operation MUST be implemented by a block device
 * driver.
 */
struct blkdev_ops {
	/**
	 * Create a blkreq structure, and initialize the waitlist and request
	 * list fields of it. Return the structure.
	 */
	struct blkreq *(*alloc)(struct blkdev *dev);
	/**
	 * Free a blkreq structure. THIS WILL NOT FREE THE BUFFER CONTAINED
	 * WITHIN IT.
	 */
	void (*free)(struct blkdev *dev, struct blkreq *req);
	/**
	 * Submit a block request to the device.
	 *
	 * When this function returns, the request is submitted, but not
	 * finished. The `wait` field of the req is an event which can be used
	 * to block until the request is satisfied. However, a caller may want
	 * to submit additional requests first, and then wait for all of them.
	 *
	 * Submitting the request to the device hands over ownership until the
	 * `wait` event is triggered. Callers MAY NOT modify the request after
	 * this function returns, until the wait event is triggered.
	 */
	void (*submit)(struct blkdev *dev, struct blkreq *req);
	/**
	 * Print out status information.
	 */
	void (*status)(struct blkdev *dev);
};


