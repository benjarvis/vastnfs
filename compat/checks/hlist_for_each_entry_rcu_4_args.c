#include <linux/module.h>
#include <linux/rculist.h>

struct compat_check_entry {
        struct hlist_node       list;
        struct rcu_head         rcu_head;
};

void test_wrapper(struct hlist_head *head)
{
	struct compat_check_entry *en;

        hlist_for_each_entry_rcu(en, head, list, rcu_head) {
	}
}

MODULE_LICENSE("GPL");
