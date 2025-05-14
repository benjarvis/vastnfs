#include <linux/module.h>
#include <linux/key.h>

struct key *test_wrapper(struct key_type *type,
			 const char *description,
			 struct key_tag *domain_tag,
			 const void *callout_info,
			 size_t callout_len,
			 void *aux)
{
	return request_key_with_auxdata(type, description, domain_tag, callout_info, callout_len, aux);
}

MODULE_LICENSE("GPL");
