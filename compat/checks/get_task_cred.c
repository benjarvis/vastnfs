#include <linux/module.h>
#include <linux/cred.h>

const struct cred *test_dummy(struct task_struct *task) {
	return get_task_cred(task);
}

MODULE_LICENSE("GPL");
