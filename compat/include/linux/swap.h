#ifndef __COMPAT_SWAP_H__
#define __COMPAT_SWAP_H__

#include_next <linux/swap.h>

static inline unsigned long compat_totalram_pages(void)
{
#ifndef COMPAT_DETECT_TOTALRAM_PAGES
	return totalram_pages;
#else
	return totalram_pages();
#endif
}

static inline unsigned long compat_totalhigh_pages(void)
{
#ifndef COMPAT_DETECT_TOTALRAM_PAGES
	return totalhigh_pages;
#else
	return totalhigh_pages();
#endif
}

#endif
