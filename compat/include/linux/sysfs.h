#ifndef __COMPAT_LINUX_SYSFS_H__
#define __COMPAT_LINUX_SYSFS_H__

#include_next <linux/sysfs.h>

#if !defined(COMPAT_DETECT_SYSFS_CREATE_LINK_NOWARN)
#define sysfs_create_link_nowarn sysfs_create_link
#endif

#if !defined(COMPAT_DETECT_SYSFS_KOBJECT_CONST_6_2)
#define KOBJECT_CONST_DECL
#else
#define KOBJECT_CONST_DECL const
#endif

#endif
