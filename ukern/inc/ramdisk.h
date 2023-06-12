#pragma once

#include <uapi/ramdisk.h>

void set_ramdisk_address(void *start, void *end);

int ramdisk_init(void);

int ramdisk_read(struct ramdisk_file *file, void *buf,
		size_t size, unsigned long offset);

int ramdisk_open(char *name, struct ramdisk_file *file);

unsigned long ramdisk_file_base(struct ramdisk_file *file);

unsigned long ramdisk_file_size(struct ramdisk_file *file);

const char *ramdisk_file_name(struct ramdisk_file *file);