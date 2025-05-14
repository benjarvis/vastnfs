// SPDX-License-Identifier: GPL-2.0
/*
 * debugfs interface for sunrpc
 *
 * (c) 2014 Jeff Layton <jlayton@primarydata.com>
 */

#include <linux/debugfs.h>
#include <linux/sunrpc/addr.h>
#include <linux/sunrpc/sched.h>
#include <linux/sunrpc/clnt.h>
#include <linux/sunrpc/metrics.h>
#include <trace/events/sunrpc.h>

#include "netns.h"
#include "fail.h"

static struct dentry *topdir;
static struct dentry *rpc_clnt_dir;
static struct dentry *rpc_xprt_dir;

static int
tasks_show(struct seq_file *f, void *v)
{
	u32 xid = 0;
	struct rpc_task *task = v;
	struct rpc_clnt *clnt = task->tk_client;
	const char *rpc_waitq = "none";

	if (RPC_IS_QUEUED(task))
		rpc_waitq = rpc_qname(task->tk_waitqueue);

	if (task->tk_rqstp)
		xid = be32_to_cpu(task->tk_rqstp->rq_xid);

	seq_printf(f, "%llx %04x %6d 0x%x 0x%x %8ld %ps %sv%u %s a:%ps q:%s\n",
		task->tk_pid, task->tk_flags, task->tk_status,
		clnt->cl_clid, xid, rpc_task_timeout(task), task->tk_ops,
		clnt->cl_program->name, clnt->cl_vers, rpc_proc_name(task),
		task->tk_action, rpc_waitq);

	return 0;
}

static void *
tasks_start(struct seq_file *f, loff_t *ppos)
	__acquires(&clnt->cl_lock)
{
	struct rpc_clnt *clnt = f->private;
	loff_t pos = *ppos;
	struct rpc_task *task;

	spin_lock(&clnt->cl_lock);
	list_for_each_entry(task, &clnt->cl_tasks, tk_task)
		if (pos-- == 0)
			return task;
	return NULL;
}

static void *
tasks_next(struct seq_file *f, void *v, loff_t *pos)
{
	struct rpc_clnt *clnt = f->private;
	struct rpc_task *task = v;
	struct list_head *next = task->tk_task.next;

	++*pos;

	/* If there's another task on list, return it */
	if (next == &clnt->cl_tasks)
		return NULL;
	return list_entry(next, struct rpc_task, tk_task);
}

static void
tasks_stop(struct seq_file *f, void *v)
	__releases(&clnt->cl_lock)
{
	struct rpc_clnt *clnt = f->private;
	spin_unlock(&clnt->cl_lock);
}

static const struct seq_operations tasks_seq_operations = {
	.start	= tasks_start,
	.next	= tasks_next,
	.stop	= tasks_stop,
	.show	= tasks_show,
};

static int tasks_open(struct inode *inode, struct file *filp)
{
	int ret = seq_open(filp, &tasks_seq_operations);
	if (!ret) {
		struct seq_file *seq = filp->private_data;
		struct rpc_clnt *clnt = seq->private = inode->i_private;

		if (!refcount_inc_not_zero(&clnt->cl_count)) {
			seq_release(inode, filp);
			ret = -EINVAL;
		}
	}

	return ret;
}

static int
tasks_release(struct inode *inode, struct file *filp)
{
	struct seq_file *seq = filp->private_data;
	struct rpc_clnt *clnt = seq->private;

	rpc_release_client(clnt);
	return seq_release(inode, filp);
}

static const struct file_operations tasks_fops = {
	.owner		= THIS_MODULE,
	.open		= tasks_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= tasks_release,
};

static int do_xprt_debugfs(struct rpc_clnt *clnt, struct rpc_xprt *xprt, void *numv)
{
	int len;
	char name[24]; /* enough for "../../rpc_xprt/ + 8 hex digits + NULL */
	char link[9]; /* enough for 8 hex digits + NULL */
	int *nump = numv;

	if (IS_ERR_OR_NULL(xprt->debugfs))
		return 0;
	len = snprintf(name, sizeof(name), "../../rpc_xprt/%s",
		       xprt->debugfs->d_name.name);
	if (len >= sizeof(name))
		return -1;
	if (*nump == 0)
		strcpy(link, "xprt");
	else {
		len = snprintf(link, sizeof(link), "xprt%d", *nump);
		if (len >= sizeof(link))
			return -1;
	}
	debugfs_create_symlink(link, clnt->cl_debugfs, name);
	(*nump)++;
	return 0;
}

static int
clnt_stats_show(struct seq_file *f, void *v)
{
	struct rpc_clnt *clnt = f->private;
	rpc_clnt_show_stats(f, clnt, true);
	return 0;
}

static int
clnt_stats_open(struct inode *inode, struct file *filp)
{
	int ret;
	struct rpc_clnt *clnt = inode->i_private;

	ret = single_open(filp, clnt_stats_show, clnt);

	if (!ret && !refcount_inc_not_zero(&clnt->cl_count)) {
		single_release(inode, filp);
		return -EINVAL;
	}

	return ret;
}

