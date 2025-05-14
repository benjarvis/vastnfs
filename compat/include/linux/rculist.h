#ifndef __COMPAT_LINUX_RCULIST_H__
#define __COMPAT_LINUX_RCULIST_H__

#include_next <linux/rculist.h>

#ifndef list_tail_rcu
#define list_tail_rcu(head)	(*((struct list_head __rcu **)(&(head)->prev)))
#endif

#ifndef list_for_each_entry_from_rcu
#define list_for_each_entry_from_rcu(pos, head, member)                        \
       for (; &(pos)->member != (head);                                        \
               pos = list_entry_rcu(pos->member.next, typeof(*(pos)), member))
#endif

#endif
