#ifndef __COMPAT_RDMA_RDMA_CM_H__
#define __COMPAT_RDMA_RDMA_CM_H__

#include_next <rdma/rdma_cm.h>

#ifndef COMPAT_DETECT_RDMA_LOCK_HANDLER
static inline void rdma_lock_handler(struct rdma_cm_id *id) {
}

static inline void rdma_unlock_handler(struct rdma_cm_id *id) {
}
#endif

#endif
