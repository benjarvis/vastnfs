#include <linux/module.h>
#include <net/route.h>

/*
 * v6.13-rc1~135^2~105 "ipv4: Prepare ip_route_output() to future .flowi4_tos conversion"
 */
struct rtable *test_wrapper(struct net *net, __be32 daddr, __be32 saddr)
{
	return ip_route_output(net, daddr, saddr, 0, 0, 0);
}

MODULE_LICENSE("GPL");
