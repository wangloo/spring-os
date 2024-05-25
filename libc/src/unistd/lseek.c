#include <unistd.h>
#include "syscall.h"

// off_t __lseek(int fd, off_t offset, int whence)
// {
// #ifdef SYS__llseek
// 	off_t result;
// 	return syscall(SYS__llseek, fd, offset>>32, offset, &result, whence) ? -1 : result;
// #else
// 	return syscall(SYS_lseek, fd, offset, whence);
// #endif
// }
off_t __lseek(int fd, off_t off, int whence);
off_t lseek(int fd, off_t off, int whence)
{
	return __lseek(fd, off, whence);
}
