#ifndef __COMPAT_SLAB_H__
#define __COMPAT_SLAB_H__

#include_next <linux/slab.h>

#ifndef COMPAT_DETECT_KFREE_SENSITIVE
#define kfree_sensitive kfree
#endif

#include <linux/overflow.h>

#endif
