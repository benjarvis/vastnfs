#include <linux/module.h>
#include <linux/uio.h>

#define foo(x)                                                    \
  __builtin_choose_expr (                                         \
      __builtin_types_compatible_p (typeof (x), unsigned int),    \
      0,                                                          \
      (void)0)

int test_dummy(struct iov_iter *i)
{
	return foo(i->type);
}

MODULE_LICENSE("GPL");
