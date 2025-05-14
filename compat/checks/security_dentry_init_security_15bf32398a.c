#include <linux/module.h>
#include <linux/types.h>
#include <linux/security.h>

int test_dummy(struct dentry *dentry,
	       int mode, const struct qstr *name, const char **xattr_name,
	       void **ctx, u32 *ctxlen)
{
	security_dentry_init_security(dentry, mode, name, xattr_name, ctx, ctxlen);
	return 0;
}

MODULE_LICENSE("GPL");
