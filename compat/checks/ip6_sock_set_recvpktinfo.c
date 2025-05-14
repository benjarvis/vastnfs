#include <linux/module.h>
#include <net/ipv6.h>

void check_type(struct sock *sk)
{
	ip6_sock_set_recvpktinfo(sk);
}

MODULE_LICENSE("GPL");
