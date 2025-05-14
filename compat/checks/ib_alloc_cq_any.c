#include <linux/module.h>
#include <rdma/ib_verbs.h>

struct ib_cq *check_type(struct ib_device *desc,
			 void *private, int nr_cqe,
			 enum ib_poll_context poll_ctx)
{
	return ib_alloc_cq_any(desc, private, nr_cqe, poll_ctx);
}

MODULE_LICENSE("GPL");
