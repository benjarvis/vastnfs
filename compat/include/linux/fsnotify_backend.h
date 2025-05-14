#ifndef __COMPAT_LINUX_FS_NOTIFY_BACKEND_H__
#define __COMPAT_LINUX_FS_NOTIFY_BACKEND_H__

#include_next <linux/fsnotify_backend.h>

#if defined(COMPAT_DETECT_FSNOTIFY_ALLOC_GROUP_2_PARAMS)
/* v5.19-rc1~148^2~15 */

static inline struct fsnotify_group *compat_fsnotify_alloc_group(
	const struct fsnotify_ops *ops, int flags)
{
	return fsnotify_alloc_group(ops, 0);
}

#define fsnotify_alloc_group(ops) \
	compat_fsnotify_alloc_group(ops, 0)
#endif

#endif
