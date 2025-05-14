#include <linux/module.h>
#include <net/ipv6.h>

int check_type(struct sock *sk, int val)
{
	ip6_sock_set_addr_preferences(sk, val);
	return 0;
}

MODULE_LICENSE("GPL");
