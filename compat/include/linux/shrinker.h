#ifndef __COMPAT_LINUX_SHRINKER_H__
#define __COMPAT_LINUX_SHRINKER_H__

#include_next <linux/shrinker.h>

#ifndef COMPAT_DETECT_DYNAMIC_SHRINKER_ALLOC

#if !defined(COMPAT_DETECT_REGISTER_SHRINKER_WITH_NAME_6_0)
static inline int __printf(2, 3) compat_register_shrinker(struct shrinker *shrinker,
							  const char *fmt, ...)
{
	return register_shrinker(shrinker);
}
#define register_shrinker compat_register_shrinker
#endif

#endif /* COMPAT_DETECT_DYNAMIC_SHRINKER_ALLOC */
#endif