static int
clnt_stats_release(struct inode *inode, struct file *filp)
{
	struct seq_file *seq = filp->private_data;
	struct rpc_clnt *clnt = seq->private;

	rpc_release_client(clnt);
	return seq_release(inode, filp);
}

static const struct file_operations clnt_stats_fops = {
	.owner		= THIS_MODULE,
	.open		= clnt_stats_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= clnt_stats_release,
};

static void
port_usage_show(struct seq_file *f, const char *prefix, struct xprt_portusage *portusage)
{
	int i, nr = portusage->nr;
	char buf[64];

	seq_printf(f, "%ss: %d diversion %s\n", prefix, nr,
			(portusage->diversion_enabled ? "enabled" : "disabled"));
	for (i = 0; i < nr; ++i) {
		struct xprt_port *port = portusage->ports + i;
		if (port->len <= 0 || rpc_ntop((struct sockaddr *)&port->addr, buf, sizeof(buf)) <= 0)
			*buf = 0;
		seq_printf(f, "%s%d: %s\n", prefix, i, buf);
	}
}

static int
clnt_extra_show(struct seq_file *f, void *v)
{
	struct rpc_clnt *clnt = f->private;

	if (clnt->cl_parent != clnt)
		seq_printf(f, "\trpc_parent: %u\n", clnt->cl_parent->cl_clid);
	if (!IS_ERR_OR_NULL(clnt->cl_localports_usage))
		port_usage_show(f, "\tlocalport", clnt->cl_localports_usage);
	if (!IS_ERR_OR_NULL(clnt->cl_remoteports_usage))
		port_usage_show(f, "\tremoteport", clnt->cl_remoteports_usage);

	return 0;
}

static int
clnt_extra_open(struct inode *inode, struct file *filp)
{
	struct rpc_clnt *clnt = inode->i_private;
	int ret = single_open(filp, clnt_extra_show, clnt);

	if (!ret && !refcount_inc_not_zero(&clnt->cl_count)) {
		single_release(inode, filp);
		return -EINVAL;
	}
	return ret;
}

static int
clnt_extra_release(struct inode *inode, struct file *filp)
{
	struct seq_file *seq = filp->private_data;
	struct rpc_clnt *clnt = seq->private;

	rpc_release_client(clnt);
	return seq_release(inode, filp);
}

static const struct file_operations clnt_extra_fops = {
	.owner		= THIS_MODULE,
	.open		= clnt_extra_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= clnt_extra_release
};

void
rpc_clnt_debugfs_register(struct rpc_clnt *clnt)
{
	int len;
	char name[9]; /* enough for 8 hex digits + NULL */
	int xprtnum = 0;

	len = snprintf(name, sizeof(name), "%x", clnt->cl_clid);
	if (len >= sizeof(name))
		return;

	/* make the per-client dir */
	clnt->cl_debugfs = debugfs_create_dir(name, rpc_clnt_dir);

	/* make tasks file */
	debugfs_create_file("tasks", S_IFREG | 0400, clnt->cl_debugfs, clnt,
			    &tasks_fops);
	/* make stats file */
	debugfs_create_file("stats", S_IFREG | 0400, clnt->cl_debugfs, clnt,
			    &clnt_stats_fops);

	debugfs_create_file("extra", S_IFREG | 0400, clnt->cl_debugfs,
						clnt, &clnt_extra_fops);

	rpc_clnt_iterate_for_each_xprt(clnt, do_xprt_debugfs, &xprtnum);
}

void
rpc_clnt_debugfs_unregister(struct rpc_clnt *clnt)
{
	debugfs_remove_recursive(clnt->cl_debugfs);
	clnt->cl_debugfs = NULL;
}

static int
xprt_info_show(struct seq_file *f, void *v)
{
	struct rpc_xprt *xprt = f->private;

	seq_printf(f, "netid: %s\n", xprt->address_strings[RPC_DISPLAY_NETID]);
	seq_printf(f, "addr:  %s\n", xprt->address_strings[RPC_DISPLAY_ADDR]);
	seq_printf(f, "port:  %s\n", xprt->address_strings[RPC_DISPLAY_PORT]);
	seq_printf(f, "state: 0x%lx\n", xprt->state);
	return 0;
}

static int
xprt_info_open(struct inode *inode, struct file *filp)
{
	int ret;
	struct rpc_xprt *xprt = inode->i_private;

	ret = single_open(filp, xprt_info_show, xprt);

	if (!ret) {
		if (!xprt_get(xprt)) {
			single_release(inode, filp);
			ret = -EINVAL;
		}
	}
	return ret;
}

static int
xprt_info_release(struct inode *inode, struct file *filp)
{
	struct rpc_xprt *xprt = inode->i_private;

	xprt_put(xprt);
	return single_release(inode, filp);
}

