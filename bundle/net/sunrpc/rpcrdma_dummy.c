#include <linux/module.h>

static int __init rpcrdma_init(void)
{
	printk(KERN_DEBUG "rpcrdma: dummy driver loaded\n");
	return 0;
}

static void __exit rpcrdma_cleanup(void)
{
}

module_init(rpcrdma_init);
module_exit(rpcrdma_cleanup);
MODULE_LICENSE("GPL");
