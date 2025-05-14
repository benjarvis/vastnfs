#ifndef COMPAT_LINUX_KREF_H
#define COMPAT_LINUX_KREF_H

static inline int kref_read(const struct kref *kref)
{
	return atomic_read(&kref->refcount);
}

#endif /* COMPAT_LINUX_KREF_H */