static const struct file_operations xprt_info_fops = {
	.owner		= THIS_MODULE,
	.open		= xprt_info_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= xprt_info_release,
};

static int
xprt_extra_show(struct seq_file *f, void *v)
{
	struct rpc_xprt *xprt = f->private;
	char buf[64];

	seq_printf(f, "id: %d\n", xprt->id);
	seq_printf(f, "remote_port_idx: %d\n", xprt->remote_port_idx);
	seq_printf(f, "reconnection_attempts: %lu\n", xprt->reconnection_attempts);
	if (xprt->pci_devname_address[0])
		seq_printf(f, "pci_device: %s\n", xprt->pci_devname_address);
	if (xprt->localport_len > 0 && rpc_ntop((struct sockaddr *)&xprt->localport, buf, sizeof(buf)) > 0)
		seq_printf(f, "localport: %s\n", buf);
	if (xprt->ops && xprt->ops->print_stats)
		xprt->ops->print_stats(xprt, f);

	return 0;
}

static int
xprt_extra_open(struct inode *inode, struct file *filp)
{
	struct rpc_xprt *xprt = inode->i_private;
	int ret = single_open(filp, xprt_extra_show, xprt);

	if (!ret && !xprt_get(xprt)) {
		single_release(inode, filp);
		ret = -EINVAL;
	}
	return ret;
}

static int
xprt_extra_release(struct inode *inode, struct file *filp)
{
	struct rpc_xprt *xprt = inode->i_private;

	xprt_put(xprt);
	return single_release(inode, filp);
}

static const struct file_operations xprt_extra_fops = {
	.owner		= THIS_MODULE,
	.open		= xprt_extra_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= xprt_extra_release
};

void
rpc_xprt_debugfs_register(struct rpc_xprt *xprt)
{
	int len, id;
	static atomic_t	cur_id;
	char		name[9]; /* 8 hex digits + NULL term */

	id = (unsigned int)atomic_inc_return(&cur_id);

	len = snprintf(name, sizeof(name), "%x", id);
	if (len >= sizeof(name))
		return;

	/* make the per-client dir */
	xprt->debugfs = debugfs_create_dir(name, rpc_xprt_dir);

	/* make tasks file */
	debugfs_create_file("info", S_IFREG | 0400, xprt->debugfs, xprt,
			    &xprt_info_fops);
	/* make vast addition file */
	debugfs_create_file("extra", S_IFREG | 0400, xprt->debugfs,
						xprt, &xprt_extra_fops);
}

void
rpc_xprt_debugfs_unregister(struct rpc_xprt *xprt)
{
	debugfs_remove_recursive(xprt->debugfs);
	xprt->debugfs = NULL;
}

#if IS_ENABLED(CONFIG_FAIL_SUNRPC)
struct fail_sunrpc_attr fail_sunrpc = {
	.attr			= FAULT_ATTR_INITIALIZER,
};
EXPORT_SYMBOL_GPL(fail_sunrpc);

static void fail_sunrpc_init(void)
{
	struct dentry *dir;

	dir = fault_create_debugfs_attr("fail_sunrpc", NULL,
					&fail_sunrpc.attr);

	debugfs_create_bool("ignore-client-disconnect", S_IFREG | 0600, dir,
			    &fail_sunrpc.ignore_client_disconnect);

	debugfs_create_bool("ignore-server-disconnect", S_IFREG | 0600, dir,
			    &fail_sunrpc.ignore_server_disconnect);
}
#else
static void fail_sunrpc_init(void)
{
}
#endif

static void sunrpc_debug_walltime_correlation_cb(struct work_struct *work);
static DECLARE_DELAYED_WORK(walltime_correlation_work,
			    sunrpc_debug_walltime_correlation_cb);

static void sunrpc_debug_walltime_correlation_cb(struct work_struct *work)
{
	unsigned long flags;

	if (!trace_walltime_correlation_enabled())
		return;

	if (!schedule_delayed_work(&walltime_correlation_work, HZ))
		return;

	local_irq_save(flags);
	trace_walltime_correlation(0);
	local_irq_restore(flags);
}

void sunrpc_debug_check_walltime_correlation(void)
{
	sunrpc_debug_walltime_correlation_cb(NULL);
}
EXPORT_SYMBOL_GPL(sunrpc_debug_check_walltime_correlation);

void __exit
sunrpc_debugfs_exit(void)
{
	cancel_delayed_work_sync(&walltime_correlation_work);
	debugfs_remove_recursive(topdir);
	topdir = NULL;
	rpc_clnt_dir = NULL;
	rpc_xprt_dir = NULL;
}

void __init
sunrpc_debugfs_init(void)
{
	topdir = debugfs_create_dir("sunrpc", NULL);

	rpc_clnt_dir = debugfs_create_dir("rpc_clnt", topdir);

	rpc_xprt_dir = debugfs_create_dir("rpc_xprt", topdir);

	fail_sunrpc_init();

	sunrpc_debug_check_walltime_correlation();
}
