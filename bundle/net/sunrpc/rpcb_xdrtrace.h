/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022 Dan Aloni <dan.aloni@vastdata.com>
 */
#undef TRACE_SYSTEM
#undef TRACE_INCLUDE_FILE
#define TRACE_SYSTEM sunrpc

#if !defined(_TRACE_RPCB_XDR_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_RPCB_XDR_H

#include <linux/tracepoint.h>
#include "rpcb_trace_classes.h"

DEFINE_RPCB_PROG_EVENT_ENCODE(rpcb_encode_mapping_request);
DEFINE_RPCB_PROG_EVENT_ENCODE(rpcb_encode_getaddr_request);
DEFINE_RPCB_PROG_EVENT_DECODE(rpcb_decode_set_response);
DEFINE_RPCB_PROG_EVENT_DECODE(rpcb_decode_getport_response);
DEFINE_RPCB_PROG_EVENT_DECODE(rpcb_decode_getaddr_response);

#endif /* _TRACE_RPCB_XDR_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE rpcb_xdrtrace
/* This part must be outside protection */
#include <trace/define_trace.h>
