#include <linux/module.h>
#include <linux/moduleparam.h>

int check_type(const char *val, const struct kernel_param *kp,
	       unsigned int min, unsigned int max)
{
	return param_set_uint_minmax(val, kp, min, max);
}

MODULE_LICENSE("GPL");
