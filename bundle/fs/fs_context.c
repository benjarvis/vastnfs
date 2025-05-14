// SPDX-License-Identifier: GPL-2.0-or-later
/* Provide a way to create a superblock configuration context within the kernel
 * that allows a superblock to be set up prior to mounting.
 *
 * Copyright (C) 2017 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 */

#undef pr_fmt
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/module.h>
#include <linux/fs_context.h>
#include <linux/fs_parser.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/nsproxy.h>
#include <linux/slab.h>
#include <linux/magic.h>
#include <linux/mnt_namespace.h>
#include <linux/pid_namespace.h>
#include <linux/user_namespace.h>
#include <net/net_namespace.h>
#include <asm/sections.h>

enum legacy_fs_param {
	LEGACY_FS_UNSET_PARAMS,
	LEGACY_FS_MONOLITHIC_PARAMS,
	LEGACY_FS_INDIVIDUAL_PARAMS,
};

struct legacy_fs_context {
	char			*legacy_data;	/* Data page for legacy filesystems */
	size_t			data_size;
	enum legacy_fs_param	param_type;
};

static int legacy_init_fs_context(struct fs_context *fc);

static const struct constant_table common_set_sb_flag[] = {
	{ "dirsync",	SB_DIRSYNC },
	{ "lazytime",	SB_LAZYTIME },
	{ "mand",	SB_MANDLOCK },
	{ "ro",		SB_RDONLY },
	{ "sync",	SB_SYNCHRONOUS },
	{ },
};

static const struct constant_table common_clear_sb_flag[] = {
	{ "async",	SB_SYNCHRONOUS },
	{ "nolazytime",	SB_LAZYTIME },
	{ "nomand",	SB_MANDLOCK },
	{ "rw",		SB_RDONLY },
	{ },
};

/*
 * Check for a common mount option that manipulates s_flags.
 */
static int vfs_parse_sb_flag(struct fs_context *fc, const char *key)
{
	unsigned int token;

	token = lookup_constant(common_set_sb_flag, key, 0);
	if (token) {
		fc->sb_flags |= token;
		fc->sb_flags_mask |= token;
		return 0;
	}

	token = lookup_constant(common_clear_sb_flag, key, 0);
	if (token) {
		fc->sb_flags &= ~token;
		fc->sb_flags_mask |= token;
		return 0;
	}

	return -ENOPARAM;
}

/**
 * vfs_parse_fs_param - Add a single parameter to a superblock config
 * @fc: The filesystem context to modify
 * @param: The parameter
 *
 * A single mount option in string form is applied to the filesystem context
 * being set up.  Certain standard options (for example "ro") are translated
 * into flag bits without going to the filesystem.  The active security module
 * is allowed to observe and poach options.  Any other options are passed over
 * to the filesystem to parse.
 *
 * This may be called multiple times for a context.
 *
 * Returns 0 on success and a negative error code on failure.  In the event of
 * failure, supplementary error information may have been set.
 */
int vfs_parse_fs_param(struct fs_context *fc, struct fs_parameter *param)
{
	int ret;

	if (!param->key)
		return invalf(fc, "Unnamed parameter\n");

	ret = vfs_parse_sb_flag(fc, param->key);
	if (ret != -ENOPARAM)
		return ret;

	if (fc->ops->parse_param) {
		ret = fc->ops->parse_param(fc, param);
		if (ret != -ENOPARAM)
			return ret;
	}

	/* If the filesystem doesn't take any arguments, give it the
	 * default handling of source.
	 */
	if (strcmp(param->key, "source") == 0) {
		if (param->type != fs_value_is_string)
			return invalf(fc, "VFS: Non-string source");
		if (fc->source)
			return invalf(fc, "VFS: Multiple sources");
		fc->source = param->string;
		param->string = NULL;
		return 0;
	}

	return invalf(fc, "%s: Unknown parameter '%s'",
		      fc->fs_type->name, param->key);
}
EXPORT_SYMBOL(vfs_parse_fs_param);

