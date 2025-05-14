// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2011 Bryan Schumaker <bjschuma@netapp.com>
 * Copyright (c) 2022 Dan Aloni <dan.aloni@vastdata.com>
 *
 * Uses debugfs to create fault injection points for client testing
 */

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/nsproxy.h>
#include <linux/sunrpc/addr.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>

#include "fault_inject.h"
#include "state.h"
#include "netns.h"

static struct dentry *debug_dir;

struct fi_config {
	bool multiple_major_id;
} fi_config;

static DEFINE_MUTEX(major_id_mod);
static char major_id_buf[0x200];

static ssize_t fault_inject_write(struct file *file, const char __user *from,
				  size_t count, loff_t *ppos)
{
	int ret, err;
	unsigned long enable;
	char buf[8] = { 0 };

	ret = simple_write_to_buffer(buf, sizeof(buf) - 1, ppos, from, count);
	err = kstrtoul(buf, 0, &enable);
	if (err)
		return err;

	mutex_lock(&major_id_mod);
	fi_config.multiple_major_id = enable != 0;
	mutex_unlock(&major_id_mod);

	return ret;
}

static ssize_t fault_inject_read(struct file *file, char __user *to,
                                 size_t count, loff_t *ppos)
{
	char buf[128];
	int len, ret;

	len = scnprintf(buf, sizeof(buf), "%d\n", fi_config.multiple_major_id);
	ret = simple_read_from_buffer(to, count, ppos, buf, len);

	return ret;
}

static const struct file_operations fops_multiple_major_ids = {
       .owner   = THIS_MODULE,
       .read    = fault_inject_read,
       .write   = fault_inject_write,
};

void nfsd_fault_inject_init(void)
{
	umode_t mode = S_IFREG | S_IRUSR | S_IWUSR;
	debug_dir = debugfs_create_dir("nfsd", NULL);
	debugfs_create_file("multiple-major-id", mode, debug_dir, NULL,
			    &fops_multiple_major_ids);
}

void nfsd_fault_inject_exit(void)
{
	if (!debug_dir)
		return;

	debugfs_remove_recursive(debug_dir);
	debug_dir = NULL;
}

bool nfsd_fault_injection_major_id_take(struct svc_rqst *rqstp, char **pmajor_id)
{
	struct sockaddr *src_sap = svc_daddr(rqstp);
	char addr_str[INET6_ADDRSTRLEN];
	if (!fi_config.multiple_major_id)
		return false;

	rpc_ntop(src_sap, addr_str, sizeof(addr_str));

	mutex_lock(&major_id_mod);
	snprintf(major_id_buf, sizeof(major_id_buf), "%s [%s]", *pmajor_id,
		addr_str);
	*pmajor_id = &major_id_buf[0];
	return true;
}

void nfsd_fault_injection_major_id_release(bool activated)
{
	if (!activated)
		return;

	mutex_unlock(&major_id_mod);
}
