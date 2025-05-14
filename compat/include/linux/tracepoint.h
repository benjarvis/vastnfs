#ifndef __COMPAT_TRACEPOINTS_H__
#define __COMPAT_TRACEPOINTS_H__

#include_next <linux/tracepoint.h>

#ifndef DEFINED_COMPAT_STRING_LEN
#define DEFINED_COMPAT_STRING_LEN

#if !defined(COMPAT_DETECT_TRACE_EVENTS__STRING_LEN)
#define COMPAT___STRING_LEN(a, b, c) __dynamic_array(char, b, c + 1)
#else
#define COMPAT___STRING_LEN __string_len
#endif

#if !defined(COMPAT_DETECT_TRACE_EVENTS__STRING_LEN)
#define COMPAT__ASSIGN_STR_LEN(a, b, c) \
	memcpy(__get_str(a), b, c); \
	__get_str(a)[c] = '\0';
#else
#define COMPAT__ASSIGN_STR_LEN __assign_str_len
#endif

#endif

#endif
