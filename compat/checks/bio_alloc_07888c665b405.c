#include <linux/module.h>
#include <linux/bio.h>

struct bio *test_dummy(struct block_device *bdev,
		       unsigned short nr_vecs, unsigned int opf, gfp_t gfp_mask)
{
	return bio_alloc(bdev, nr_vecs, opf, gfp_mask);
}

MODULE_LICENSE("GPL");
