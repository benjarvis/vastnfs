#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>

static int __init init_xprtrdma_dummy(void)
{
	printk(KERN_DEBUG "nfs: RDMA not supported and disabled\n");
	return 0;
}

static void __exit exit_xprtrdma_dummy(void)
{
}

module_init(init_xprtrdma_dummy);
module_exit(exit_xprtrdma_dummy);
