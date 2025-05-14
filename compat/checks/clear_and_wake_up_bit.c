#include <linux/module.h>
#include <linux/wait_bit.h>

void test_dummy(int bit, void *word)
{
	clear_and_wake_up_bit(bit, word);
}

MODULE_LICENSE("GPL");
