#ifndef __COMPAT_LINUX_BLKDEV_H__
#define __COMPAT_LINUX_BLKDEV_H__

#include_next <linux/blkdev.h>
#include_next <linux/blk-mq.h>

#if defined(COMPAT_DETECT_BLK_EXECUTE_REQ_5_16)
static inline blk_status_t compat_blk_execute_rq(
	struct gendisk *bd_disk, struct request *rq, int at_head)
{
	blk_execute_rq(rq, at_head);
	return 0;
}
#elif defined(COMPAT_DETECT_BLK_EXECUTE_REQ_5_14)
static inline blk_status_t compat_blk_execute_rq(
	struct gendisk *bd_disk, struct request *rq, int at_head)
{
	blk_execute_rq(bd_disk, rq, at_head);
	return 0;
}
#define blk_execute_rq compat_blk_execute_rq
#elif !defined(COMPAT_DETECT_BLK_EXECUTE_REQ_3_PARAMS) && \
    !defined(COMPAT_DETECT_BLK_EXECUTE_REQ_3_PARAMS_RETVAL)
static inline blk_status_t compat_blk_execute_rq(
	struct gendisk *bd_disk, struct request *rq, int at_head)
{
	blk_execute_rq(rq->q, bd_disk, rq, at_head);
	return 0;
}
#define blk_execute_rq compat_blk_execute_rq
#endif

#ifndef COMPAT_DETECT_BDEV_IS_PARTITION
static inline bool bdev_is_partition(struct block_device *bdev)
{
       return bdev->bd_partno;
}
#endif

#endif
