#ifndef __COMPAT_LINUX_XATTR_H__
#define __COMPAT_LINUX_XATTR_H__

#include_next <linux/xattr.h>

#ifndef COMPAT_DETECT_XATTR_SUPPORTS_USER_PREFIX

#ifndef COMPAT_DETECT_XATTR_SUPPORTED_NAMESPACE

extern int xattr_supported_namespace(struct inode *inode, const char *prefix);

#endif

extern int xattr_supports_user_prefix(struct inode *inode);

#endif

#endif
