#include <linux/module.h>
#include <linux/shrinker.h>

static struct shrinker binder_shrinker;

int check_type(void)
{
	return register_shrinker(&binder_shrinker, "name");
}

MODULE_LICENSE("GPL");
