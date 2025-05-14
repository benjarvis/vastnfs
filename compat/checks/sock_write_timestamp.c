#include <linux/module.h>
#include <net/sock.h>

void test_wrapper(struct sock *sk)
{
	sock_write_timestamp(sk, 0);
}

MODULE_LICENSE("GPL");
