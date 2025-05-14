#include "linux/module.h"
#include "linux/tcp.h"

int test_wrapper(struct sock *sk, bool val)
{
	tcp_sock_set_cork(sk, val);
	return 0;
}

MODULE_LICENSE("GPL");
