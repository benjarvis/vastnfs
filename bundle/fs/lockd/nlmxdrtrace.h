/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022 Dan Aloni <dan.aloni@vastdata.com>
 */
#undef TRACE_SYSTEM
#undef TRACE_INCLUDE_FILE
#define TRACE_SYSTEM nfs

#if !defined(_TRACE_NLM_XDR_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_NLM_XDR_H

#include <linux/tracepoint.h>

#include "nlmtrace_classes.h"

DEFINE_NLM_PROG_EVENT_ENCODE(nlm_encode_test_request);
DEFINE_NLM_PROG_EVENT_ENCODE(nlm_encode_testres_request);
DEFINE_NLM_PROG_EVENT_ENCODE(nlm_encode_lock_request);
DEFINE_NLM_PROG_EVENT_ENCODE(nlm_encode_canc_request);
DEFINE_NLM_PROG_EVENT_ENCODE(nlm_encode_unlock_request);
DEFINE_NLM_PROG_EVENT_ENCODE(nlm_encode_res_request);

DEFINE_NLM_PROG_EVENT_DECODE(nlm_decode_testres_response);
DEFINE_NLM_PROG_EVENT_DECODE(nlm_decode_test_response);
DEFINE_NLM_PROG_EVENT_DECODE(nlm_decode_res_response);
DEFINE_NLM_PROG_EVENT_DECODE(nlm_decode_norep_response);

#endif /* _TRACE_NLM_XDR_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE nlmxdrtrace
/* This part must be outside protection */
#include <trace/define_trace.h>
