#include <linux/module.h>
#include <linux/mount.h>

static int mode;

static int param_set_pool_mode(const char *val, const struct kernel_param *kp)
{
	return 0;
}

static int param_get_pool_mode(char *buf, const struct kernel_param *kp)
{
	return 0;
}

module_param_call(pool_mode, param_set_pool_mode, param_get_pool_mode,
		 &mode, 0644);

MODULE_LICENSE("GPL");
