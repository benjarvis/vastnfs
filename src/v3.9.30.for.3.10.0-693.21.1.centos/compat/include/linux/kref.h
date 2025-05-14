#ifndef __COMPAT_LINUX_KREF_H__
#define __COMPAT_LINUX_KREF_H__

#include_next <linux/kref.h>

#if !defined(COMPAT_DETECT_KREF_READ)
static inline int kref_read(const struct kref *kref)
{
        return atomic_read(&kref->refcount);
}
#endif

#endif
