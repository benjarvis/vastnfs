#include "linux/module.h"
#include "linux/fsnotify.h"

#include "../../compat.h"

/* LAYER: 2 */

/* 
 * v5.11-rc1~87^2~2 "fsnotify: generalize handle_inode_event()"
 */
static inline int test_file_fsnotify_handle_event(struct fsnotify_mark *mark, u32 mask,
						  struct inode *inode, struct inode *dir,
						  const struct qstr *name, u32 cookie)
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
