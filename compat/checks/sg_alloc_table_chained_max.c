#include <linux/module.h>
#include <linux/scatterlist.h>

int test_dummy(struct sg_table *table, int nents,
	       struct scatterlist *first_chunk,
	       unsigned nents_first_chunk)
{
	return sg_alloc_table_chained(table, nents, first_chunk,
				      nents_first_chunk);
}

MODULE_LICENSE("GPL");
