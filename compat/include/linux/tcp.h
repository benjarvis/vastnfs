#ifndef __COMPAT_TCP_H__
#define __COMPAT_TCP_H__

#include_next <linux/tcp.h>

static inline int compat_tcp_sock_set_keepidle(struct socket *sock, int val)
{
#ifndef COMPAT_DETECT_TCP_SOCK_SET_KEEPIDLE
	return kernel_setsockopt(sock, SOL_TCP, TCP_KEEPIDLE, (char *)&val, sizeof(val));
#else
	return tcp_sock_set_keepidle(sock->sk, val);
#endif
}

static inline int compat_tcp_sock_set_keepintvl(struct socket *sock, int val)
{
#ifndef COMPAT_DETECT_TCP_SOCK_SET_KEEPINTV
	return kernel_setsockopt(sock, SOL_TCP, TCP_KEEPINTVL, (char *)&val, sizeof(val));
#else
	return tcp_sock_set_keepintvl(sock->sk, val);
#endif
}

static inline int compat_tcp_sock_set_keepcnt(struct socket *sock, int val)
{
#ifndef COMPAT_DETECT_TCP_SOCK_SET_KEEPCNT
	return kernel_setsockopt(sock, SOL_TCP, TCP_KEEPCNT, (char *)&val, sizeof(val));
#else
	return tcp_sock_set_keepcnt(sock->sk, val);
#endif
}

static inline int compat_tcp_sock_set_user_timeout(struct socket *sock, int val)
{
#ifndef COMPAT_DETECT_TCP_SOCK_SET_USER_TIMEOUT
	return kernel_setsockopt(sock, SOL_TCP, TCP_USER_TIMEOUT, (char *)&val, sizeof(val));
#else
	tcp_sock_set_user_timeout(sock->sk, val);
	return 0;
#endif
}

static inline int compat_tcp_sock_set_cork(struct sock *sk, bool val)
{
#ifndef COMPAT_DETECT_TCP_SOCK_SET_CORK
	return 0;
#else
	tcp_sock_set_cork(sk, val);
	return 0;
#endif
}

#ifndef COMPAT_DETECT_TCP_SOCK_SET_NODELAY
extern int compat_tcp_sock_set_nodelay(struct sock *sk);
#else
#define compat_tcp_sock_set_nodelay tcp_sock_set_nodelay
#endif

#endif
