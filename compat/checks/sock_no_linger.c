#include <linux/module.h>
#include <net/sock.h>

void test_wrapper(struct sock *sk)
{
	sock_no_linger(sk);
}

MODULE_LICENSE("GPL");
