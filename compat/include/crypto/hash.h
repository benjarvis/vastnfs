#ifndef __COMPAT_LINUX_CRYPTO_HASH_H__
#define __COMPAT_LINUX_CRYPTO_HASH_H__

#include_next <crypto/hash.h>

#ifndef COMPAT_DETECT_CRYPTO_SHASH_TFM_DIGEST
#define crypto_shash_tfm_digest compat_crypto_shash_tfm_digest
extern int crypto_shash_tfm_digest(struct crypto_shash *tfm, const u8 *data,
				   unsigned int len, u8 *out);
#endif

#endif
