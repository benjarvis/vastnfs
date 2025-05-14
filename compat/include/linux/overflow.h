#ifndef __COMPAT_OVERFLOW_H__
#define __COMPAT_OVERFLOW_H__

#ifdef COMPAT_DETECT_INCLUDE_OVERFLOW
#include_next <linux/overflow.h>
#endif

#ifndef struct_size
/* No overflow checks */
#define struct_size(p, member, count) \
	(sizeof(*p) + (sizeof(p->member[0]) * count))
#endif

#endif
