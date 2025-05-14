#ifndef __COMPAT_LINUX_NET_IPV6_H__
#define __COMPAT_LINUX_NET_IPV6_H__

#include_next <net/ipv6.h>

#ifndef COMPAT_DETECT_IP6_SOCK_SET_RECVPKTINFO
static inline void ip6_sock_set_recvpktinfo(struct sock *sk)
{
	lock_sock(sk);
	inet6_sk(sk)->rxopt.bits.rxinfo = true;
	release_sock(sk);
}
#endif

#ifndef COMPAT_DETECT_IP6_SOCK_SET_V6ONLY
static inline int ip6_sock_set_v6only(struct sock *sk)
{
	if (inet_sk(sk)->inet_num)
		return -EINVAL;
	lock_sock(sk);
	sk->sk_ipv6only = true;
	release_sock(sk);
	return 0;
}
#endif

#ifndef COMPAT_DETECT_IP6_SOCK_SET_ADDR_PREFERENCES
static inline void compat_ip6_sock_set_addr_preferences(struct socket *sock, unsigned int addr_pref)
{
	kernel_setsockopt(sock, SOL_IPV6, IPV6_ADDR_PREFERENCES,
			  (char *)&addr_pref, sizeof(addr_pref));
}
#else
#define compat_ip6_sock_set_addr_preferences(sock, val) ip6_sock_set_addr_preferences(sock->sk, val)
#endif

#endif
