#ifndef __COMPAT_LINUX_KEY_H__
#define __COMPAT_LINUX_KEY_H__

#include_next <linux/key.h>

#ifndef COMPAT_DETECT_REQUEST_KEY_AUTH_AUXDATA_DOMAIN_TAG

struct key_tag;
static inline
struct key *compat__request_key_with_auxdata(struct key_type *type,
					     const char *description,
					     struct key_tag *domain_tag,
					     const void *callout_info,
					     size_t callout_len,
					     void *aux)
{
	return request_key_with_auxdata(type, description, callout_info, callout_len, aux);
}

#define request_key_with_auxdata compat__request_key_with_auxdata

#endif

#endif
