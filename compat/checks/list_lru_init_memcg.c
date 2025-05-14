#include <linux/module.h>
#include <linux/list_lru.h>

int check_type(struct list_lru *list, struct shrinker *shrinker)
{
	return list_lru_init_memcg(list, shrinker);
}

MODULE_LICENSE("GPL");
