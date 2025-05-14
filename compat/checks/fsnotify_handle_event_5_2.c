#include "linux/module.h"
#include "linux/fsnotify.h"

#include "../../compat.h"

/* LAYER: 2 */

/* 
 * v5.2-rc1~142^2~3 "fsnotify: switch send_to_group() and ->handle_event to const struct qstr *"
 */
static inline int test_file_fsnotify_handle_event(struct fsnotify_group *group,
						  struct inode *inode,
						  u32 mask, const void *data, int data_type,
						  const struct qstr *file_name, u32 cookie,
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
