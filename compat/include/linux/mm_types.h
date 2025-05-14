#ifndef __COMPAT_LINUX_MM_TYPES_H__
#define __COMPAT_LINUX_MM_TYPES_H__

#include_next <linux/mm_types.h>

#if !defined(COMPAT_DETECT_VM_FAULT_T)
typedef __bitwise int vm_fault_t;
#endif

#endif
