#ifndef __COMPAT_LINUX_VMALLOC_H__
#define __COMPAT_LINUX_VMALLOC_H__

#include_next <linux/vmalloc.h>

#ifndef COMPAT_DETECT_F__VMALLOC_2PARAMS
static inline void *compat__vmalloc(unsigned long size, gfp_t gfp_mask)
{
	return __vmalloc(size, gfp_mask, PAGE_KERNEL);
}
#define __vmalloc compat__vmalloc
#endif

#endif
