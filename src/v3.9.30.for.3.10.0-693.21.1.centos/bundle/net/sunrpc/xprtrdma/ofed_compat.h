#ifndef __OFED_COMPAT_H__
#define __OFED_COMPAT_H__

#include <rdma/ib_verbs.h>
#include <rdma/rdma_vt.h>
#include <linux/mlx5/driver.h>

#if defined(MLX5_LOG_SW_ICM_BLOCK_SIZE)
#include <rdma/rdmavt_cq.h>
#include <rdma/ib_hdrs.h>
#if defined(IB_BTH_OPCODE_CNP)
#define OFED_5_5
#define OFED_5_4
#define OFED_5_3
#define OFED_5_2
#endif
#if defined(RDMA_READ_UAPI_ATOMIC)
#define OFED_5_1
#endif
#define OFED_5_0
#define OFED_4_7
#define OFED_4_6
#define OFED_4_5
#define OFED_4_4
#define OFED_4_3_3
#define OFED_4_3_1
#define OFED_4_2
#elif defined(INIT_RDMA_OBJ_SIZE)
#define OFED_4_7
#define OFED_4_6
#define OFED_4_5
#define OFED_4_4
#define OFED_4_3_3
#define OFED_4_3_1
#define OFED_4_2
#elif defined(RVT_SGE_COPY_MEMCPY)
#define OFED_4_6
#define OFED_4_5
#define OFED_4_4
#define OFED_4_3_3
#define OFED_4_3_1
#define OFED_4_2
#elif defined(RDMA_CORE_CAP_IB_GRH_REQUIRED)
#define OFED_4_5
#define OFED_4_4
#define OFED_4_3_3
#define OFED_4_3_1
#define OFED_4_2
#elif defined(MLX5_CAP_DEBUG)
#define OFED_4_4
#define OFED_4_3_3
#define OFED_4_3_1
#define OFED_4_2
#elif defined(ib_alloc_pd_notrack)
#define OFED_4_3_3
#define OFED_4_3_1
#define OFED_4_2
#elif defined(OPA_GID_INDEX)
#define OFED_4_3_1
#define OFED_4_2
#else
#define OFED_4_2
#endif

#ifdef OFED_4_6
#define IB_DEFINE_BAD_WR(_type_, _name_) const struct _type_ *_name_
#else
#define IB_DEFINE_BAD_WR(_type_, _name_) struct _type_ *_name_
#endif

#ifdef OFED_4_7
#define TRACE_MR_ID(x) (x)->res.id
#else
#define TRACE_MR_ID(x) 0
#endif

#ifdef OFED_4_6

static inline int ib_device_attr__max_send_sge(const struct ib_device_attr *attr) {
    return attr->max_send_sge;
}

#else

static inline int ib_device_attr__max_send_sge(const struct ib_device_attr *attr) {
    return attr->max_sge;
}

#endif

#endif
