#ifndef __COMPAT_RDMA_IB_VERBS_H__
#define __COMPAT_RDMA_IB_VERBS_H__

#include_next <rdma/ib_verbs.h>

#if defined(COMPAT_DETECT_IB_KERNEL_CAPS)
/* e945c653c8e - RDMA: Split kernel-only global device caps from uverbs device caps */
#define IB_DEVICE_SG_GAPS_REG IBK_SG_GAPS_REG
#endif

#if !defined(COMPAT_DETECT_IB_ALLOC_CQ_ANY)
static inline struct ib_cq *ib_alloc_cq_any(struct ib_device *dev,
					    void *private, int nr_cqe,
					    enum ib_poll_context poll_ctx)
{
	return ib_alloc_cq(dev, private, nr_cqe, 0, poll_ctx);
}
#endif

#if defined(COMPAT_DETECT_IB_MAX_SEND_SGE)
static inline int compat_attr_get_max_send_sge(const struct ib_device_attr *attrs)
{
	return attrs->max_send_sge;
}
#else
static inline int compat_attr_get_max_send_sge(const struct ib_device_attr *attrs)
{
	return attrs->max_sge;
}
#endif

#if defined(COMPAT_DETECT_IB_WR_CONST)
#define COMPAT_IB_WR_CONST const
#else
#define COMPAT_IB_WR_CONST
#endif

#endif
