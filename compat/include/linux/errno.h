#ifndef __COMPAT_LINUX_ERRNO_H__
#define __COMPAT_LINUX_ERRNO_H__

#include_next <linux/errno.h>

#ifndef ENOGRACE
#define ENOGRACE	531	/* NFS file lock reclaim refused */
#endif

#endif
