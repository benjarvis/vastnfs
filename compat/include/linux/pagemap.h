#ifndef __COMPAT_LINUX_PAGEMAP_H__
#define __COMPAT_LINUX_PAGEMAP_H__

#include_next <linux/pagemap.h>

#if !defined(COMPAT_DETECT_PAGE_CACHE_NEXT_MISS)
#define page_cache_next_miss page_cache_next_hole
#endif

#if !defined(COMPAT_DETECT_WAIT_ON_PAGE_LOCKED_KILLABLE)
static inline int wait_on_page_locked_killable(struct page *page)
{
       return folio_wait_locked_killable(page_folio(page));
}
#endif

#endif
