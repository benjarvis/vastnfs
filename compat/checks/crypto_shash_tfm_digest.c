#include <linux/module.h>
#include <crypto/hash.h>

int check_type(struct crypto_shash *tfm, const u8 *data,
	       unsigned int len, u8 *out)
{
	return crypto_shash_tfm_digest(tfm, data, len, out);
}

MODULE_LICENSE("GPL");
