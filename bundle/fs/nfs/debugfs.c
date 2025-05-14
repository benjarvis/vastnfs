#include <linux/module.h>
#include <linux/string.h>
#include <linux/debugfs.h>
#include <linux/nfs_fs.h>
#include "internal.h"

static struct dentry *topdir;
static struct dentry *nfs_sb_dir;
static struct dentry *nfs_clnt_dir;

static int client_info_show(struct seq_file *f, void *v)
{
	struct nfs_client *clp = f->private;

	seq_printf(f, "state: %d\n", clp->cl_cons_state);
	if (clp->cl_nconnect > 0)
		seq_printf(f, "nconnect: %u\n", clp->cl_nconnect);
	if (clp->cl_hostname && clp->cl_hostname[0])
		seq_printf(f, "hostname: %s\n", clp->cl_hostname);
	if (!IS_ERR_OR_NULL(clp->cl_rpcclient))
		seq_printf(f, "rpc_clnt: %u\n", clp->cl_rpcclient->cl_clid);

	return 0;
}

static int client_info_open(struct inode *inode, struct file *filp)
{
	struct nfs_client *clp = inode->i_private;
	int ret = single_open(filp, client_info_show, clp);

	if (!ret && !refcount_inc_not_zero(&clp->cl_count)) {
		single_release(inode, filp);
		ret = -EINVAL;
	}
	return ret;
}

static int client_info_release(struct inode *inode, struct file *filp)
{
	nfs_put_client(inode->i_private);
	return single_release(inode, filp);
}

static const struct file_operations client_info_fops = {
	.owner   = THIS_MODULE,
	.open    = client_info_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = client_info_release
};

void nfs_client_debugfs_register(struct nfs_client* clp)
{
	char name[32];
	if (!IS_ERR_OR_NULL(nfs_clnt_dir)) {
		snprintf(name, sizeof(name), "%u", clp->cl_trid);
		clp->cl_debugfs = debugfs_create_file(name, S_IFREG|0400,
					nfs_clnt_dir, clp, &client_info_fops);
	}
}

void nfs_client_debugfs_unregister(struct nfs_client* clp)
{
	if (!IS_ERR_OR_NULL(clp->cl_debugfs)) {
		debugfs_remove(clp->cl_debugfs);
		clp->cl_debugfs = NULL;
	}
}

static int server_info_show(struct seq_file *f, void *v)
{
	struct nfs_server *server = f->private;

	if (server && !IS_ERR_OR_NULL(server->debugfs)) {
		struct super_block *sb = server->super;
		if (sb) {
			int i, num = server->superblock.nr_alt_servers;
			seq_printf(f, "sb_id: %s\n", sb->s_id);
			if (!IS_ERR_OR_NULL(server->client))
				seq_printf(f, "rpc_id: %u\n", server->client->cl_clid);
			if (!IS_ERR_OR_NULL(server->client_acl))
				seq_printf(f, "acl_id: %u\n", server->client_acl->cl_clid);
			if (!IS_ERR_OR_NULL(server->nfs_client))
				seq_printf(f, "nfs_id: %u\n", server->nfs_client->cl_trid);
			for (i = 0; i < num; ++i) {
				struct nfs_server* srv = server->superblock.alt_server[i];
				if (srv && srv->trid != server->trid)
					seq_printf(f, "alt_server%d: %u\n", i+1, srv->trid);
			}
		}
	}
	return 0;
}

static int server_info_open(struct inode *inode, struct file *filp)
{
	struct nfs_server *server = inode->i_private;
	struct dentry* dent = NULL;
	int ret = -EINVAL;

	if (server) {
		struct super_block *sb = server->super;
		if (sb)
			dent = dget(sb->s_root);
	}
	if (dent) {
		ret = single_open(filp, server_info_show, server);
		if (ret < 0)
			dput(dent);
	}
	return ret;
}

static int server_info_release(struct inode *inode, struct file *filp)
{
	struct nfs_server *server = inode->i_private;

	dput(server->super->s_root);

	return single_release(inode, filp);
}

static const struct file_operations server_info_fops = {
	.owner   = THIS_MODULE,
	.open    = server_info_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = server_info_release
};

void nfs_server_debugfs_register(struct nfs_server *server)
{
	char name[32];
	if (!IS_ERR_OR_NULL(nfs_sb_dir)) {
		snprintf(name, sizeof(name), "%u", server->trid);
		server->debugfs = debugfs_create_file(name, S_IFREG|0400,
					nfs_sb_dir, server, &server_info_fops);
	}
}

void nfs_server_debugfs_unregister(struct nfs_server *server)
{
	if (!IS_ERR_OR_NULL(server->debugfs)) {
		debugfs_remove(server->debugfs);
		server->debugfs = NULL;
	}
}

void __init vastnfs_debugfs_init(void)
{
	topdir = debugfs_create_dir("vastnfs", NULL);
	if (!IS_ERR_OR_NULL(topdir)) {
		nfs_sb_dir = debugfs_create_dir("nfs_sb", topdir);
		nfs_clnt_dir = debugfs_create_dir("nfs_clnt", topdir);
	}
}

void __exit vastnfs_debugfs_exit(void)
{
	nfs_sb_dir = NULL;
	nfs_clnt_dir = NULL;
	if (!IS_ERR_OR_NULL(topdir)) {
		debugfs_remove_recursive(topdir);
		topdir = NULL;
	}
}
