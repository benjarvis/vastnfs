// SPDX-License-Identifier: GPL-2.0
/*
 * debugfs interface for rpcrdma
 *
 * (c) 2020 Dan Aloni <dan@kernelim.com>
 */

#include <linux/debugfs.h>

#include "nvfs.h"

static struct dentry *topdir;

static int
rpcrdma_info_show(struct seq_file *f, void *v)
{
#ifdef CONFIG_NVFS
	{
		int i;
		seq_printf(f, "nvfs_ops: ");
		for_each_possible_cpu(i) {
			seq_printf(f, "%ld ", per_cpu(nvfs_n_ops, i));
		}
		seq_printf(f, "\n");
	}
#endif
	return 0;
}

static int
rpcrdma_info_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, rpcrdma_info_show, NULL);
}

static int
rpcrdma_info_release(struct inode *inode, struct file *filp)
{
	return single_release(inode, filp);
}

static const struct file_operations rpcrdma_info_fops = {
	.owner		= THIS_MODULE,
	.open		= rpcrdma_info_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= rpcrdma_info_release,
};

void
rpcrdma_debugfs_exit(void)
{
	debugfs_remove_recursive(topdir);
	topdir = NULL;
}

int __init
rpcrdma_debugfs_init(void)
{
	topdir = debugfs_create_dir("rpcrdma", NULL);

	if (!debugfs_create_file("info", S_IFREG | 0400, topdir, NULL,
				 &rpcrdma_info_fops))
		goto err;

	return 0;

err:
	debugfs_remove_recursive(topdir);
	return -ENOMEM;
}