/**
 * vfs_parse_fs_string - Convenience function to just parse a string.
 */
int vfs_parse_fs_string(struct fs_context *fc, const char *key,
			const char *value, size_t v_size)
{
	int ret;

	struct fs_parameter param = {
		.key	= key,
		.type	= fs_value_is_flag,
		.size	= v_size,
	};

	if (value) {
		param.string = kmemdup_nul(value, v_size, GFP_KERNEL);
		if (!param.string)
			return -ENOMEM;
		param.type = fs_value_is_string;
	}

	ret = vfs_parse_fs_param(fc, &param);
	kfree(param.string);
	return ret;
}
EXPORT_SYMBOL(vfs_parse_fs_string);

/**
 * generic_parse_monolithic - Parse key[=val][,key[=val]]* mount data
 * @ctx: The superblock configuration to fill in.
 * @data: The data to parse
 *
 * Parse a blob of data that's in key[=val][,key[=val]]* form.  This can be
 * called from the ->monolithic_mount_data() fs_context operation.
 *
 * Returns 0 on success or the error returned by the ->parse_option() fs_context
 * operation on failure.
 */
int generic_parse_monolithic(struct fs_context *fc, void *data)
{
	char *options = data, *key;
	int ret = 0;

	if (!options)
		return 0;

	while ((key = strsep(&options, ",")) != NULL) {
		if (*key) {
			size_t v_len = 0;
			char *value = strchr(key, '=');

			if (value) {
				if (value == key)
					continue;
				*value++ = 0;
				v_len = strlen(value);
			}
			ret = vfs_parse_fs_string(fc, key, value, v_len);
			if (ret < 0)
				break;
		}
	}

	return ret;
}
EXPORT_SYMBOL(generic_parse_monolithic);

static DEFINE_SPINLOCK(compat_fs_spinlock);
static LIST_HEAD(compat_fs_list);

struct init_fs_context_extern_entry *fs_context_extern_lookup(struct file_system_type *fs_type)
{
	struct init_fs_context_extern_entry *reg;
	struct init_fs_context_extern_entry *found = NULL;

	spin_lock(&compat_fs_spinlock);
	list_for_each_entry(reg, &compat_fs_list, node) {
		if (reg->fs_type == fs_type) {
			found = reg;
			break;
		}
	}
	spin_unlock(&compat_fs_spinlock);

	return found;
}


/**
 * alloc_fs_context - Create a filesystem context.
 * @fs_type: The filesystem type.
 * @reference: The dentry from which this one derives (or NULL)
 * @sb_flags: Filesystem/superblock flags (SB_*)
 * @sb_flags_mask: Applicable members of @sb_flags
 * @purpose: The purpose that this configuration shall be used for.
 *
 * Open a filesystem and create a mount context.  The mount context is
 * initialised with the supplied flags and, if a submount/automount from
 * another superblock (referred to by @reference) is supplied, may have
 * parameters such as namespaces copied across from that superblock.
 */
static struct fs_context *alloc_fs_context(struct file_system_type *fs_type,
				      struct dentry *reference,
				      unsigned int sb_flags,
				      unsigned int sb_flags_mask,
				      enum fs_context_purpose purpose)
{
	struct init_fs_context_extern_entry *entry;
	int (*init_fs_context)(struct fs_context *) = NULL;
	struct fs_context *fc;
	int ret = -ENOMEM;

	fc = kzalloc(sizeof(struct fs_context), GFP_KERNEL);
	if (!fc)
		return ERR_PTR(-ENOMEM);

	fc->purpose	= purpose;
	fc->sb_flags	= sb_flags;
	fc->sb_flags_mask = sb_flags_mask;
	fc->fs_type	= fs_type;
	fc->cred	= get_current_cred();
	fc->net_ns	= get_net(current->nsproxy->net_ns);
	fc->log.prefix	= fs_type->name;

	mutex_init(&fc->uapi_mutex);

