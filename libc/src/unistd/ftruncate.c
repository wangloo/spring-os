#include <unistd.h>
#include "syscall.h"

int ftruncate(int fd, off_t length)
{
	// return syscall(SYS_ftruncate, fd, __SYSCALL_LL_O(length));
	return -1;
}

weak_alias(ftruncate, ftruncate64);
