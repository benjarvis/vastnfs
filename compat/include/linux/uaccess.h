#ifndef __COMPAT_LINUX_UACCESS_H__
#define __COMPAT_LINUX_UACCESS_H__

#include_next <linux/uaccess.h>

#if !defined(COMPAT_DETECT_ACCESS_OK)
#define compat_access_ok(type, addr, len) access_ok(type, addr, len)
#else
#define compat_access_ok(type, addr, len) access_ok(addr, len)
#endif

#endif
