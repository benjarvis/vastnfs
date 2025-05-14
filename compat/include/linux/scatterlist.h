#ifndef __COMPAT_LINUX_SCATTERLIST_H__
#define __COMPAT_LINUX_SCATTERLIST_H__

#include_next <linux/scatterlist.h>

#if !defined(COMPAT_DETECT_SG_ALLOC_TABLE_CHAINED_MAX)

static inline int compat_sg_alloc_table_chained(struct sg_table *table, int nents,
						struct scatterlist *first_chunk,
						unsigned nents_first_chunk)
{
	BUG_ON(nents_first_chunk != SG_CHUNK_SIZE);
	return sg_alloc_table_chained(table, nents, first_chunk);
}

static inline void compat_sg_free_table_chained(struct sg_table *table, int nents)
{
	BUG_ON(nents != SG_CHUNK_SIZE);
	sg_free_table_chained(table, true);
}

#else

#define compat_sg_alloc_table_chained sg_alloc_table_chained
#define compat_sg_free_table_chained sg_free_table_chained

#endif

#endif
