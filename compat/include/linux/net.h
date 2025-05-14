#ifndef __COMPAT_LINUX_NET_H__
#define __COMPAT_LINUX_NET_H__

#include_next <linux/net.h>

#if !defined(COMPAT_DETECT_KERNEL_GETSOCKNAME)

static inline int compat_kernel_getsockname(struct socket *sock, struct sockaddr *addr)
{
	int len = 0;
	int ret = kernel_getsockname(sock, addr, &len);
	if (ret < 0)
		return ret;
	return len;
}

static inline int compat_kernel_getpeername(struct socket *sock, struct sockaddr *addr)
{
	int len = 0;
	int ret = kernel_getpeername(sock, addr, &len);
	if (ret < 0)
		return ret;
	return len;
}

#else

#define compat_kernel_getsockname kernel_getsockname
#define compat_kernel_getpeername kernel_getpeername

#endif

#endif
