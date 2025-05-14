#include <linux/module.h>
#include <linux/hashtable.h>

#include <rdma/rdma_cm.h>
#include <rdma/ib_verbs.h>
#include <rdma/restrack.h>

u32 check_type(struct rdma_restrack_entry *rre)
{
	return rre->id;
}

MODULE_LICENSE("GPL");
