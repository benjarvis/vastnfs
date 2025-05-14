#include <linux/module.h>
#include <linux/types.h>
#include <linux/security.h>

int test_dummy(struct dentry *dentry,
	       int mode, const struct qstr *name, const char **xattr_name,
	       struct lsmcontext *lsmctx)
{
	/*
	 * Reference in Linux repo:
	 *
	 * git diff Ubuntu-5.19.0-16.16..applied/5.19.0-45.46_22.04.1 --  fs/nfs
	 */
	security_dentry_init_security(dentry, mode, name, xattr_name, lsmctx);
	return 0;
}

MODULE_LICENSE("GPL");
