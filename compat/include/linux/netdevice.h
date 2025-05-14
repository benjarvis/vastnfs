#ifndef __COMPAT_LINUX_NETDEVICE_H__
#define __COMPAT_LINUX_NETDEVICE_H__

#include_next <linux/netdevice.h>

#ifndef COMPAT_DETECT_NETDEV_RX_CSUM_FAULT_TWO_ARGS

static inline
void compat__netdev_rx_csum_fault(struct net_device *dev, struct sk_buff *skb)
{
	netdev_rx_csum_fault(dev);
}

#define netdev_rx_csum_fault compat__netdev_rx_csum_fault

#endif

#endif
