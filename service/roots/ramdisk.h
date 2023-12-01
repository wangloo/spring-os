#include <uapi/ramdisk.h>

int ramdisk_read(struct ramdisk_file *file, void *buf, size_t size, unsigned long offset);
int ramdisk_open(char *name, struct ramdisk_file *file);

int ramdisk_init(unsigned long base, unsigned long end);
