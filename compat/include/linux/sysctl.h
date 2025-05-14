#ifndef __COMPAT_SYSCTL_H__
#define __COMPAT_SYSCTL_H__

#include_next <linux/sysctl.h>

#ifndef SYSCTL_ZERO

#define IMPL_COMPAT_SYSCTL_NUMBERS
#define SYSCTL_ZERO    ((void *)&compat_sysctl_vals[0])
#define SYSCTL_ONE     ((void *)&compat_sysctl_vals[1])
#define SYSCTL_INT_MAX ((void *)&compat_sysctl_vals[2])
extern const int compat_sysctl_vals[];

#endif

#endif
