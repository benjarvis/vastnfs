#ifndef __COMPAT_SEQ_FILE_H__
#define __COMPAT_SEQ_FILE_H__

#include_next <linux/seq_file.h>

#ifndef COMPAT_DETECT_SEQ_ESCAPE_MEM
#include <linux/string_helpers.h>

static inline void seq_escape_mem(struct seq_file *m, const char *src, size_t len,
				  unsigned int flags, const char *esc)
{
       char *buf;
       size_t size = seq_get_buf(m, &buf);
       int ret;

       ret = string_escape_mem(src, len, buf, size, flags, esc);
       seq_commit(m, ret < size ? ret : -1);
}
#endif

#ifndef DEFINE_SHOW_ATTRIBUTE
#define DEFINE_SHOW_ATTRIBUTE(__name)					\
static int __name ## _open(struct inode *inode, struct file *file)	\
{									\
	return single_open(file, __name ## _show, inode->i_private);	\
}									\
									\
static const struct file_operations __name ## _fops = {			\
	.owner		= THIS_MODULE,					\
	.open		= __name ## _open,				\
	.read		= seq_read,					\
	.llseek		= seq_lseek,					\
	.release	= single_release,				\
}

#endif

#endif
