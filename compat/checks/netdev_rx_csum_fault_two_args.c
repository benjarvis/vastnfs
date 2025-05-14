#include "linux/module.h"
#include "linux/netdevice.h"

void test_wrapper(struct net_device *dev, struct sk_buff *skb)
{
	(void)netdev_rx_csum_fault(dev, skb);
}

MODULE_LICENSE("GPL");
