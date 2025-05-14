#ifndef __COMPAT_LINUX_PAGEVEC_H__
#define __COMPAT_LINUX_PAGEVEC_H__

#include_next <linux/pagevec.h>

#ifdef COMPAT_DETECT_INCLUDE_BVEC
#include <linux/bvec.h>
#endif

#if defined(COMPAT_DETECT_PAGEVEC_INIT_2_5_45)
static inline void compat_pagevec_init(struct pagevec *pvec)
{
	pagevec_init(pvec, 0);
}
#define pagevec_init compat_pagevec_init
#endif

#endif
