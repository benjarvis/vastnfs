#include <linux/module.h>
#include <linux/list_lru.h>

/*
 * v6.8-rc1~180^2~241: "list_lru: allow explicit memcg and NUMA node selection"
 */
bool test_dummy(struct list_lru *lru, struct list_head *item)
{
	return list_lru_add_obj(lru, item);
}

MODULE_LICENSE("GPL");
