/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022 Dan Aloni <dan.aloni@vastdata.com>
 */
#undef TRACE_SYSTEM
#undef TRACE_INCLUDE_FILE
#define TRACE_SYSTEM nfs

#if !defined(_TRACE_NLM4_XDR_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_NLM4_XDR_H

#include <linux/tracepoint.h>

#include "nlm4trace_classes.h"

DEFINE_NLM4_PROG_EVENT_ENCODE(nlm4_encode_test_request);
DEFINE_NLM4_PROG_EVENT_ENCODE(nlm4_encode_testres_request);
DEFINE_NLM4_PROG_EVENT_ENCODE(nlm4_encode_lock_request);
DEFINE_NLM4_PROG_EVENT_ENCODE(nlm4_encode_canc_request);
DEFINE_NLM4_PROG_EVENT_ENCODE(nlm4_encode_unlock_request);
DEFINE_NLM4_PROG_EVENT_ENCODE(nlm4_encode_res_request);

DEFINE_NLM4_PROG_EVENT_DECODE(nlm4_decode_testres_response);
DEFINE_NLM4_PROG_EVENT_DECODE(nlm4_decode_test_response);
DEFINE_NLM4_PROG_EVENT_DECODE(nlm4_decode_res_response);
DEFINE_NLM4_PROG_EVENT_DECODE(nlm4_decode_norep_response);

#endif /* _TRACE_NLM4_XDR_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE nlm4xdrtrace
/* This part must be outside protection */
#include <trace/define_trace.h>
