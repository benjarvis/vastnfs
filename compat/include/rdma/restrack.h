#ifndef __COMPAT_RDMA_RESTRACK_H__
#define __COMPAT_RDMA_RESTRACK_H__

#if defined(COMPAT_DETECT_INCLUDE_RDMA_RESTRACK)
#include_next <rdma/restrack.h>
#endif

#if defined(COMPAT_DETECT_RDMA_RESTRACK_ENTRY_ID)
#define RDMA_RESTRACK_ENTRY_ID(v) ((v)->id)
#else
#define RDMA_RESTRACK_ENTRY_ID(v) 0
#endif

#endif
