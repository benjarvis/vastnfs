#ifndef __COMPAT_FS_CONTEXT_H__
#define __COMPAT_FS_CONTEXT_H__

#ifdef COMPAT_DETECT_INCLUDE_FS_CONTEXT
#include_next <linux/fs_context.h>

#ifdef COMPAT_DETECT_FS_CONTEXT_LOG_LOG
#define compat_fs_context_to_log(fc) (fc)->log.log
#elif defined(COMPAT_DETECT_FS_CONTEXT_LOG)
#define compat_fs_context_to_log(fc) (fc)->log
#else
#define compat_fs_context_to_log(fc) NULL
#endif

#define COMPAT_FS_CONTEXT_DETECTOR_IMPL(_fs_type, _fs_context_func, _fs_params)
#define COMPAT_FS_CONTEXT_DETECTOR_DECL(_fs_type)

#define COMPAT_REGISTER_FILESYSTEM(name)  register_filesystem(&name)
#define COMPAT_UNREGISTER_FILESYSTEM(name) unregister_filesystem(&name)

#if !defined(COMPAT_DETECT_FS_CONTEXT_GET_TREE_KEYED)
static inline int get_tree_keyed(struct fs_context *fc,
				 int (*fill_super)(struct super_block *sb,
						   struct fs_context *fc),
				 void *key)
{
       fc->s_fs_info = key;
       return vfs_get_super(fc, vfs_get_keyed_super, fill_super);
}
#endif

#else

#include <linux/fs_context_backport.h>
#define compat_fs_context_to_log(fc) (fc)->log.log

struct fs_context;
struct init_fs_context_extern_entry {
	struct list_head node;
	bool registered;
	int (*init_fs_context)(struct fs_context *);
	const struct file_system_type *fs_type;
	const struct fs_parameter_spec *parameters;
};

#define COMPAT_FS_CONTEXT_DETECTOR_IMPL(_fs_type, _fs_context_func, _fs_params) \
	struct init_fs_context_extern_entry _fs_type##_fs_context = { \
		.fs_type = &_fs_type, \
		.init_fs_context = &_fs_context_func, \
		.parameters = _fs_params, \
	}; \

#define COMPAT_FS_CONTEXT_DETECTOR_DECL(_fs_type) \
	extern struct init_fs_context_extern_entry _fs_type##_fs_context;

extern int compat_register_filesystem(struct file_system_type *fs_type,
				      struct init_fs_context_extern_entry *reg);
extern void compat_unregister_filesystem(struct file_system_type *fs_type,
					 struct init_fs_context_extern_entry *reg);

#define COMPAT_REGISTER_FILESYSTEM(name) compat_register_filesystem(&name, &name ##_fs_context)
#define COMPAT_UNREGISTER_FILESYSTEM(name) compat_unregister_filesystem(&name, &name ##_fs_context)

struct dentry *compat_emulate_fs_context_mount(
	struct file_system_type *, int, const char *, void *);

#endif

#ifndef SB_I_STABLE_WRITES
#define SB_I_STABLE_WRITES 0
#endif

#endif
