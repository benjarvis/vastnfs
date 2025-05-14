#include <linux/module.h>
#include <net/udp.h>

/*
 * Detect change:
 *
 * commit v5.19-rc1~159^2~338
 * Author: Oliver Hartkopp <socketcan@hartkopp.net>
 * Date:   Mon Apr 11 14:49:55 2022 +0200
 *
 *    net: remove noblock parameter from recvmsg() entities
 */

static inline struct sk_buff *test_wrapper(
	struct sock *sk, unsigned int flags, int *err)
{
	return skb_recv_udp(sk, flags, err);
}

MODULE_LICENSE("GPL");