	switch (purpose) {
	case FS_CONTEXT_FOR_MOUNT:
		fc->user_ns = get_user_ns(fc->cred->user_ns);
		break;
	case FS_CONTEXT_FOR_SUBMOUNT:
		fc->user_ns = get_user_ns(reference->d_sb->s_user_ns);
		break;
	case FS_CONTEXT_FOR_RECONFIGURE:
		atomic_inc(&reference->d_sb->s_active);
		fc->user_ns = get_user_ns(reference->d_sb->s_user_ns);
		fc->root = dget(reference);
		break;
	}

	/* TODO: Make all filesystems support this unconditionally */
	entry = fs_context_extern_lookup(fs_type);
	if (entry)
		init_fs_context = entry->init_fs_context;
	if (!init_fs_context)
		init_fs_context = legacy_init_fs_context;

	ret = init_fs_context(fc);
	if (ret < 0)
		goto err_fc;
	fc->need_free = true;
	return fc;

err_fc:
	put_fs_context(fc);
	return ERR_PTR(ret);
}

struct fs_context *fs_context_for_mount(struct file_system_type *fs_type,
					unsigned int sb_flags)
{
	return alloc_fs_context(fs_type, NULL, sb_flags, 0,
					FS_CONTEXT_FOR_MOUNT);
}
EXPORT_SYMBOL(fs_context_for_mount);

#if (0)
struct fs_context *fs_context_for_reconfigure(struct dentry *dentry,
					unsigned int sb_flags,
					unsigned int sb_flags_mask)
{
	return alloc_fs_context(dentry->d_sb->s_type, dentry, sb_flags,
				sb_flags_mask, FS_CONTEXT_FOR_RECONFIGURE);
}
EXPORT_SYMBOL(fs_context_for_reconfigure);
#endif

struct fs_context *fs_context_for_submount(struct file_system_type *type,
					   struct dentry *reference)
{
	return alloc_fs_context(type, reference, 0, 0, FS_CONTEXT_FOR_SUBMOUNT);
}
EXPORT_SYMBOL(fs_context_for_submount);

static void legacy_fs_context_free(struct fs_context *fc);

/**
 * vfs_dup_fc_config: Duplicate a filesystem context.
 * @src_fc: The context to copy.
 */
struct fs_context *vfs_dup_fs_context(struct fs_context *src_fc)
{
	struct fs_context *fc;
	int ret;

	if (!src_fc->ops->dup)
		return ERR_PTR(-EOPNOTSUPP);

	fc = kmemdup(src_fc, sizeof(struct fs_context), GFP_KERNEL);
	if (!fc)
		return ERR_PTR(-ENOMEM);

	mutex_init(&fc->uapi_mutex);

	fc->fs_private	= NULL;
	fc->s_fs_info	= NULL;
	fc->source	= NULL;
	get_net(fc->net_ns);
	get_user_ns(fc->user_ns);
	get_cred(fc->cred);
	if (fc->log.log)
		refcount_inc(&fc->log.log->usage);

	/* Can't call put until we've called ->dup */
	ret = fc->ops->dup(fc, src_fc);
	if (ret < 0)
		goto err_fc;

	return fc;

err_fc:
	put_fs_context(fc);
	return ERR_PTR(ret);
}
EXPORT_SYMBOL(vfs_dup_fs_context);

/**
 * logfc - Log a message to a filesystem context
 * @fc: The filesystem context to log to.
 * @fmt: The format of the buffer.
 */
