#include <linux/module.h>
#include <linux/net.h>

int check_type(struct socket *sock, struct sockaddr *addr)
{
	return kernel_getsockname(sock, addr);
}

MODULE_LICENSE("GPL");
