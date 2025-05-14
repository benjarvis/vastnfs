#ifndef _LINUX_OLD_KERNEL_H
#define _LINUX_OLD_KERNEL_H

#include <linux/cred.h>

static inline const struct cred *newer_get_cred(const struct cred *cred)
{
	if (!cred)
		return NULL;
	return get_cred(cred);
}

static inline void newer_put_cred(const struct cred *cred)
{
	if (!cred)
		return;
	put_cred(cred);
}

#define get_cred newer_get_cred
#define put_cred newer_put_cred

#endif
