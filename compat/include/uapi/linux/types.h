#ifndef __COMPAT_UAPI_LINUX_TYPES_H__
#define __COMPAT_UAPI_LINUX_TYPES_H__

#include_next <uapi/linux/types.h>

#if !defined(COMAPT_DETECT___POLL_T_TYPES)
typedef unsigned __poll_t;
#endif

#endif
