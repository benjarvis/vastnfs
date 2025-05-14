#ifndef __COMPAT_BIO_H__
#define __COMPAT_BIO_H__

#ifdef COMPAT_DETECT_INCLUDE_LINUX_MINMAX
#include <linux/minmax.h>
#endif

#include_next <linux/bio.h>

#ifndef COMPAT_DETECT_BIO_MAX_SEGS
static inline unsigned int bio_max_segs(unsigned int nr_segs)
{
	return min(nr_segs, (unsigned int)BIO_MAX_PAGES);
}
#endif

#endif
