#include "linux/module.h"
#include "linux/fsnotify.h"

void test_wrapper(void)
{
	fsnotify_wait_marks_destroyed();
}

MODULE_LICENSE("GPL");
