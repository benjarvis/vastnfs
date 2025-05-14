#ifndef __COMPAT_LINUX_HASHTABLE_H__
#define __COMPAT_LINUX_HASHTABLE_H__

#include_next <linux/hashtable.h>

#ifndef COMPAT_DETECT_HLIST_FOR_EACH_ENTRY_RCU_4_ARGS
#define compat4_hlist_for_each_entry_rcu(a, b, c, d) \
	hlist_for_each_entry_rcu(a, b, c)
#else
#define compat4_hlist_for_each_entry_rcu(a, b, c, d) \
	hlist_for_each_entry_rcu(a, b, c, d)
#endif

#endif
