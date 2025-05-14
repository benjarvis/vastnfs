#ifndef __COMPAT_LINUX_NET_SOCK_H__
#define __COMPAT_LINUX_NET_SOCK_H__

#include_next <net/sock.h>

#ifndef COMPAT_DETECT_SOCK_SET_REUSEPORT
static inline void sock_set_reuseport(struct sock *sk)
{
	lock_sock(sk);
	sk->sk_reuseport = true;
	release_sock(sk);
}
#endif

#ifndef COMPAT_DETECT_SOCK_SET_KEEPALIVE
static inline void compat_sock_set_keepalive(struct socket *sock)
{
	int keepalive = 1;

	(void)kernel_setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE,
				(char *)&keepalive, sizeof(keepalive));
}
#else
#define compat_sock_set_keepalive(sock) sock_set_keepalive(sock->sk)
#endif

#ifndef COMPAT_DETECT_SOCK_NO_LINGER
static inline void compat_sock_no_linger(struct socket *sock)
{
	struct linger no_linger = {
		.l_onoff = 1,
		.l_linger = 0,
	};

	kernel_setsockopt(sock, SOL_SOCKET, SO_LINGER,
			  (char *)&no_linger, sizeof(no_linger));
}
#else
#define compat_sock_no_linger(sock) sock_no_linger(sock->sk)
#endif


#ifndef COMPAT_DETECT_SOCK_WRITE_TIMESTAMP
static inline void sock_write_timestamp(struct sock *sk, unsigned long timestamp)
{
	sk->sk_stamp = timestamp;
}
#endif

#endif
