#include <linux/module.h>
#include <linux/fs_parser.h>

int check_type(void)
{
	return fs_param_v_optional;
}

MODULE_LICENSE("GPL");
