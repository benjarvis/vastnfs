#include <linux/module.h>
#include <rdma/rdma_cm.h>

int check_type(struct rdma_cm_id *id)
{
	rdma_lock_handler(id);
	return 0;
}

MODULE_LICENSE("GPL");
