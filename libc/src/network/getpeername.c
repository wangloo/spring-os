#include <sys/socket.h>
#include "syscall.h"

int getpeername(int fd, struct sockaddr *restrict addr, socklen_t *restrict len)
{
	return -1;
	// return socketcall(getpeername, fd, addr, len, 0, 0, 0);
}
