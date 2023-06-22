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

#include <ramdisk.h>
#include <addrspace.h>
#include <string.h>
#include <errno.h>
#include <page.h>
#include <memattr.h>
#include <print.h>
#include <config/config.h>
#include <cfi.h>

void *ramdisk_start = (void *)CONFIG_RAMDISK_BASE;
void *ramdisk_end = (void *)(CONFIG_RAMDISK_BASE + CONFIG_RAMDISK_SIZE);

static struct ramdisk_inode *root;
static struct ramdisk_sb *sb;
static void *ramdisk_data;

void set_ramdisk_address(void *start, void *end)
{
	ramdisk_start = start;
	ramdisk_end = end;
}

// 这是一个临时的函数, 因为目前没有uboot, 没办法将ramdisk.bin
// 在启动时放入内存, 只能暂时放入pflash中, 这个是qemu启动项支持的.
// 然而这时就需要做一个从pflash拷贝到ramdisk指定内存位置的拷贝过程
void ramdisk_copy_from_flash(void)
{
	// memcpy((void *)ptov(CONFIG_RAMDISK_BASE),
	// 		   (void *)ptov(CONFIG_PFLASH_BASE),
  //        CONFIG_RAMDISK_SIZE);
  // memset((void *)ptov(CONFIG_RAMDISK_BASE), 0, 64);
  cfi_read((void *)ptov(CONFIG_RAMDISK_BASE),
              0, CONFIG_RAMDISK_SIZE);

}

int ramdisk_init(void)
{
	unsigned long start = ptov(ramdisk_start);
	// size_t size = ramdisk_end - ramdisk_start;

	/*
	 * need remap the ramdisk memory space, if it
	 * is not in the kernel memory space TBD.
	 */
	if (!ramdisk_start || !ramdisk_end) {
		panic("ramdisk address is not set\n");
		return -EINVAL;
	}

	if (!IS_PAGE_ALIGN(ramdisk_start)) {
		panic("ramdisk start address need PAGE align\n");
		return -EINVAL;
	}

	// if (create_host_mapping(start, (unsigned long)ramdisk_start, size, VM_RO)) {
	// 	panic("unable map ramdisk memory\n");
	// 	return -ENOMEM;
	// }

	if (strncmp((void *)start, RAMDISK_MAGIC, RAMDISK_MAGIC_SIZE) != 0) {
		// destroy_host_mapping(start, size);
		panic("bad ramdisk format\n");
		return -EBADF;
	}

	/*
	 * the ramdisk is read only, init the ramdisk
	 * information, inclue the superblock and the
	 * root inode
	 */
	ramdisk_start = (void *)ptov(ramdisk_start);
	ramdisk_end = (void *)ptov(ramdisk_end);

	sb = ramdisk_start + RAMDISK_MAGIC_SIZE;
	root = ramdisk_start + sb->inode_offset;
	ramdisk_data = ramdisk_start + sb->data_offset;

	return 0;
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

static struct ramdisk_inode *get_file_inode(const char *name)
{
	struct ramdisk_inode *inode;

	for (inode = root; inode < root + sb->file_cnt; inode++) {
		if (strncmp(inode->f_name, name, RAMDISK_FNAME_SIZE - 1) == 0)
			return inode;
	}

	return NULL;
}

int ramdisk_read(struct ramdisk_file *file, void *buf,
		size_t size, unsigned long offset)
{
	if (!file)
		return -EINVAL;

	if ((offset + size) > file->inode->f_size)
		return -EINVAL;

	memcpy(buf, ramdisk_data + file->inode->f_offset + offset, size);
	return 0;
}

int ramdisk_open(char *name, struct ramdisk_file *file)
{
	struct ramdisk_inode *inode;

	if (!sb) {
		printf("super block not found\n");
		return -ENOENT;
	}

	inode = get_file_inode(name);
	if (!inode)
		return -ENOENT;

	memset(file, 0, sizeof(struct ramdisk_file));
	file->inode = inode;

	return 0;
}
