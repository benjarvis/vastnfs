#include <linux/module.h>
#include <net/ip.h>

void check_type(struct sock *sk)
{
	ip_sock_set_pktinfo(sk);
}

MODULE_LICENSE("GPL");