void logfc(struct fc_log *log, const char *prefix, char level, const char *fmt, ...)
{
	va_list va;
	struct va_format vaf = {.fmt = fmt, .va = &va};

	va_start(va, fmt);
	if (!log) {
		switch (level) {
		case 'w':
			printk(KERN_WARNING "%s%s%pV\n", prefix ? prefix : "",
						prefix ? ": " : "", &vaf);
			break;
		case 'e':
			printk(KERN_ERR "%s%s%pV\n", prefix ? prefix : "",
						prefix ? ": " : "", &vaf);
			break;
		default:
			printk(KERN_NOTICE "%s%s%pV\n", prefix ? prefix : "",
						prefix ? ": " : "", &vaf);
			break;
		}
	} else {
		unsigned int logsize = ARRAY_SIZE(log->buffer);
		u8 index;
		char *q = kasprintf(GFP_KERNEL, "%c %s%s%pV\n", level,
						prefix ? prefix : "",
						prefix ? ": " : "", &vaf);

		index = log->head & (logsize - 1);
		BUILD_BUG_ON(sizeof(log->head) != sizeof(u8) ||
			     sizeof(log->tail) != sizeof(u8));
		if ((u8)(log->head - log->tail) == logsize) {
			/* The buffer is full, discard the oldest message */
			if (log->need_free & (1 << index))
				kfree(log->buffer[index]);
			log->tail++;
		}

		log->buffer[index] = q ? q : "OOM: Can't store error string";
		if (q)
			log->need_free |= 1 << index;
		else
			log->need_free &= ~(1 << index);
		log->head++;
	}
	va_end(va);
}
EXPORT_SYMBOL(logfc);

/*
 * Free a logging structure.
 */
static void put_fc_log(struct fs_context *fc)
{
	struct fc_log *log = fc->log.log;
	int i;

	if (log) {
		if (refcount_dec_and_test(&log->usage)) {
			fc->log.log = NULL;
			for (i = 0; i <= 7; i++)
				if (log->need_free & (1 << i))
					kfree(log->buffer[i]);
			kfree(log);
		}
	}
}

/**
 * put_fs_context - Dispose of a superblock configuration context.
 * @fc: The context to dispose of.
 */
void put_fs_context(struct fs_context *fc)
{
	struct super_block *sb;

	if (fc->root) {
		sb = fc->root->d_sb;
		dput(fc->root);
		fc->root = NULL;
		deactivate_super(sb);
	}

	if (fc->need_free && fc->ops && fc->ops->free)
		fc->ops->free(fc);

	put_net(fc->net_ns);
	put_user_ns(fc->user_ns);
	put_cred(fc->cred);
	put_fc_log(fc);
	kfree(fc->source);
	kfree(fc);
}
EXPORT_SYMBOL(put_fs_context);

/*
 * Free the config for a filesystem that doesn't support fs_context.
 */
static void legacy_fs_context_free(struct fs_context *fc)
{
	struct legacy_fs_context *ctx = fc->fs_private;

	if (ctx) {
		if (ctx->param_type == LEGACY_FS_INDIVIDUAL_PARAMS)
			kfree(ctx->legacy_data);
		kfree(ctx);
	}
}

/*
 * Duplicate a legacy config.
 */
