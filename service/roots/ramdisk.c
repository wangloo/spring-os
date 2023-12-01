#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <minos/types.h>
#include <minos/utils.h>
#include <minos/page.h>
#include <ramdisk.h>

static struct ramdisk_inode *root;
static struct ramdisk_sb *sb;
static void *ramdisk_data;



// Read data from file
// Return bytes actually read
int 
ramdisk_read(struct ramdisk_file *file, void *buf, size_t size, unsigned long offset)
{
    if (offset > file->inode->f_size)
        return 0;
    if ((offset + size) > file->inode->f_size)
        return 0;
    memcpy(buf, ramdisk_data + file->inode->f_offset + offset, size);
    return size;
}

// Give file name, return the file descriptor
// Return NULL if not found
static struct ramdisk_inode *
__ramdisk_open(const char *name)
{
    struct ramdisk_inode *inode;
    for (inode = root; inode < root + sb->file_cnt; inode++) {
        if (strncmp(inode->f_name, name, RAMDISK_FNAME_SIZE - 1) == 0)
            return inode;
    }
    return NULL;
}



// Open a file, fill the file descriptor
int 
ramdisk_open(char *name, struct ramdisk_file *fd)
{
    struct ramdisk_inode *inode;

    if (!sb) {
        printf("super block not found, forget to init ramdisk?\n");
        return -ENOENT;
    }

    inode = __ramdisk_open(name);
    if (!inode) {
        printf("open %s: file not found!\n", name);
        return -ENOENT;
    }

    memset(fd, 0, sizeof(struct ramdisk_file));
    fd->inode = inode;
    return 0;
}


// List all files in ramdisk
// No dir-file now, so depth of tree is always 1
void
ramdisk_tree(void)
{
  struct ramdisk_inode *inode;
  int nr_file = sb->file_cnt;
  printf("File            Size    \n");
  for (inode = root; inode < root+nr_file; inode++) {
    printf("%-16s%-10ld\n", inode->f_name, inode->f_size);
  }
}

// Init kernel component -- RAMDISK
// Return < 0 if error
int 
ramdisk_init(unsigned long base, unsigned long end)
{
    if (!page_aligned(base)) {
        printf("ramdisk start address need PAGE align\n");
        return -EINVAL;
    }
    if (strncmp((void *)base, RAMDISK_MAGIC, RAMDISK_MAGIC_SIZE) != 0) {
        printf("bad ramdisk format\n");
        return -EBADF;
    }

    sb = (struct ramdisk_sb *)(base+RAMDISK_MAGIC_SIZE);
    root = (struct ramdisk_inode *)(base + sb->inode_offset);
    ramdisk_data = (void *)(base+sb->data_offset);

    ramdisk_tree();
    return 0;
}