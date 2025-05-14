#include <linux/seq_file.h>
#include <linux/module.h>

int test_dummy(struct user_namespace *mnt_userns, struct dentry *dentry,
		 struct iattr *attr)
{
	setattr_prepare(mnt_userns, dentry, attr);
	return 0;
}

MODULE_LICENSE("GPL");
