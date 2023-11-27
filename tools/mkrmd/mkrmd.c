#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <uapi/ramdisk.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>

#define MODE_FILE 0
#define MODE_DIR  1

#define IMEM_BLOCK_SIZE 32 * 1024 * 1024
#define FMEM_BLOCK_SIZE 64 * 1024 * 1024

struct mem_block {
  size_t size;
  size_t used;
  size_t free;
  void *mem;
};

struct mem_block imem;
struct mem_block fmem;

struct ramdisk_sb superblk;

const char *mkrmd_usage = 
              "mkrmd -f <out file> <in file>\n";

static void 
mem_block_init(void)
{
  imem.mem = malloc(IMEM_BLOCK_SIZE);
  imem.size = imem.free = IMEM_BLOCK_SIZE;
  imem.used = 0;

  fmem.mem = malloc(FMEM_BLOCK_SIZE);
  fmem.size = fmem.free = FMEM_BLOCK_SIZE;
  fmem.used = 0;

	if (!imem.mem || !fmem.mem) {
		printf("alloc memory for ramdisk failed\n");
		exit(-ENOMEM);
	}
}

// Filename we pass to mkrmd include misleading prefix
static void
clean_file_name(char **name)
{
  // Redundant and meaningless prefix
  char *prefix = "out/ramdisk/";
  
  assert(strstr(*name, prefix) == *name);
  *name += strlen(prefix);
}

static int 
__add_file(int fd, char *filename)
{
  struct ramdisk_inode *inode;

  if (imem.free < sizeof(*inode))
    return -1;
  
  inode = (struct ramdisk_inode *)(imem.mem + imem.used);
  imem.used += sizeof(*inode);
  imem.free -= sizeof(*inode);
  inode->f_offset = fmem.used;
  strcpy(inode->f_name, filename);
  
  // fill file data
  size_t readsize;
  size_t filesize = 0, filesize_aligned;
  size_t padding;
  size_t cnt;
  while (1) {
    if (fmem.free == 0) {
      printf("no more memory in fmemblock\n");
      return -ENOMEM;
    }

    readsize = (fmem.free > 4096) ? 4096 : fmem.free;
    cnt = read(fd, fmem.mem+fmem.used, readsize);
    if (cnt == 0)
      break;

    fmem.used += cnt;
    fmem.free -= cnt;
    filesize += cnt;
  }
  filesize_aligned = (filesize + 4095) & ~4095;
  padding = filesize_aligned - filesize;
  if (padding > 0) {
    memset(fmem.mem+fmem.used, 0, padding);
    fmem.used += padding;
    fmem.free -= padding; // fmem.free<padding ==> BUG? 
  }
  inode->f_size = filesize;
  superblk.file_cnt += 1;

  printf("add file %s, offset: %ld, filesize: %ld\n", 
            filename, inode->f_offset, inode->f_size);
  return 0;
}

static int 
add_file(char *path)
{
  char *last_slash;
  int fd, ret=0;

  if ((fd = open(path, O_RDONLY)) < 0) {
    printf("open file %s failed!\n", path);
    return fd;
  }

  clean_file_name(&path);
  
  ret = __add_file(fd, path);

out:
  close(fd);
  return ret;
}


static int 
make_ramdisk_files(int cnt, char **files)
{
  int i;

  for (i = 0; i < cnt; i++) {
    if (add_file(files[i]))
      return -ENOENT;
  }
  return 0;
}

static int
make_ramdisk_dir(char *dir)
{
  DIR *dp;
  char path[256];
  struct dirent *dirp;
  size_t dirlen = strlen(dir);

  if (dir[dirlen-1] == '/')
    dir[dirlen-1] = 0;
  
  if ((dp = opendir(dir)) == NULL) {
    printf("can not open directory %s\n", dir);
    return -ENOENT;
  }

  while ((dirp = readdir(dp)) != NULL) {
    if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
      continue;

    if (dirp->d_type != 8) {
      printf("skip %s\n", dirp->d_name);
      continue;
    }
    if ((strlen(dir) + strlen(dirp->d_name)) > 255
        || (strlen(dirp->d_name) + 1) > RAMDISK_FNAME_SIZE) {
      printf("skip file %s: filename too long\n", dirp->d_name);
      continue;
    }

    sprintf(path, "%s/%s", dir, dirp->d_name);
    add_file(path);
  }

  closedir(dp);
  return 0;
}




static int
pack_ramdisk(int fd)
{
  ssize_t cnt;
  size_t inode_offset, data_offset;

  // 1. write MAGIC NUMBER
  cnt = write(fd, RAMDISK_MAGIC, RAMDISK_MAGIC_SIZE);
  assert(cnt == RAMDISK_MAGIC_SIZE);

  // 2. aligned to write SUPER BLOCK
  inode_offset = RAMDISK_MAGIC_SIZE + sizeof(struct ramdisk_sb);
  inode_offset = (inode_offset+15) & ~15;
  superblk.inode_offset = inode_offset;

  data_offset = inode_offset + imem.used;
  data_offset = (data_offset+4095) & ~4095;
  superblk.data_offset = data_offset;

  superblk.block_size = 4096; // fixed
  superblk.ramdisk_size = data_offset + fmem.used;

  cnt = write(fd, &superblk, sizeof(struct ramdisk_sb));
  assert(cnt == sizeof(struct ramdisk_sb));
  
  // 3. write inodes
  cnt = lseek(fd, inode_offset, SEEK_SET);
  assert(cnt == inode_offset);

  cnt = write(fd, imem.mem, imem.used);
  assert(cnt == imem.used);

  // 4. write file data
  cnt = lseek(fd, data_offset, SEEK_SET);
  assert(cnt == data_offset);

  cnt = write(fd, fmem.mem, fmem.used);
  assert(cnt == fmem.used);

  return 0;
}

int 
main(int argc, char **argv)
{
  char opt;
  int mode;
  int outfd;
  int ret;
  
  while ((opt = getopt(argc, argv, "fd")) != -1) {
    switch(opt) {
    case 'f':
      mode = MODE_FILE;
      break;
    case 'd':
      mode = MODE_DIR;
      break;
    default:
      printf("Unsupported option\n");
      exit(1);
    }
  }

  // printf("mkrmd with mode: %s\n", mode == MODE_DIR? "DIR" : "FILE");

  outfd = open(argv[optind], O_RDWR | O_CREAT, 0666);
  if (outfd < 0) {
    printf("can not open/create file: %s\n", argv[optind]);
    return outfd;
  }

  mem_block_init();

  if (mode == MODE_FILE) {
    ret = make_ramdisk_files(argc-3, &argv[3]);
  } else {
    ret = make_ramdisk_dir(argv[3]);
  }
  if (ret) {
		printf("create %s failed wit %d\n", argv[2], ret);
		goto out;
  }

  ret = pack_ramdisk(outfd);
  if (ret)
    goto out;
  
  printf("generate ramdisk: %s done, size: %ld\n", 
            argv[optind], superblk.ramdisk_size);
  printf("including %d files, inodes offset: %ld, data offset: %ld\n",
          superblk.file_cnt, superblk.inode_offset, superblk.data_offset);
      
out:
  close(outfd); 
  return ret;
}
