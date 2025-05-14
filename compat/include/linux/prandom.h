#ifndef __COMPAT_PRANDOM_H__
#define __COMPAT_PRANDOM_H__

#include_next <linux/kernel.h>
#include_next <linux/prandom.h>

#if !defined(COMPAT_DETECT_PRANDOM_U32_EXISTS_PRE_6_1)
static inline u32 prandom_u32(void)
{
       return get_random_u32();
}
#endif

#endif
