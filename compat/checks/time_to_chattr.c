#include <linux/module.h>
#include <linux/iversion.h>

static inline u64 test(struct timespec64 *t)
{
	return time_to_chattr(t);
}

MODULE_LICENSE("GPL");
