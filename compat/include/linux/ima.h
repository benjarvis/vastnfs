#ifndef __COMPAT_LINUX_IMA_H__
#define __COMPAT_LINUX_IMA_H__

#include_next <linux/ima.h>

#ifndef COMPAT_DETECT_IMA_FILE_CHECK_2_ARGS

static inline
int compat__ima_file_check(struct file *file, int mask)
{
	return ima_file_check(file, mask, 0);
}

#define ima_file_check compat__ima_file_check

#endif

#endif
