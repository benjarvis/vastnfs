#include <linux/module.h>
#include <linux/fs.h>
#include <linux/fs_context.h>

static void nfsd_umount(struct super_block *sb)
{
}

static void nfsd_fs_free_fc(struct fs_context *fc)
{
}

static int nfsd_fs_get_tree(struct fs_context *fc)
{
	return -ENODEV;
}

static const struct fs_context_operations nfsd_fs_context_ops = {
	.free		= nfsd_fs_free_fc,
	.get_tree	= nfsd_fs_get_tree,
};

static int nfsd_init_fs_context(struct fs_context *fc)
{
	fc->ops = &nfsd_fs_context_ops;
	return 0;
}

static struct file_system_type nfsd_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "nfsd",
#ifdef COMPAT_DETECT_INCLUDE_FS_CONTEXT
	.init_fs_context = nfsd_init_fs_context,
#else
	.mount          = compat_emulate_fs_context_mount,
#endif
	.kill_sb	= nfsd_umount,
};
COMPAT_FS_CONTEXT_DETECTOR_IMPL(nfsd_fs_type, nfsd_init_fs_context, NULL);

static int init_nfsd_stub(void)
{
	printk(KERN_INFO "nfsd stub loaded\n");

	return COMPAT_REGISTER_FILESYSTEM(nfsd_fs_type);
}

static void __exit exit_nfsd_stub(void)
{
	COMPAT_UNREGISTER_FILESYSTEM(nfsd_fs_type);
}

module_init(init_nfsd_stub);
module_exit(exit_nfsd_stub);

MODULE_LICENSE("GPL");
