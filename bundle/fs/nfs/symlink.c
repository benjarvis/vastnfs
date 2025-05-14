// SPDX-License-Identifier: GPL-2.0
/*
 *  linux/fs/nfs/symlink.c
 *
 *  Copyright (C) 1992  Rick Sladkey
 *
 *  Optimization changes Copyright (C) 1994 Florian La Roche
 *
 *  Jun 7 1999, cache symlink lookups in the page cache.  -DaveM
 *
 *  nfs symlink handling code
 */

#include <linux/time.h>
#include <linux/errno.h>
#include <linux/sunrpc/clnt.h>
#include <linux/nfs.h>
#include <linux/nfs2.h>
#include <linux/nfs_fs.h>
#include <linux/pagemap.h>
#include <linux/stat.h>
#include <linux/mm.h>
#include <linux/string.h>

#include "nfstrace.h"

/* Symlink caching in the page cache is even more simplistic
 * and straight-forward than readdir caching.
 */

#if defined(COMPAT_DETECT_PAGE_FOLIO_5_19)
static int nfs_symlink_filler(struct file *file, struct folio *folio)
{
       struct inode *inode = folio->mapping->host;
       struct nfs_fh *fh = NFS_FH(inode);
       int error;

       if (unlikely(file && file->private_data))
		fh = file->private_data;

       error = NFS_PROTO(inode)->readlink(inode, fh, compat_folio_page(folio), 0, PAGE_SIZE);
       if (error < 0)
               goto error;
       folio_mark_uptodate(folio);
       folio_unlock(folio);
       return 0;

error:
       folio_set_error(folio);
       folio_unlock(folio);
       return error;
}
#else

struct inode_and_fh {
	struct inode *inode;
	void *ptr;
};

static int nfs_symlink_filler(void *ptr, struct page *page)
{
	struct inode_and_fh *ctx = ptr;
	struct inode *inode = ctx->inode;
	struct nfs_fh *fh = NFS_FH(inode);
	int error;

	if (unlikely(ctx->ptr))
		fh = ctx->ptr;

	error = NFS_PROTO(inode)->readlink(inode, fh, page, 0, PAGE_SIZE);
	if (error < 0)
		goto error;
	SetPageUptodate(page);
	unlock_page(page);
	return 0;

error:
	SetPageError(page);
	unlock_page(page);
	return error;
}
#endif

static struct page *nfs_readlink_read_cache_page(struct inode *inode, struct dentry *dentry, 
						 void *ptr)
{
#if defined(COMPAT_DETECT_PAGE_FOLIO_5_19)
	struct file fake_file = {
		.private_data = ptr,
	};
	return read_cache_page(&inode->i_data, 0, nfs_symlink_filler, &fake_file);
#else
	struct inode_and_fh ctx = {
		.inode = inode,
		.ptr = ptr,
	};
	return read_cache_page(&inode->i_data, 0, nfs_symlink_filler, &ctx);
#endif
}

static struct page *nfs_readlink_stale_fh_recovery(struct inode *inode, struct dentry *dentry)
{
	struct page *page = NULL;
	int error;

	while (!page) {
		struct nfs_fattr fattr = {};
		struct nfs4_label label = {};
		struct nfs_fh fh_stale_bypass = {};
		struct inode *dir = dentry->d_parent->d_inode;

		/* Perform a lookup to obtain a new symlink file handle */

		trace_nfs_symlink_stale_lookup_enter(dir);
		error = NFS_PROTO(dir)->lookup(dir, dentry, &fh_stale_bypass,
					       &fattr, &label);
		trace_nfs_symlink_stale_lookup_exit(dir, error);

#ifdef CONFIG_NFS_V4_SECURITY_LABEL
		kfree(label.label);
#endif
		nfs4_label_free(fattr.label);

		if (error < 0)
			return ERR_PTR(error);

		/*
		 * Retry with the new result of resolution. If it is not
		 * a new symlink, then READLINK should simply fail and
		 * we abort this loop.
		 */
		page = nfs_readlink_read_cache_page(inode, dentry, &fh_stale_bypass);
		if (IS_ERR(page)) {
			if (signalled())
				return ERR_PTR(-EIO);

			if (PTR_ERR(page) == -ESTALE) {
				page = NULL; /* Retry */
				schedule_timeout_interruptible(1);
			}
		}
	}

	return page;
}

static struct page *nfs_readlink_fill_page(struct inode *inode, struct dentry *dentry)
{
	struct page *page;

	page = nfs_readlink_read_cache_page(inode, dentry, NULL);
	if (PTR_ERR(page) == -ESTALE) {
		trace_nfs_symlink_stale_enter(inode);
		page = nfs_readlink_stale_fh_recovery(inode, dentry);
		trace_nfs_symlink_stale_exit(inode, PTR_ERR(page));
		return page;
	}

	return page;
}

static const char *nfs_get_link(struct dentry *dentry,
				struct inode *inode,
				struct delayed_call *done)
{
	struct page *page;
	void *err;

	if (!dentry) {
		err = ERR_PTR(nfs_revalidate_mapping_rcu(inode));
		if (err)
			return err;
		page = find_get_page(inode->i_mapping, 0);
		if (!page)
			return ERR_PTR(-ECHILD);
		if (!PageUptodate(page)) {
			put_page(page);
			return ERR_PTR(-ECHILD);
		}
	} else {
		err = ERR_PTR(nfs_revalidate_mapping(inode, inode->i_mapping));
		if (err)
			return err;

		page = nfs_readlink_fill_page(inode, dentry);
		if (IS_ERR(page))
			return ERR_CAST(page);
	}
	set_delayed_call(done, page_put_link, page);
	return page_address(page);
}

/*
 * symlinks can't do much...
 */
const struct inode_operations nfs_symlink_inode_operations = {
	.get_link	= nfs_get_link,
	.getattr	= nfs_getattr,
	.setattr	= nfs_setattr,
};
