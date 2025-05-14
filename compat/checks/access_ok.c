#include <linux/module.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

bool check_type(void *addr, size_t size)
{
	return access_ok(addr, size);
}

MODULE_LICENSE("GPL");
