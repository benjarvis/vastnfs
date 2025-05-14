#ifndef __COMPAT_LINUX_NET_UDP_H__
#define __COMPAT_LINUX_NET_UDP_H__

#include_next <net/udp.h>

#if defined(COMPAT_DETECT_SKB_RECV_UDP_5_19)
static inline struct sk_buff *compat_skb_recv_udp(
	struct sock *sk, unsigned int flags,
	int noblock, int *err)
{
	return skb_recv_udp(sk, flags | (noblock ? MSG_DONTWAIT : 0), err);
}
#define skb_recv_udp compat_skb_recv_udp
#endif

#endif
