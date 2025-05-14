#ifndef __COMPAT_LINUX_UIO_H__
#define __COMPAT_LINUX_UIO_H__

#include_next <linux/uio.h>

#if !defined(COMPAT_DETECT_IOV_ITER_ITER_TYPE) && \
    !defined(COMPAT_DETECT_IOV_ITER_NO_DIRECTION)

/* Iter before: iov_iter: Separate type from direction and use accessor functions */

static inline
void compat_iov_iter_kvec(struct iov_iter *i, unsigned int direction,
			  const struct kvec *kvec,
			  unsigned long nr_segs, size_t count)
{
	iov_iter_kvec(i, direction | ITER_KVEC, kvec, nr_segs, count);
}

static inline
void compat_iov_iter_bvec(struct iov_iter *i, unsigned int direction,
			  const struct bio_vec *bvec,
			  unsigned long nr_segs, size_t count)
{
	iov_iter_bvec(i, direction | ITER_BVEC, bvec, nr_segs, count);
}

#else

#define compat_iov_iter_bvec iov_iter_bvec
#define compat_iov_iter_kvec iov_iter_kvec

#endif

#if !defined(COMPAT_DETECT_IOV_ITER_GET_PAGES_2_6_0)

static inline ssize_t iov_iter_get_pages2(struct iov_iter *i, struct page **pages,
                       size_t maxsize, unsigned maxpages, size_t *start)
{
       ssize_t res = iov_iter_get_pages(i, pages, maxsize, maxpages, start);

       if (res >= 0)
               iov_iter_advance(i, res);
       return res;
}

static inline ssize_t iov_iter_get_pages_alloc2(struct iov_iter *i, struct page ***pages,
                       size_t maxsize, size_t *start)
{
       ssize_t res = iov_iter_get_pages_alloc(i, pages, maxsize, start);

       if (res >= 0)
               iov_iter_advance(i, res);
       return res;
}

#endif

#if !defined(COMPAT_DETECT_IOV_ITER_IS_KVEC)
static inline int iov_iter_type(const struct iov_iter *i)
{
       return i->type & ~(READ | WRITE);
}

static inline bool iov_iter_is_kvec(const struct iov_iter *i)
{
       return iov_iter_type(i) == ITER_KVEC;
}
#endif

#endif
