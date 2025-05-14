#include "linux/module.h"
#include "linux/skbuff.h"

__wsum test_wrapper(const struct sk_buff *skb, int offset, u8 *to,
			      int len)
{
	return skb_copy_and_csum_bits(skb, offset, to, len);
}

MODULE_LICENSE("GPL");
