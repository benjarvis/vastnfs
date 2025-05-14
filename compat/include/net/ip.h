#ifndef __COMPAT_LINUX_NET_IP_H__
#define __COMPAT_LINUX_NET_IP_H__

#include_next <net/ip.h>

#ifndef COMPAT_DETECT_IP_SOCK_SET_PKTINFO
static inline void ip_sock_set_pktinfo(struct sock *sk)
{
	lock_sock(sk);
	inet_sk(sk)->cmsg_flags |= IP_CMSG_PKTINFO;
	release_sock(sk);
}
#endif

#endif
