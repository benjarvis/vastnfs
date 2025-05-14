#include <rdma/rdma_cm.h>		/* RDMA connection api */
#include <rdma/ib_verbs.h>		/* RDMA verbs api */

#include "ib_compat.h"
#include "ofed_compat.h"

/**
 * __ib_alloc_cq_any - allocate a completion queue
 * @dev:               device to allocate the CQ for
 * @private:           driver private data, accessible from cq->cq_context
 * @nr_cqe:            number of CQEs to allocate
 * @poll_ctx:          context to poll the CQ from
 * @caller:            module owner name
 *
 * Attempt to spread ULP Completion Queues over each device's interrupt
 * vectors. A simple best-effort mechanism is used.
 */
struct ib_cq *__ib_alloc_cq_any(struct ib_device *dev, void *private,
                               int nr_cqe, enum ib_poll_context poll_ctx,
                               const char *caller)
{
       static atomic_t counter;
       int comp_vector = 0;

       if (dev->num_comp_vectors > 1)
               comp_vector =
                       atomic_inc_return(&counter) %
                       min_t(int, dev->num_comp_vectors, num_online_cpus());
#ifdef OFED_5_0
       return __ib_alloc_cq_user(dev, private, nr_cqe, comp_vector, poll_ctx,
                                 caller, NULL, false);
#else
       return ib_alloc_cq(dev, private, nr_cqe, comp_vector, poll_ctx);
#endif
}
