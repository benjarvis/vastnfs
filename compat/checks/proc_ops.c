#include "linux/module.h"
#include "linux/proc_fs.h"

struct proc_ops check_type(void)
{
	return (struct proc_ops) {};
}

MODULE_LICENSE("GPL");
