#ifndef __COMPAT_LINUX_MM_H__
#define __COMPAT_LINUX_MM_H__

#include_next <linux/mm.h>
#include <linux/mm_types.h>

static inline void compat_set_page_writeback(struct page *page)
{
#ifdef COMPAT_DETECT_TEST_SET_PAGE_WRITEBACK
	int ret = test_set_page_writeback(page);
	WARN_ON_ONCE(ret != 0);
#else
	set_page_writeback(page);
#endif
}

#ifdef COMPAT_DETECT_PAGE_FOLIO_5_19
static inline struct page* compat_folio_page(struct folio *folio)
{
#ifdef CONFIG_NFS_CRASH_ON_WARN
	BUG_ON(folio_nr_pages(folio) != 1);
#else
	WARN_ON_ONCE(folio_nr_pages(folio) != 1);
#endif
	return &folio->page;
}
#endif /* COMPAT_DETECT_PAGE_FOLIO_5_19 */

#endif
