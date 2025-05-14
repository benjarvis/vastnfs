#ifndef __COMPAT_LINUX_SKBUFF_H__
#define __COMPAT_LINUX_SKBUFF_H__

#include_next <linux/skbuff.h>

#ifndef COMPAT_DETECT_SKB_COPY_AND_CSUM_BITS_4_ARGS

static inline
__wsum compat__skb_copy_and_csum_bits(const struct sk_buff *skb, int offset, u8 *to,
				      int len)
{
	return skb_copy_and_csum_bits(skb, offset, to, len, 0);
}

#define skb_copy_and_csum_bits compat__skb_copy_and_csum_bits

#endif

#endif
