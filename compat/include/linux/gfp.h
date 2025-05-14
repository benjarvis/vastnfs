#ifndef __COMPAT_GFP_H__
#define __COMPAT_GFP_H__

#include_next <linux/kernel.h>
#include_next <linux/gfp.h>

#ifndef COMPAT_DETECT_ALLOC_PAGES_BULK_ARRAY
static inline unsigned long
alloc_pages_bulk_array(gfp_t gfp, unsigned long nr_pages, struct page **page_array)
{
	int i, allocated = 0;

	for (i = 0; i < nr_pages; i++) {
		if (page_array[i]) {
			allocated += 1;
			continue;
		}

		page_array[i] = alloc_page(gfp);
		if (page_array[i])
			allocated += 1;
	}

	return allocated;
}
#endif

#endif
