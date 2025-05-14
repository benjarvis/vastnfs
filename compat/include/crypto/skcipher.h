#ifndef __COMPAT_LINUX_CRYPTO_SKCIPHER_H__
#define __COMPAT_LINUX_CRYPTO_SKCIPHER_H__

#include_next <crypto/skcipher.h>

#ifndef COMPAT_DETECT_CRYPTO_ALLOC_SYNC_SKCIPHER
struct crypto_sync_skcipher {
       struct crypto_skcipher base;
};
#define crypto_alloc_sync_skcipher compat_crypto_alloc_sync_skcipher
#define MAX_SYNC_SKCIPHER_REQSIZE      384
/*
 * This performs a type-check against the "tfm" argument to make sure
 * all users have the correct skcipher tfm for doing on-stack requests.
 */
#define SYNC_SKCIPHER_REQUEST_ON_STACK(name, tfm) \
       char __##name##_desc[sizeof(struct skcipher_request) + \
                            MAX_SYNC_SKCIPHER_REQSIZE + \
                            (!(sizeof((struct crypto_sync_skcipher *)1 == \
                                      (typeof(tfm))1))) \
                           ] CRYPTO_MINALIGN_ATTR; \
       struct skcipher_request *name = (void *)__##name##_desc

struct crypto_sync_skcipher *crypto_alloc_sync_skcipher(const char *alg_name,
                                             u32 type, u32 mask);

static inline void crypto_free_sync_skcipher(struct crypto_sync_skcipher *tfm)
{
       crypto_free_skcipher(&tfm->base);
}

static inline unsigned int crypto_sync_skcipher_ivsize(
       struct crypto_sync_skcipher *tfm)
{
       return crypto_skcipher_ivsize(&tfm->base);
}

static inline unsigned int crypto_sync_skcipher_blocksize(
       struct crypto_sync_skcipher *tfm)
{
       return crypto_skcipher_blocksize(&tfm->base);
}

static inline int crypto_sync_skcipher_setkey(struct crypto_sync_skcipher *tfm,
					 const u8 *key, unsigned int keylen)
{
	return crypto_skcipher_setkey(&tfm->base, key, keylen);
}

static inline void skcipher_request_set_sync_tfm(struct skcipher_request *req,
					    struct crypto_sync_skcipher *tfm)
{
	skcipher_request_set_tfm(req, &tfm->base);
}

static inline struct crypto_sync_skcipher *crypto_sync_skcipher_reqtfm(
	struct skcipher_request *req)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(req);

	return container_of(tfm, struct crypto_sync_skcipher, base);
}

#endif

#endif
