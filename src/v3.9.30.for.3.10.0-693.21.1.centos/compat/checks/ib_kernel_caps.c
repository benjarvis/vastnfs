#include <linux/module.h>
#include <rdma/ib_verbs.h>

int check_type(void)
{
	return IBK_SG_GAPS_REG;
}

MODULE_LICENSE("GPL");
