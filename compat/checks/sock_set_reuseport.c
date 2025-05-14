#include <linux/module.h>
#include <net/sock.h>

void test_wrapper(struct sock *sk)
{
	sock_set_reuseport(sk);
}

MODULE_LICENSE("GPL");
