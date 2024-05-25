#include <unistd.h>
#include "syscall.h"

uid_t getuid(void)
{
	// return __syscall(SYS_getuid);
	return -1;
}
