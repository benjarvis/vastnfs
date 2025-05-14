#include <linux/module.h>
#include <linux/dns_resolver.h>

int check_type(struct net *net,
	       const char *type, const char *name, size_t namelen,
	       const char *options, char **_result, time64_t *_expiry,
	       bool invalidate)
{
	return dns_query(net, type, name, namelen, options, _result, _expiry, invalidate);
}

MODULE_LICENSE("GPL");
