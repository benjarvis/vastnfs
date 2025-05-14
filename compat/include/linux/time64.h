#ifndef __COMPAT_LINUX_TIME64_H__
#define __COMPAT_LINUX_TIME64_H__

#include_next <linux/time64.h>

#if defined(timespec64)
/* Linux v4.17 and below */
#define TV_SEC_FORMAT_SPEC "%ld"
#else
#define TV_SEC_FORMAT_SPEC "%lld"
#endif

#endif
