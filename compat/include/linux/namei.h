#ifndef __COMPAT_LINUX_NAMEI_H__
#define __COMPAT_LINUX_NAMEI_H__

#include_next <linux/namei.h>

#ifndef COMPAT_DETECT_LOOKUP_POSITIVE_UNLOCKED
extern struct dentry *lookup_positive_unlocked(const char *, struct dentry *, int);
#endif

#endif
