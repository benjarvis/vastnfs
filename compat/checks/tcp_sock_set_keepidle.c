#include "linux/module.h"
#include "linux/tcp.h"

int test_wrapper(struct sock *sk, int val)
{
	return tcp_sock_set_keepidle(sk, val);
}

MODULE_LICENSE("GPL");
