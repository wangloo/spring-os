/*
 * Copyright (C) 2018 Min Le (lemin9538@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <kernel.h>
#include <memlayout.h>
#include <page.h>
// #include <memattr.h>
#include <cfi.h>
#include <ramdisk.h>

// Set the virtual address region of RAMDISK
// Because we are in kernel, so feel free to use va directly
// The address map are done by kvspace_init(), RAMDISK is one of 
// the static memory regions.
static const vaddr_t ramdisk_start = ptov(RAMDISK_BASE);
static const vaddr_t ramdisk_end = ptov(RAMDISK_BASE+RAMDISK_SIZE);

static struct ramdisk_inode *root;
static struct ramdisk_sb *sb;
static void *ramdisk_data;


// 这是一个临时的函数, 因为目前没有uboot, 没办法将ramdisk.bin
// 在启动时放入内存, 只能暂时放入pflash中, 这个是qemu启动项支持的.
// 然而这时就需要做一个从pflash拷贝到ramdisk指定内存位置的拷贝过程
static void 
ramdisk_copy_from_flash(void)
{
    u64 offset = 0;
    cfi_read((void *)ptov(RAMDISK_BASE), offset, RAMDISK_SIZE);
}


const char *ramdisk_file_name(struct ramdisk_file *file)
{
    return file->inode->f_name;
}

unsigned long ramdisk_file_size(struct ramdisk_file *file)
{
    return file->inode->f_size;
}

unsigned long ramdisk_file_base(struct ramdisk_file *file)
{
    return (unsigned long)ramdisk_data + file->inode->f_offset;
}


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
        LOG_ERROR("super block not found, forget to init ramdisk?\n");
        return -ENOENT;
    }

    inode = __ramdisk_open(name);
    if (!inode) {
        LOG_ERROR("open %s: file not found!\n", name);
        return -ENOENT;
    }

    memset(fd, 0, sizeof(struct ramdisk_file));
    fd->inode = inode;
    return 0;
}

// Init kernel component -- RAMDISK
// Return < 0 if error
int 
init_ramdisk(void)
{
    // Fill the content of RAMDISK
    ramdisk_copy_from_flash();

    if (!ramdisk_start || !ramdisk_end) {
        LOG_ERROR("ramdisk address is not set yet\n");
        return -EINVAL;
    }
    if (!page_aligned(ramdisk_start)) {
        LOG_ERROR("ramdisk start address need PAGE align\n");
        return -EINVAL;
    }
    if (strncmp((void *)ramdisk_start, RAMDISK_MAGIC, RAMDISK_MAGIC_SIZE) != 0) {
        LOG_ERROR("bad ramdisk format\n");
        return -EBADF;
    }

    sb = (struct ramdisk_sb *)(ramdisk_start+RAMDISK_MAGIC_SIZE);
    root = (struct ramdisk_inode *)(ramdisk_start + sb->inode_offset);
    ramdisk_data = (void *)(ramdisk_start+sb->data_offset);
    LOG_DEBUG("init ok\n");
    return 0;
}