#ifndef __COMPAT_LINUX_DNS_RESOLVER_H__
#define __COMPAT_LINUX_DNS_RESOLVER_H__

#include_next <linux/dns_resolver.h>

#if !defined(COMPAT_DETECT_DNS_QUERY_NET)

static inline
int compat__dns_query(struct net *net,
		      const char *type, const char *name, size_t namelen,
		      const char *options, char **_result, time64_t *_expiry,
		      bool invalidate)
{
#if defined(COMPAT_DETECT_DNS_QUERY_INVALIDATE)
	return dns_query(type, name, namelen, options, _result, _expiry, invalidate);
#else
	return dns_query(type, name, namelen, options, _result, _expiry);
#endif
}

#define dns_query compat__dns_query

#endif

#endif
