#include <linux/module.h>
#include <crypto/skcipher.h>

void* check_type(void)
{
	return crypto_alloc_sync_skcipher("x", 0, 0);
}

MODULE_LICENSE("GPL");
