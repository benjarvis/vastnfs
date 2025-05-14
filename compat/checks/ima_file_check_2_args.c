#include "linux/module.h"
#include "linux/ima.h"

void test_wrapper(struct file *file)
{
	ima_file_check(file, 0);
}

MODULE_LICENSE("GPL");
