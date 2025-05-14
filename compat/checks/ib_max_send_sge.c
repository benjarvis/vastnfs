#include <linux/module.h>
#include <rdma/ib_verbs.h>

int check_type(struct ib_device_attr *attrs)
{
	return attrs->max_send_sge;
}

MODULE_LICENSE("GPL");
