#include <linux/module.h>
#include <linux/uio.h>


/*

v6.0
eba2d3d79: `iov_iter_get_pages_alloc` is renamed to `iov_iter_get_pages_alloc2`.
          "get rid of non-advancing variants"

v6.2
d82076403: 5-arg `iov_iter_get_pages_alloc` is added, and it is a call to `iov_iter_get_pages_alloc2`
           but with zero flags.
          "iov_iter: introduce iov_iter_get_pages_[alloc_]flags()"
 */

ssize_t test_dummy(struct iov_iter *i, struct page **pages,
		size_t maxsize, unsigned maxpages, size_t *start)
{
	return iov_iter_get_pages2(i, pages, maxsize, maxpages, start);
}

MODULE_LICENSE("GPL");