static int legacy_fs_context_dup(struct fs_context *fc, struct fs_context *src_fc)
{
	struct legacy_fs_context *ctx;
	struct legacy_fs_context *src_ctx = src_fc->fs_private;

	ctx = kmemdup(src_ctx, sizeof(*src_ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	if (ctx->param_type == LEGACY_FS_INDIVIDUAL_PARAMS) {
		ctx->legacy_data = kmemdup(src_ctx->legacy_data,
					   src_ctx->data_size, GFP_KERNEL);
		if (!ctx->legacy_data) {
			kfree(ctx);
			return -ENOMEM;
		}
	}

	fc->fs_private = ctx;
	return 0;
}

/*
 * Add a parameter to a legacy config.  We build up a comma-separated list of
 * options.
 */
static int legacy_parse_param(struct fs_context *fc, struct fs_parameter *param)
{
	struct legacy_fs_context *ctx = fc->fs_private;
	unsigned int size = ctx->data_size;
	size_t len = 0;

	if (strcmp(param->key, "source") == 0) {
		if (param->type != fs_value_is_string)
			return invalf(fc, "VFS: Legacy: Non-string source");
		if (fc->source)
			return invalf(fc, "VFS: Legacy: Multiple sources");
		fc->source = param->string;
		param->string = NULL;
		return 0;
	}

	if (ctx->param_type == LEGACY_FS_MONOLITHIC_PARAMS)
		return invalf(fc, "VFS: Legacy: Can't mix monolithic and individual options");

	switch (param->type) {
	case fs_value_is_string:
		len = 1 + param->size;
		fallthrough;
	case fs_value_is_flag:
		len += strlen(param->key);
		break;
	default:
		return invalf(fc, "VFS: Legacy: Parameter type for '%s' not supported",
			      param->key);
	}

	if (len > PAGE_SIZE - 2 - size)
		return invalf(fc, "VFS: Legacy: Cumulative options too large");
	if (strchr(param->key, ',') ||
	    (param->type == fs_value_is_string &&
	     memchr(param->string, ',', param->size)))
		return invalf(fc, "VFS: Legacy: Option '%s' contained comma",
			      param->key);
	if (!ctx->legacy_data) {
		ctx->legacy_data = kmalloc(PAGE_SIZE, GFP_KERNEL);
		if (!ctx->legacy_data)
			return -ENOMEM;
	}

	ctx->legacy_data[size++] = ',';
	len = strlen(param->key);
	memcpy(ctx->legacy_data + size, param->key, len);
	size += len;
	if (param->type == fs_value_is_string) {
		ctx->legacy_data[size++] = '=';
		memcpy(ctx->legacy_data + size, param->string, param->size);
		size += param->size;
	}
	ctx->legacy_data[size] = '\0';
	ctx->data_size = size;
	ctx->param_type = LEGACY_FS_INDIVIDUAL_PARAMS;
	return 0;
}

/*
 * Add monolithic mount data.
 */
static int legacy_parse_monolithic(struct fs_context *fc, void *data)
{
	struct legacy_fs_context *ctx = fc->fs_private;

	if (ctx->param_type != LEGACY_FS_UNSET_PARAMS) {
		pr_warn("VFS: Can't mix monolithic and individual options\n");
		return -EINVAL;
	}

	ctx->legacy_data = data;
	ctx->param_type = LEGACY_FS_MONOLITHIC_PARAMS;
	if (!ctx->legacy_data)
		return 0;

	if (fc->fs_type->fs_flags & FS_BINARY_MOUNTDATA)
		return 0;
	return 0;
}

/*
 * Get a mountable root with the legacy mount command.
 */
static int legacy_get_tree(struct fs_context *fc)
{
	struct legacy_fs_context *ctx = fc->fs_private;
	struct super_block *sb;
	struct dentry *root;

	root = fc->fs_type->mount(fc->fs_type, fc->sb_flags,
				      fc->source, ctx->legacy_data);
	if (IS_ERR(root))
		return PTR_ERR(root);

	sb = root->d_sb;
	BUG_ON(!sb);

	fc->root = root;
	return 0;
}

/*
 * Handle remount.
 */
static int legacy_reconfigure(struct fs_context *fc)
{
	struct legacy_fs_context *ctx = fc->fs_private;
	struct super_block *sb = fc->root->d_sb;

	if (!sb->s_op->remount_fs)
		return 0;

	return sb->s_op->remount_fs(sb, &fc->sb_flags,
				    ctx ? ctx->legacy_data : NULL);
}

const struct fs_context_operations legacy_fs_context_ops = {
	.free			= legacy_fs_context_free,
	.dup			= legacy_fs_context_dup,
	.parse_param		= legacy_parse_param,
	.parse_monolithic	= legacy_parse_monolithic,
	.get_tree		= legacy_get_tree,
	.reconfigure		= legacy_reconfigure,
};

/*
 * Initialise a legacy context for a filesystem that doesn't support
 * fs_context.
 */
static int legacy_init_fs_context(struct fs_context *fc)
{
	fc->fs_private = kzalloc(sizeof(struct legacy_fs_context), GFP_KERNEL);
	if (!fc->fs_private)
		return -ENOMEM;
	fc->ops = &legacy_fs_context_ops;
	return 0;
}

int parse_monolithic_mount_data(struct fs_context *fc, void *data)
{
	int (*monolithic_mount_data)(struct fs_context *, void *);

	monolithic_mount_data = fc->ops->parse_monolithic;
	if (!monolithic_mount_data)
		monolithic_mount_data = generic_parse_monolithic;

	return monolithic_mount_data(fc, data);
}

#ifndef COMPAT_DETECT_FS_CONTEXT_SGET_FC

struct sget_fc_context {
	struct fs_context *fc;
	int (*test)(struct super_block *, struct fs_context *);
	int (*set)(struct super_block *, struct fs_context *);
};

static int sget_fc_set(struct super_block *sb, void *data)
{
	struct sget_fc_context *ctx = data;
	int ret;

	sb->s_fs_info = ctx->fc->s_fs_info;
	ret = ctx->set(sb, ctx->fc);
	if (ret) {
		sb->s_fs_info = NULL;
	} else {
		ctx->fc->s_fs_info = NULL;
		sb->s_iflags |= ctx->fc->s_iflags;
	}

	return ret;
}

static int sget_fc_test(struct super_block *sb, void *data)
{
	struct sget_fc_context *ctx = data;

	return ctx->test(sb, ctx->fc);
}

struct super_block *sget_fc(struct fs_context *fc,
			    int (*test)(struct super_block *, struct fs_context *),
			    int (*set)(struct super_block *, struct fs_context *))
{
	struct user_namespace *user_ns = fc->global ? &init_user_ns : fc->user_ns;
	struct sget_fc_context ctx = {
		.fc = fc,
		.test = test,
		.set = set,
	};

	if (!(fc->sb_flags & SB_KERNMOUNT) &&
	    fc->purpose != FS_CONTEXT_FOR_SUBMOUNT) {
		/* Don't allow mounting unless the caller has CAP_SYS_ADMIN
		 * over the namespace.
		 */
		if (!(fc->fs_type->fs_flags & FS_USERNS_MOUNT)) {
			if (!capable(CAP_SYS_ADMIN))
				return ERR_PTR(-EPERM);
		} else {
			if (!ns_capable(fc->user_ns, CAP_SYS_ADMIN))
				return ERR_PTR(-EPERM);
		}
	}

	return sget_userns(fc->fs_type, sget_fc_test, sget_fc_set,
			   fc->sb_flags, user_ns, &ctx);
}
EXPORT_SYMBOL(compat_sget_fc);
#endif

/**
 * vfs_get_tree - Get the mountable root
 * @fc: The superblock configuration context.
 *
 * The filesystem is invoked to get or create a superblock which can then later
 * be used for mounting.  The filesystem places a pointer to the root to be
 * used for mounting in @fc->root.
 */
int vfs_get_tree(struct fs_context *fc)
{
	struct super_block *sb;
	int error;

	if (fc->root)
		return -EBUSY;

	/* Get the mountable root in fc->root, with a ref on the root and a ref
	 * on the superblock.
	 */
	error = fc->ops->get_tree(fc);
	if (error < 0)
		return error;

	if (!fc->root) {
		pr_err("Filesystem %s get_tree() didn't set fc->root\n",
		       fc->fs_type->name);
		/* We don't know what the locking state of the superblock is -
		 * if there is a superblock.
		 */
		BUG();
	}

	sb = fc->root->d_sb;
	WARN_ON(!sb->s_bdi);

	/*
	 * Write barrier is for super_cache_count(). We place it before setting
	 * SB_BORN as the data dependency between the two functions is the
	 * superblock structure contents that we just set up, not the SB_BORN
	 * flag.
	 */
	smp_wmb();
	sb->s_flags |= SB_BORN;

	/*
	 * filesystems should never set s_maxbytes larger than MAX_LFS_FILESIZE
	 * but s_maxbytes was an unsigned long long for many releases. Throw
	 * this warning for a little while to try and catch filesystems that
	 * violate this rule.
	 */
	WARN((sb->s_maxbytes < 0), "%s set sb->s_maxbytes to "
		"negative value (%lld)\n", fc->fs_type->name, sb->s_maxbytes);

	return 0;
}
EXPORT_SYMBOL(vfs_get_tree);

struct vfsmount *fc_mount(struct fs_context *fc)
{
	int err = vfs_get_tree(fc);
	if (!err) {
		up_write(&fc->root->d_sb->s_umount);
		return vfs_create_mount(fc);
	}
	return ERR_PTR(err);
}
EXPORT_SYMBOL(fc_mount);

static int set_anon_super_fc(struct super_block *sb, struct fs_context *fc)
{
	return set_anon_super(sb, NULL);
}

static int test_keyed_super(struct super_block *sb, struct fs_context *fc)
{
	return sb->s_fs_info == fc->s_fs_info;
}

static int test_single_super(struct super_block *s, struct fs_context *fc)
{
	return 1;
}

/**
 * vfs_get_super - Get a superblock with a search key set in s_fs_info.
 * @fc: The filesystem context holding the parameters
 * @keying: How to distinguish superblocks
 * @fill_super: Helper to initialise a new superblock
 *
 * Search for a superblock and create a new one if not found.  The search
 * criterion is controlled by @keying.  If the search fails, a new superblock
 * is created and @fill_super() is called to initialise it.
 *
 * @keying can take one of a number of values:
 *
 * (1) vfs_get_single_super - Only one superblock of this type may exist on the
 *     system.  This is typically used for special system filesystems.
 *
 * (2) vfs_get_keyed_super - Multiple superblocks may exist, but they must have
 *     distinct keys (where the key is in s_fs_info).  Searching for the same
 *     key again will turn up the superblock for that key.
 *
 * (3) vfs_get_independent_super - Multiple superblocks may exist and are
 *     unkeyed.  Each call will get a new superblock.
 *
 * A permissions check is made by sget_fc() unless we're getting a superblock
 * for a kernel-internal mount or a submount.
 */
int vfs_get_super(struct fs_context *fc,
		  enum vfs_get_super_keying keying,
		  int (*fill_super)(struct super_block *sb,
				    struct fs_context *fc))
{
	int (*test)(struct super_block *, struct fs_context *);
	struct super_block *sb;
	int err;

	switch (keying) {
	case vfs_get_single_super:
	case vfs_get_single_reconf_super:
		test = test_single_super;
		break;
	case vfs_get_keyed_super:
		test = test_keyed_super;
		break;
	case vfs_get_independent_super:
		test = NULL;
		break;
	default:
		BUG();
	}

	sb = sget_fc(fc, test, set_anon_super_fc);
	if (IS_ERR(sb))
		return PTR_ERR(sb);

	if (!sb->s_root) {
		err = fill_super(sb, fc);
		if (err)
			goto error;

		sb->s_flags |= SB_ACTIVE;
		fc->root = dget(sb->s_root);
	} else {
		fc->root = dget(sb->s_root);
		if (keying == vfs_get_single_reconf_super) {
			err = -ENODEV;
			dput(fc->root);
			fc->root = NULL;
			goto error;
		}
	}

	return 0;

error:
	deactivate_locked_super(sb);
	return err;
}
EXPORT_SYMBOL(vfs_get_super);

/*
 * Clean up a context after performing an action on it and put it into a state
 * from where it can be used to reconfigure a superblock.
 *
 * Note that here we do only the parts that can't fail; the rest is in
 * finish_clean_context() below and in between those fs_context is marked
 * FS_CONTEXT_AWAITING_RECONF.  The reason for splitup is that after
 * successful mount or remount we need to report success to userland.
 * Trying to do full reinit (for the sake of possible subsequent remount)
 * and failing to allocate memory would've put us into a nasty situation.
 * So here we only discard the old state and reinitialization is left
 * until we actually try to reconfigure.
 */
void vfs_clean_context(struct fs_context *fc)
{
	if (fc->need_free && fc->ops && fc->ops->free)
		fc->ops->free(fc);
	fc->need_free = false;
	fc->fs_private = NULL;
	fc->s_fs_info = NULL;
	fc->sb_flags = 0;
	kfree(fc->source);
	fc->source = NULL;

	fc->purpose = FS_CONTEXT_FOR_RECONFIGURE;
	fc->phase = FS_CONTEXT_AWAITING_RECONF;
}

int finish_clean_context(struct fs_context *fc)
{
	struct init_fs_context_extern_entry *entry;
	int error;

	if (fc->phase != FS_CONTEXT_AWAITING_RECONF)
		return 0;

	entry = fs_context_extern_lookup(fc->fs_type);
	if (entry)
		error = entry->init_fs_context(fc);
	else
		error = legacy_init_fs_context(fc);

	if (unlikely(error)) {
		fc->phase = FS_CONTEXT_FAILED;
		return error;
	}

	fc->need_free = true;
	fc->phase = FS_CONTEXT_RECONF_PARAMS;
	return 0;
}

struct mnt_alloc_wrapper {
	struct file_system_type type;
	struct fs_context *fc;
};

static struct dentry *just_alloc_mount(struct file_system_type *type, int sb_flags,
				       const char *name, void *data)
{
	struct mnt_alloc_wrapper *ctx =
		container_of(type, struct mnt_alloc_wrapper, type);
	return dget(ctx->fc->root);
}

struct vfsmount *compat_vfs_create_mount(struct fs_context *fc)
{
	struct mnt_alloc_wrapper ctx = {
		.type = { .mount = just_alloc_mount },
		.fc = fc,
	};

	/*
	 * Trick vfs_kern_mount to just allocate the mount point.
	 *
	 * For that, we need to mimick its effects on superblock like the
	 * vfs_create_mount.
	 */

	atomic_inc(&fc->root->d_sb->s_active);
	down_write(&fc->root->d_sb->s_umount);

	return vfs_kern_mount(&ctx.type, fc->sb_flags, fc->source ?: "none", NULL);
}
EXPORT_SYMBOL(compat_vfs_create_mount);

struct dentry *compat_emulate_fs_context_mount(struct file_system_type *type,
					       int flags, const char *name, void *data)
{
	struct fs_context *fc;
	struct dentry *root;
	int ret = 0;

	if (!type)
		return ERR_PTR(-EINVAL);

	fc = fs_context_for_mount(type, flags);
	if (IS_ERR(fc))
		return ERR_CAST(fc);

	if (name)
		ret = vfs_parse_fs_string(fc, "source",
					  name, strlen(name));
	if (!ret)
		ret = parse_monolithic_mount_data(fc, data);

	if (!ret) {
		ret = vfs_get_tree(fc);
		if (!ret) {
			root = fc->root;
			fc->root = NULL;
		} else {
			root = ERR_PTR(ret);
		}
	} else {
		root = ERR_PTR(ret);
	}

	put_fs_context(fc);

	return root;
}
EXPORT_SYMBOL(compat_emulate_fs_context_mount);

int compat_register_filesystem(struct file_system_type *fs_type,
			       struct init_fs_context_extern_entry *reg)
{
	spin_lock(&compat_fs_spinlock);
	reg->registered = true;
	list_add_tail(&reg->node, &compat_fs_list);
	spin_unlock(&compat_fs_spinlock);

	return register_filesystem(fs_type);
}
EXPORT_SYMBOL(compat_register_filesystem);

void compat_unregister_filesystem(struct file_system_type *fs_type,
				 struct init_fs_context_extern_entry *reg)
{
	spin_lock(&compat_fs_spinlock);
	if (reg->registered) {
		list_del_init(&reg->node);
		reg->registered = false;
	}
	spin_unlock(&compat_fs_spinlock);

	unregister_filesystem(fs_type);

}
EXPORT_SYMBOL(compat_unregister_filesystem);
