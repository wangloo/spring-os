#include <uspace/kobject.h>
#include <uspace/uaccess.h>
#include <console.h>
#include <assert.h>
#include <task.h>
#include <current.h>

#define STDIO_BUF_SIZE	2048

struct kobject stdio_kobj;
static char stdio_out_buf[STDIO_BUF_SIZE];

long stdio_send (struct kobject *kobj, void __user *data, size_t data_size,
        void __user *extra, size_t extra_size, uint32_t timeout)
{
    size_t left = data_size;
	int copy;
	int ret = 0;

	if (left <=0 )
		return 0;
	while (left > 0) {
		copy = left > STDIO_BUF_SIZE ? STDIO_BUF_SIZE : left;
		ret = copy_from_user(stdio_out_buf, data, copy);
		if (ret <= 0)
			goto out;

		console_puts(stdio_out_buf, copy);
		left -= copy;
		data += copy;
	}
            
    ret = data_size;
out:
    return ret;
}

long stdio_recv(struct kobject *kobj, void __user *data, size_t data_size,
            size_t *actual_data, void __user *extra, size_t extra_size,
            size_t *actual_extra, uint32_t timeout)
{
    // TODO
    assert(0);
    return 0;
}

struct kobject_ops stdio_ops = {
    .send = stdio_send,
    .recv = stdio_recv,
};


int stdio_kobject_init(void)
{
	kobject_init(&stdio_kobj, KOBJ_TYPE_STDIO, KOBJ_RIGHT_RW, &stdio_ops, 0);
	kobject_get(&stdio_kobj);
	// mutex_init(&stdio_wlock);

	return 0;
}