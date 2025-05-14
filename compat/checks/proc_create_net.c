#include <linux/module.h>
#include <linux/proc_fs.h>

struct proc_dir_entry *check_type(const char *name, umode_t mode,
		struct proc_dir_entry *parent, const struct seq_operations *ops,
		unsigned int state_size)
{
	return proc_create_net(name, mode, parent, ops, state_size);
}

MODULE_LICENSE("GPL");
