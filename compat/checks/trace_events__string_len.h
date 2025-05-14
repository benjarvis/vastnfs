#undef TRACE_SYSTEM
#define TRACE_SYSTEM compat_test_trace

#if !defined(_COMPAT_TRACE_TEST_H) || defined(TRACE_HEADER_MULTI_READ)
#define _COMPAT_TRACE_TEST_H

#include <linux/tracepoint.h>


TRACE_EVENT(test_event,
	TP_PROTO(const char *name,
		 int namlen),
	TP_ARGS(name, namlen),
	TP_STRUCT__entry(
		__string_len(name, name, namlen)
	),
	TP_fast_assign(
		__assign_str_len(name, name, namlen)
	),
	TP_printk("name=%s", __get_str(name)
	)
)

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE trace_events__string_len
#include <trace/define_trace.h>

#endif
