#include "linux/module.h"
#include "linux/tcp.h"

int test_wrapper(struct sock *sk)
{
	tcp_sock_set_nodelay(sk);
	return 0;
}

MODULE_LICENSE("GPL");
