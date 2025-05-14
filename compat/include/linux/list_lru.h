#ifndef __COMPAT_LINUX_LIST_LRU_H__
#define __COMPAT_LINUX_LIST_LRU_H__

#include_next <linux/list_lru.h>

#if defined(COMPAT_DETECT_LIST_LRU_INIT_MEMCG)
#define compat_list_lru_init_memcg list_lru_init_memcg
#else
#define compat_list_lru_init_memcg(lru, sh) list_lru_init_memcg(lru)
#endif

#ifndef COMPAT_DETECT_LIST_LRU_ADD_OBJ
#define compat_list_lru_add list_lru_add
#else
#define compat_list_lru_add list_lru_add_obj
#endif

#ifndef COMPAT_DETECT_LIST_LRU_DEL_OBJ
#define compat_list_lru_del list_lru_del
#else
#define compat_list_lru_del list_lru_del_obj
#endif

#endif
