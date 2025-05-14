#include "linux/module.h"
#include "linux/fsnotify.h"

#include "../../compat.h"

/* LAYER: 2 */

/*
 * v4.12-rc1~118^2~12 "fsnotify: Pass fsnotify_iter_info into handle_event handler"
 */
static inline int test_file_fsnotify_handle_event(struct fsnotify_group *group,
						  struct inode *inode,
						  struct fsnotify_mark *inode_mark,
						  struct fsnotify_mark *vfsmount_mark,
						  u32 mask, const void *data, int data_type,
						  const unsigned char *file_name, u32 cookie,
						  struct fsnotify_iter_info *iter_info)
{
	return 0;
}

static const struct fsnotify_ops file_fsnotify_ops = {
#ifdef COMPAT_DETECT_HANDLE_INODE_EVENT
	.handle_inode_event = test_file_fsnotify_handle_event,
#else
	.handle_event = test_file_fsnotify_handle_event,
#endif
};

const void* test_wrapper(void)
{
	return &file_fsnotify_ops;
}

MODULE_LICENSE("GPL");
