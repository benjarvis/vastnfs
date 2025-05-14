#include "linux/module.h"
#include "linux/tcp.h"

int test_wrapper(struct sock *sk, int val)
{
	tcp_sock_set_user_timeout(sk, val);
	return 0;
}

MODULE_LICENSE("GPL");
