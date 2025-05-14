/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022 Dan Aloni <dan.aloni@vastdata.com>
 */
#undef TRACE_SYSTEM
#undef TRACE_INCLUDE_FILE
#define TRACE_SYSTEM nfs

#if !defined(_TRACE_NSM_XDR_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_NSM_XDR_H

#include <linux/tracepoint.h>

#include "nsmtrace_classes.h"

DEFINE_NSM_PROG_EVENT_ENCODE(nsm_encode_mon_request);
DEFINE_NSM_PROG_EVENT_ENCODE(nsm_encode_unmon_request);

DEFINE_NSM_PROG_EVENT_DECODE(nsm_decode_stat_res_response);
DEFINE_NSM_PROG_EVENT_DECODE(nsm_decode_stat_response);

#endif /* _TRACE_NSM_XDR_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE nsmxdrtrace
/* This part must be outside protection */
#include <trace/define_trace.h>
