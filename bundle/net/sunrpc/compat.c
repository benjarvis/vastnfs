#include <linux/module.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/dcache.h>
#include <linux/xattr.h>
#include <crypto/hash.h>
#include <crypto/skcipher.h>

#ifndef COMPAT_DETECT_INCLUDE_FS_CONTEXT
#include "../../fs/fs_context.c"
#include "../../fs/fs_parser.c"
#endif

#include "../../fs/proc/proc_sysctl.c"
#include "../../kernel/wait.c"
#include "../../kernel/cred.c"

#ifndef COMPAT_DETECT_TCP_SOCK_SET_NODELAY
#include <net/tcp.h>

int compat_tcp_sock_set_nodelay(struct sock *sk)
{
#ifndef COMPAT_DETECT_TCP_SOCK_SET_NODELAY
	tcp_sk(sk)->nonagle |= TCP_NAGLE_OFF;
	return 0;
#else
	tcp_sock_set_nodelay(sk);
	return 0;
#endif
}
EXPORT_SYMBOL(compat_tcp_sock_set_nodelay);
#endif

#ifndef COMPAT_DETECT_CRYPTO_SHASH_TFM_DIGEST
int crypto_shash_tfm_digest(struct crypto_shash *tfm, const u8 *data,
			    unsigned int len, u8 *out)
{
	SHASH_DESC_ON_STACK(desc, tfm);
	int err;

	desc->tfm = tfm;

	err = crypto_shash_digest(desc, data, len, out);

	shash_desc_zero(desc);

	return err;
}
EXPORT_SYMBOL_GPL(crypto_shash_tfm_digest);
#endif

#ifndef COMPAT_DETECT_CRYPTO_ALLOC_SYNC_SKCIPHER
struct crypto_sync_skcipher *crypto_alloc_sync_skcipher(
				const char *alg_name, u32 type, u32 mask)
{
	printk(KERN_INFO "nfs: skcipher not supported on this kernel");
	return ERR_PTR(-ENODEV); /* TODO */
}
EXPORT_SYMBOL_GPL(crypto_alloc_sync_skcipher);
#endif

#ifndef COMPAT_DETECT_XATTR_SUPPORTS_USER_PREFIX

#ifndef COMPAT_DETECT_XATTR_SUPPORTED_NAMESPACE

#define for_each_xattr_handler(handlers, handler)		\
	if (handlers)						\
		for ((handler) = *(handlers)++;			\
			(handler) != NULL;			\
			(handler) = *(handlers)++)

/*
 * Look for any handler that deals with the specified namespace.
 */
int
xattr_supported_namespace(struct inode *inode, const char *prefix)
{
	const struct xattr_handler **handlers = inode->i_sb->s_xattr;
	const struct xattr_handler *handler;
	size_t preflen;

	if (!(inode->i_opflags & IOP_XATTR)) {
		if (unlikely(is_bad_inode(inode)))
			return -EIO;
		return -EOPNOTSUPP;
	}

	preflen = strlen(prefix);

	for_each_xattr_handler(handlers, handler) {
		if (!strncmp(xattr_prefix(handler), prefix, preflen))
			return 0;
	}

	return -EOPNOTSUPP;
}
#endif

int
xattr_supports_user_prefix(struct inode *inode)
{
	return xattr_supported_namespace(inode, XATTR_USER_PREFIX);
}
EXPORT_SYMBOL(xattr_supports_user_prefix);

#endif

#ifndef COMPAT_DETECT_LOOKUP_POSITIVE_UNLOCKED

#ifndef COMPAT_DETECT_D_FLAGS_NEGATIVE
static inline bool d_flags_negative(unsigned flags)
{
	return (flags & DCACHE_ENTRY_TYPE) == DCACHE_MISS_TYPE;
}
#endif

/*
 * Like lookup_one_len_unlocked(), except that it yields ERR_PTR(-ENOENT)
 * on negatives.  Returns known positive or ERR_PTR(); that's what
 * most of the users want.  Note that pinned negative with unlocked parent
 * _can_ become positive at any time, so callers of lookup_one_len_unlocked()
 * need to be very careful; pinned positives have ->d_inode stable, so
 * this one avoids such problems.
 */
struct dentry *lookup_positive_unlocked(const char *name,
				       struct dentry *base, int len)
{
	struct dentry *ret = lookup_one_len_unlocked(name, base, len);
	if (!IS_ERR(ret) && d_flags_negative(smp_load_acquire(&ret->d_flags))) {
		dput(ret);
		ret = ERR_PTR(-ENOENT);
	}
	return ret;
}
EXPORT_SYMBOL(lookup_positive_unlocked);
#endif
