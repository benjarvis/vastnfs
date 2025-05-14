#ifndef __COMPAT_MODULE_H__
#define __COMPAT_MODULE_H__

#include_next <linux/module.h>

#if defined(COMPAT_DETECT_MODULE_PARAM_CALL_CONST)
#define MODULE_PARAM_CALL_CONST const
#else
#define MODULE_PARAM_CALL_CONST
#endif

#if defined(COMPAT_DETECT_MODULE_PUT_AND_KTHREAD_EXIT_5_17)
#define module_put_and_exit \
       module_put_and_kthread_exit
#endif

#endif
