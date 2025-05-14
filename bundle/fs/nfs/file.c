// SPDX-License-Identifier: GPL-2.0-only
/*
 *  linux/fs/nfs/file.c
 *
 *  Copyright (C) 1992  Rick Sladkey
 *
 *  Changes Copyright (C) 1994 by Florian La Roche
 *   - Do not copy data too often around in the kernel.
 *   - In nfs_file_read the return value of kmalloc wasn't checked.
 *   - Put in a better version of read look-ahead buffering. Original idea
 *     and implementation by Wai S Kok elekokws@ee.nus.sg.
 *
 *  Expire cache on write to a file by Wai S Kok (Oct 1994).
 *
 *  Total rewrite of read side for new NFS buffer cache.. Linus.
 *
 *  nfs regular file handling functions
 */

#include <linux/module.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/stat.h>
#include <linux/nfs_fs.h>
#include <linux/nfs_mount.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/gfp.h>
#include <linux/swap.h>

#include <linux/uaccess.h>

#include "delegation.h"
#include "internal.h"
#include "iostat.h"
#include "fscache.h"
#include "pnfs.h"

#include "nfstrace.h"

#define NFSDBG_FACILITY		NFSDBG_FILE

static const struct vm_operations_struct nfs_file_vm_ops;

/* Hack for future NFS swap support */
#ifndef IS_SWAPFILE
# define IS_SWAPFILE(inode)	(0)
#endif


#if defined(CONFIG_NFS_BUFFERED_INTERNAL_WRITEBACK_ENABLED)
bool nfs_buffered_internal_writeback = true;
#else
bool nfs_buffered_internal_writeback = false;
#endif
module_param_named(buffered_internal_writeback, nfs_buffered_internal_writeback, bool, 0644);
MODULE_PARM_DESC(buffered_internal_writeback, "Large writes are issued immediately with internal writeback");

int nfs_check_flags(int flags)
{
	if ((flags & (O_APPEND | O_DIRECT)) == (O_APPEND | O_DIRECT))
		return -EINVAL;

	return 0;
}
EXPORT_SYMBOL_GPL(nfs_check_flags);

/*
 * Open file
 */
static int
nfs_file_open(struct inode *inode, struct file *filp)
{
	int res;

	dprintk("NFS: open file(%pD2)\n", filp);

	nfs_inc_stats(inode, NFSIOS_VFSOPEN);
	res = nfs_check_flags(filp->f_flags);
	if (res)
		return res;

	res = nfs_open(inode, filp);
	return res;
}

int
nfs_file_release(struct inode *inode, struct file *filp)
{
	dprintk("NFS: release(%pD2)\n", filp);

	nfs_inc_stats(inode, NFSIOS_VFSRELEASE);
	nfs_file_clear_open_context(filp);
	return 0;
}
EXPORT_SYMBOL_GPL(nfs_file_release);

/**
 * nfs_revalidate_file_size - Revalidate the file size
 * @inode: pointer to inode struct
 * @filp: pointer to struct file
 *
 * Revalidates the file length. This is basically a wrapper around
 * nfs_revalidate_inode() that takes into account the fact that we may
 * have cached writes (in which case we don't care about the server's
 * idea of what the file length is), or O_DIRECT (in which case we
 * shouldn't trust the cache).
 */
static int nfs_revalidate_file_size(struct inode *inode, struct file *filp)
{
	struct nfs_server *server = NFS_SERVER(inode);

	if (filp->f_flags & O_DIRECT)
		goto force_reval;
	if (nfs_check_cache_invalid(inode, NFS_INO_INVALID_SIZE))
		goto force_reval;
	return 0;
force_reval:
	return __nfs_revalidate_inode(server, inode);
}

loff_t nfs_file_llseek(struct file *filp, loff_t offset, int whence)
{
	dprintk("NFS: llseek file(%pD2, %lld, %d)\n",
			filp, offset, whence);

	/*
	 * whence == SEEK_END || SEEK_DATA || SEEK_HOLE => we must revalidate
	 * the cached file length
	 */
	if (whence != SEEK_SET && whence != SEEK_CUR) {
		struct inode *inode = filp->f_mapping->host;

		int retval = nfs_revalidate_file_size(inode, filp);
		if (retval < 0)
			return (loff_t)retval;
	}

	return generic_file_llseek(filp, offset, whence);
}
EXPORT_SYMBOL_GPL(nfs_file_llseek);

/*
 * Flush all dirty pages, and check for write errors.
 */
static int
nfs_file_flush(struct file *file, fl_owner_t id)
{
	struct inode	*inode = file_inode(file);
	errseq_t since;

	dprintk("NFS: flush(%pD2)\n", file);

	nfs_inc_stats(inode, NFSIOS_VFSFLUSH);
	if ((file->f_mode & FMODE_WRITE) == 0)
		return 0;

	/* Flush writes to the server and return any errors */
	since = filemap_sample_wb_err(file->f_mapping);
	nfs_wb_all(inode);
	return filemap_check_wb_err(file->f_mapping, since);
}

ssize_t
nfs_file_read(struct kiocb *iocb, struct iov_iter *to)
{
	struct inode *inode = file_inode(iocb->ki_filp);
	ssize_t result;

	if (iocb->ki_flags & IOCB_DIRECT)
		return nfs_file_direct_read(iocb, to, false);

	dprintk("NFS: read(%pD2, %zu@%lu)\n",
		iocb->ki_filp,
		iov_iter_count(to), (unsigned long) iocb->ki_pos);

	nfs_start_io_read(inode);
	result = nfs_revalidate_mapping(inode, iocb->ki_filp->f_mapping);
	if (!result) {
		result = generic_file_read_iter(iocb, to);
		if (result > 0)
			nfs_add_stats(inode, NFSIOS_NORMALREADBYTES, result);
	}
	nfs_end_io_read(inode);
	return result;
}
EXPORT_SYMBOL_GPL(nfs_file_read);

#if !defined(COMPAT_DETECT_GENERIC_FILE_SPLICE_READ_6_5)
ssize_t
nfs_file_splice_read(struct file *in, loff_t *ppos, struct pipe_inode_info *pipe,
		     size_t len, unsigned int flags)
{
	struct inode *inode = file_inode(in);
	ssize_t result;

	dprintk("NFS: splice_read(%pD2, %zu@%llu)\n", in, len, *ppos);

	nfs_start_io_read(inode);
	result = nfs_revalidate_mapping(inode, in->f_mapping);
	if (!result) {
		result = filemap_splice_read(in, ppos, pipe, len, flags);
		if (result > 0)
			nfs_add_stats(inode, NFSIOS_NORMALREADBYTES, result);
	}
	nfs_end_io_read(inode);
	return result;
}
EXPORT_SYMBOL_GPL(nfs_file_splice_read);
#endif

int
nfs_file_mmap(struct file * file, struct vm_area_struct * vma)
{
	struct inode *inode = file_inode(file);
	int	status;

	dprintk("NFS: mmap(%pD2)\n", file);

	/* Note: generic_file_mmap() returns ENOSYS on nommu systems
	 *       so we call that before revalidating the mapping
	 */
	status = generic_file_mmap(file, vma);
	if (!status) {
		vma->vm_ops = &nfs_file_vm_ops;
		status = nfs_revalidate_mapping(inode, file->f_mapping);
	}
	return status;
}
EXPORT_SYMBOL_GPL(nfs_file_mmap);

/*
 * Flush any dirty pages for this process, and check for write errors.
 * The return status from this call provides a reliable indication of
 * whether any write errors occurred for this process.
 */
static int
nfs_file_fsync_commit(struct file *file, int datasync)
{
	struct inode *inode = file_inode(file);
	int ret, ret2;

	dprintk("NFS: fsync file(%pD2) datasync %d\n", file, datasync);

	nfs_inc_stats(inode, NFSIOS_VFSFSYNC);
	ret = nfs_commit_inode(inode, FLUSH_SYNC);
	ret2 = file_check_and_advance_wb_err(file);
	if (ret2 < 0)
		return ret2;
	return ret;
}

int
nfs_file_fsync(struct file *file, loff_t start, loff_t end, int datasync)
{
	struct inode *inode = file_inode(file);
	struct nfs_inode *nfsi = NFS_I(inode);
	long save_nredirtied = atomic_long_read(&nfsi->redirtied_pages);
	long nredirtied;
	int ret;

	trace_nfs_fsync_enter(inode);

	for (;;) {
		ret = file_write_and_wait_range(file, start, end);
		if (ret != 0)
			break;
		ret = nfs_file_fsync_commit(file, datasync);
		if (ret != 0)
			break;
		ret = pnfs_sync_inode(inode, !!datasync);
		if (ret != 0)
			break;
		nredirtied = atomic_long_read(&nfsi->redirtied_pages);
		if (nredirtied == save_nredirtied)
			break;
		save_nredirtied = nredirtied;
	}

	trace_nfs_fsync_exit(inode, ret);
	return ret;
}
EXPORT_SYMBOL_GPL(nfs_file_fsync);

/*
 * Decide whether a read/modify/write cycle may be more efficient
 * then a modify/write/read cycle when writing to a page in the
 * page cache.
 *
 * Some pNFS layout drivers can only read/write at a certain block
 * granularity like all block devices and therefore we must perform
 * read/modify/write whenever a page hasn't read yet and the data
 * to be written there is not aligned to a block boundary and/or
 * smaller than the block size.
 *
 * The modify/write/read cycle may occur if a page is read before
 * being completely filled by the writer.  In this situation, the
 * page must be completely written to stable storage on the server
 * before it can be refilled by reading in the page from the server.
 * This can lead to expensive, small, FILE_SYNC mode writes being
 * done.
 *
 * It may be more efficient to read the page first if the file is
 * open for reading in addition to writing, the page is not marked
 * as Uptodate, it is not dirty or waiting to be committed,
 * indicating that it was previously allocated and then modified,
 * that there were valid bytes of data in that range of the file,
 * and that the new data won't completely replace the old data in
 * that range of the file.
 */
static bool nfs_full_page_write(struct page *page, loff_t pos, unsigned int len)
{
	unsigned int pglen = nfs_page_length(page);
	unsigned int offset = pos & (PAGE_SIZE - 1);
	unsigned int end = offset + len;

	return !pglen || (end >= pglen && !offset);
}

static bool nfs_want_read_modify_write(struct file *file, struct page *page,
			loff_t pos, unsigned int len)
{
	/*
	 * Up-to-date pages, those with ongoing or full-page write
	 * don't need read/modify/write
	 */
	if (PageUptodate(page) || PagePrivate(page) ||
	    nfs_full_page_write(page, pos, len))
		return false;

	if (pnfs_ld_read_whole_page(file->f_mapping->host))
		return true;
	/* Open for reading too? */
	if (file->f_mode & FMODE_READ)
		return true;
	return false;
}

/*
 * This does the "real" work of the write. We must allocate and lock the
 * page to be sent back to the generic routine, which then copies the
 * data from user space.
 *
 * If the writer ends up delaying the write, the writer needs to
 * increment the page use counts until he is done with the page.
 */
#if defined(COMPAT_DETECT_GRAB_CACHE_PAGE_WRITE_BEGIN_5_19)
static int nfs_write_begin(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len,
			struct page **pagep, void **fsdata)
#else
static int nfs_write_begin(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned flags,
			struct page **pagep, void **fsdata)
#endif
{
	int ret;
	pgoff_t index = pos >> PAGE_SHIFT;
	struct page *page;
	int once_thru = 0;

	dfprintk(PAGECACHE, "NFS: write_begin(%pD2(%lu), %u@%lld)\n",
		file, mapping->host->i_ino, len, (long long) pos);

start:
#if defined(COMPAT_DETECT_GRAB_CACHE_PAGE_WRITE_BEGIN_5_19)
	page = grab_cache_page_write_begin(mapping, index);
#else
	page = grab_cache_page_write_begin(mapping, index, flags);
#endif
	if (!page)
		return -ENOMEM;
	*pagep = page;

	ret = nfs_flush_incompatible(file, page);
	if (ret) {
		unlock_page(page);
		put_page(page);
	} else if (!once_thru &&
		   nfs_want_read_modify_write(file, page, pos, len)) {
		once_thru = 1;
#if defined(COMPAT_DETECT_PAGE_FOLIO_5_19)
		ret = nfs_read_folio(file, page_folio(page));
#else
		ret = nfs_readpage(file, page);
#endif
		put_page(page);
		if (!ret)
			goto start;
	}
	return ret;
}

static int nfs_write_end(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned copied,
			struct page *page, void *fsdata)
{
	unsigned offset = pos & (PAGE_SIZE - 1);
	struct nfs_open_context *ctx = nfs_file_open_context(file);
	int status;

	dfprintk(PAGECACHE, "NFS: write_end(%pD2(%lu), %u@%lld)\n",
		file, mapping->host->i_ino, len, (long long) pos);

	/*
	 * Zero any uninitialised parts of the page, and then mark the page
	 * as up to date if it turns out that we're extending the file.
	 */
	if (!PageUptodate(page)) {
		unsigned pglen = nfs_page_length(page);
		unsigned end = offset + copied;

		if (pglen == 0) {
			zero_user_segments(page, 0, offset,
					end, PAGE_SIZE);
			SetPageUptodate(page);
		} else if (end >= pglen) {
			zero_user_segment(page, end, PAGE_SIZE);
			if (offset == 0)
				SetPageUptodate(page);
		} else
			zero_user_segment(page, pglen, PAGE_SIZE);
	}

	status = nfs_updatepage(file, page, offset, copied);

	unlock_page(page);
	put_page(page);

	if (status < 0)
		return status;
	NFS_I(mapping->host)->write_io += copied;

	if (nfs_ctx_key_to_expire(ctx, mapping->host))
		nfs_wb_all(mapping->host);

	return copied;
}

/*
 * Partially or wholly invalidate a page
 * - Release the private state associated with a page if undergoing complete
 *   page invalidation
 * - Called if either PG_private or PG_fscache is set on the page
 * - Caller holds page lock
 */
#if defined(COMPAT_DETECT_PAGE_FOLIO_5_19)
static void nfs_invalidate_folio(struct folio *folio, size_t offset,
                               size_t length)
{
	dfprintk(PAGECACHE, "NFS: invalidate_folio(%lu, %zu, %zu)\n",
		 folio->index, offset, length);

	if (offset != 0 || length < folio_size(folio))
		return;
	/* Cancel any unstarted writes on this page */
	nfs_wb_folio_cancel(folio->mapping->host, folio);
	folio_wait_fscache(folio);
}
#else
static void nfs_invalidate_page(struct page *page, unsigned int offset,
				unsigned int length)
{
	dfprintk(PAGECACHE, "NFS: invalidate_page(%p, %u, %u)\n",
		 page, offset, length);

	if (offset != 0 || length < PAGE_SIZE)
		return;
	/* Cancel any unstarted writes on this page */
	nfs_wb_page_cancel(page_file_mapping(page)->host, page);

	nfs_fscache_invalidate_page(page, page->mapping->host);
}
#endif

/*
 * Attempt to release the private state associated with a page
 * - Called if either PG_private or PG_fscache is set on the page
 * - Caller holds page lock
 * - Return true (may release page) or false (may not)
 */
#if defined(COMPAT_DETECT_PAGE_FOLIO_5_19)
static bool nfs_release_folio(struct folio *folio, gfp_t gfp)
{
       dfprintk(PAGECACHE, "NFS: release_folio(%p)\n", folio);

       /* If the private flag is set, then the folio is not freeable */
       if (folio_test_private(folio))
               return false;
       return nfs_fscache_release_folio(folio, gfp);
}
#else
static int nfs_release_page(struct page *page, gfp_t gfp)
{
	dfprintk(PAGECACHE, "NFS: release_page(%p)\n", page);

	/* If PagePrivate() is set, then the page is not freeable */
	if (PagePrivate(page))
		return 0;
	return nfs_fscache_release_page(page, gfp);
}
#endif

#if defined(COMPAT_DETECT_PAGE_FOLIO_5_19)
static void nfs_check_dirty_writeback(struct folio *folio,
                               bool *dirty, bool *writeback)
{
       struct nfs_inode *nfsi;
       struct address_space *mapping = folio->mapping;

       /*
        * Check if an unstable folio is currently being committed and
        * if so, have the VM treat it as if the folio is under writeback
        * so it will not block due to folios that will shortly be freeable.
        */
       nfsi = NFS_I(mapping->host);
       if (atomic_read(&nfsi->commit_info.rpcs_out)) {
		*writeback = true;
		return;
       }

       /*
        * If the private flag is set, then the folio is not freeable
        * and as the inode is not being committed, it's not going to
        * be cleaned in the near future so treat it as dirty
        */
       if (folio_test_private(folio))
               *dirty = true;
}
#else
static void nfs_check_dirty_writeback(struct page *page,
				bool *dirty, bool *writeback)
{
	struct nfs_inode *nfsi;
	struct address_space *mapping = page_file_mapping(page);

	if (!mapping || PageSwapCache(page))
		return;

	/*
	 * Check if an unstable page is currently being committed and
	 * if so, have the VM treat it as if the page is under writeback
	 * so it will not block due to pages that will shortly be freeable.
	 */
	nfsi = NFS_I(mapping->host);
	if (atomic_read(&nfsi->commit_info.rpcs_out)) {
		*writeback = true;
		return;
	}

	/*
	 * If PagePrivate() is set, then the page is not freeable and as the
	 * inode is not being committed, it's not going to be cleaned in the
	 * near future so treat it as dirty
	 */
	if (PagePrivate(page))
		*dirty = true;
}
#endif

/*
 * Attempt to clear the private state associated with a page when an error
 * occurs that requires the cached contents of an inode to be written back or
 * destroyed
 * - Called if either PG_private or fscache is set on the page
 * - Caller holds page lock
 * - Return 0 if successful, -error otherwise
 */
#if !defined(COMPAT_DETECT_PAGE_FOLIO_5_19)
static int nfs_launder_page(struct page *page)
{
	struct inode *inode = page_file_mapping(page)->host;
	struct nfs_inode *nfsi = NFS_I(inode);

	dfprintk(PAGECACHE, "NFS: launder_page(%ld, %llu)\n",
		inode->i_ino, (long long)page_offset(page));

	nfs_fscache_wait_on_page_write(nfsi, page);
	return nfs_wb_page(inode, page);
}
#else
static int nfs_launder_folio(struct folio *folio)
{
       struct inode *inode = folio->mapping->host;

       dfprintk(PAGECACHE, "NFS: launder_folio(%ld, %llu)\n",
               inode->i_ino, folio_pos(folio));

       folio_wait_fscache(folio);
       return nfs_wb_page(inode, compat_folio_page(folio));
}
#endif

static int nfs_swap_activate(struct swap_info_struct *sis, struct file *file,
						sector_t *span)
{
	unsigned long blocks;
	long long isize;
	struct rpc_clnt *clnt = NFS_CLIENT(file->f_mapping->host);
	struct inode *inode = file->f_mapping->host;

	spin_lock(&inode->i_lock);
	blocks = inode->i_blocks;
	isize = inode->i_size;
	spin_unlock(&inode->i_lock);
	if (blocks*512 < isize) {
		pr_warn("swap activate: swapfile has holes\n");
		return -EINVAL;
	}

	*span = sis->pages;

	return rpc_clnt_swap_activate(clnt);
}

static void nfs_swap_deactivate(struct file *file)
{
	struct rpc_clnt *clnt = NFS_CLIENT(file->f_mapping->host);

	rpc_clnt_swap_deactivate(clnt);
}

const struct address_space_operations nfs_file_aops = {
#if defined(COMPAT_DETECT_PAGE_FOLIO_5_19)
	.read_folio = nfs_read_folio,
#else
	.readpage = nfs_readpage,
#endif
#if defined(COMPAT_DETECT_ADDRESS_SPACE_OPERATIONS_READAHEAD)
	.readahead = nfs_readahead,
#else
	.readpages = nfs_readpages,
#endif
#if defined(COMPAT_DETECT_DIRTY_FOLIO_5_19)
	.dirty_folio = filemap_dirty_folio,
#else
	.set_page_dirty = __set_page_dirty_nobuffers,
#endif
	.writepage = nfs_writepage,
	.writepages = nfs_writepages,
	.write_begin = nfs_write_begin,
	.write_end = nfs_write_end,
#if defined(COMPAT_DETECT_PAGE_FOLIO_5_19)
	.invalidate_folio = nfs_invalidate_folio,
	.release_folio = nfs_release_folio,
#else
	.invalidatepage = nfs_invalidate_page,
	.releasepage = nfs_release_page,
#endif
	.direct_IO = nfs_direct_IO,
#ifdef CONFIG_MIGRATION
#if !defined(COMPAT_DETECT_PAGE_FOLIO_MIGRATE_PAGE_6_0)
	.migratepage = nfs_migrate_page,
#else
	.migrate_folio = nfs_migrate_folio,
#endif
#endif
#if defined(COMPAT_DETECT_PAGE_FOLIO_5_19)
	.launder_folio = nfs_launder_folio,
#else
	.launder_page = nfs_launder_page,
#endif
	.is_dirty_writeback = nfs_check_dirty_writeback,
#ifndef COMPAT_DETECT_ERROR_REMOVE_FOLIO_6_8
	.error_remove_page = generic_error_remove_page,
#else
	.error_remove_folio = generic_error_remove_folio,
#endif
	.swap_activate = nfs_swap_activate,
	.swap_deactivate = nfs_swap_deactivate,
};

/*
 * Notification that a PTE pointing to an NFS page is about to be made
 * writable, implying that someone is about to modify the page through a
 * shared-writable mapping
 */
static vm_fault_t nfs_vm_page_mkwrite(struct vm_fault *vmf)
{
	struct page *page = vmf->page;
	struct file *filp = vmf->vma->vm_file;
	struct inode *inode = file_inode(filp);
	unsigned pagelen;
	vm_fault_t ret = VM_FAULT_NOPAGE;
	struct address_space *mapping;

	dfprintk(PAGECACHE, "NFS: vm_page_mkwrite(%pD2(%lu), offset %lld)\n",
		filp, filp->f_mapping->host->i_ino,
		(long long)page_offset(page));

	sb_start_pagefault(inode->i_sb);

	/* make sure the cache has finished storing the page */
	nfs_fscache_wait_on_page_write(NFS_I(inode), page);

	wait_on_bit_action(&NFS_I(inode)->flags, NFS_INO_INVALIDATING,
			   nfs_wait_bit_killable,
			   TASK_KILLABLE|TASK_FREEZABLE_UNSAFE);

	lock_page(page);
	mapping = page_file_mapping(page);
	if (mapping != inode->i_mapping)
		goto out_unlock;

	wait_on_page_writeback(page);

	pagelen = nfs_page_length(page);
	if (pagelen == 0)
		goto out_unlock;

	ret = VM_FAULT_LOCKED;
	if (nfs_flush_incompatible(filp, page) == 0 &&
	    nfs_updatepage(filp, page, 0, pagelen) == 0)
		goto out;

	ret = VM_FAULT_SIGBUS;
out_unlock:
	unlock_page(page);
out:
	sb_end_pagefault(inode->i_sb);
	return ret;
}

static const struct vm_operations_struct nfs_file_vm_ops = {
	.fault = filemap_fault,
	.map_pages = filemap_map_pages,
	.page_mkwrite = nfs_vm_page_mkwrite,
};

#if !defined(COMPAT_DETECT_FOLIOS_5_16)

static int nfs_alloc_write_buffer(struct page ***arr, unsigned int npages)
{
	unsigned int filled = 0, done = 0;
	int i;

	*arr = kvmalloc_array(npages, sizeof(**arr), GFP_KERNEL_ACCOUNT | __GFP_ZERO);
	if (!*arr)
		return -ENOMEM;

	for (;;) {
		filled = alloc_pages_bulk_array(GFP_KERNEL | __GFP_COMP,
						npages - done,
						*arr + done);
		if (!filled)
			goto err;

		done += filled;
		if (done == npages)
			break;
	}

	return 0;

err:
	for (i = 0; i < npages; i++) {
		if ((*arr)[i])
			__free_page((*arr)[i]);
	}

	kvfree(*arr);
	return -ENOMEM;
}

static void nfs_free_write_buffer(struct page **arr, int npages)
{
	int i;

	for (i = 0; i < npages; i++) {
		if (arr[i])
			put_page(arr[i]);
	}

	kvfree(arr);
}

static void copy_page_kmap_atomic(struct page *page_to, struct page *page_from)
{
	 char *to = kmap_atomic(page_to);
	 char *from = kmap_atomic(page_from);
	 copy_page(to, from);
	 flush_dcache_page(page_to);
	 kunmap_atomic(from);
	 kunmap_atomic(to);
}

/*
 * Copy from user pages to buf pages
 */
static void nfs_async_direct_copy(struct work_struct *w)
{
	struct nfs_async_write *aw = container_of(w, struct nfs_async_write, work);
	size_t offset = aw->page_offset;
	ssize_t bcount = aw->user_count;
	struct iov_iter iov;
	int i;

	if (aw->page_offset)
		BUG_ON(aw->nr_pages + 1 != aw->user_nr_pages);
	else
		BUG_ON(aw->nr_pages != aw->user_nr_pages);

	for (i = 0; i < aw->user_nr_pages; i++) {
		ssize_t len = min_t(ssize_t, PAGE_SIZE - offset, bcount);
		aw->user_vec[i].iov_base =
			page_address(aw->user_pages[i]) + offset;
		aw->user_vec[i].iov_len = len;
		offset = 0;
		bcount -= len;
	}

	compat_iov_iter_kvec(&iov, READ, aw->user_vec, aw->user_nr_pages,
			     aw->user_count);

	aw->count = 0;
	for (i = 0; i < aw->nr_pages; i++) {
		ssize_t len = copy_page_from_iter(
			aw->buf_pages[i], 0, PAGE_SIZE, &iov);
		if (len < 0) {
			aw->count = len;
			break;
		} else {
			aw->count += len;
		}
	}

	for (i = 0; i < aw->user_nr_pages; i++) {
		put_page(aw->user_pages[i]);
		aw->user_pages[i] = NULL;
	}
}

static void nfs_async_direct_write(struct work_struct *w)
{
	struct nfs_async_write *aw =
		container_of(w, struct nfs_async_write, work);
	struct kiocb *iocb = &aw->iocb;
	struct iov_iter *from = &aw->iov;
	struct inode *inode = file_inode(aw->file);
	struct nfs_inode *nfsi = NFS_I(inode);

	aw->count = nfs_file_direct_write(iocb, from, false, true);

	/* Move to head of list */
	spin_lock(&nfsi->async_write_lock);
	if (!list_empty(&aw->node))
		list_move(&aw->node, &nfsi->async_write_list);
	spin_unlock(&nfsi->async_write_lock);
}

static void nfs_async_write_one_flush(struct nfs_async_write *aw)
{
	flush_work(&aw->work);
	nfs_free_write_buffer(aw->buf_pages, aw->nr_pages);
	kvfree(aw->vec);
	kvfree(aw->user_pages);
	kfree(aw);
}

void nfs_async_write_flush(struct nfs_inode *nfsi, int count)
{
	while (atomic_read(&nfsi->async_write_count) > count) {
		struct nfs_async_write *aw;

		spin_lock(&nfsi->async_write_lock);
		aw = list_first_entry_or_null(&nfsi->async_write_list,
						       struct nfs_async_write,
						       node);
		if (aw)
			list_del_init(&aw->node);
		spin_unlock(&nfsi->async_write_lock);

		if (!aw)
			break;

		nfs_async_write_one_flush(aw);
		atomic_dec(&nfsi->async_write_count);
	}
}

/*
 * Similar to how Linux does writeback but manages it internally - copy the
 * written data, put it as clean pages, and send the actual write to the
 * background.
 */
static ssize_t nfs_file_async_write(struct file *file, struct kiocb *iocb,
						 struct iov_iter *from)
{
	struct inode *inode = file_inode(file);
	struct nfs_inode *nfsi = NFS_I(inode);
	struct nfs_async_write *aw;
	size_t total_size = iov_iter_count(from);
	size_t offset;
	struct page **user_pages;
	struct page **buf_pages;
	struct kvec *vec;
	ssize_t count = 0, bytes, bcount;
	int i, nr_pages, user_nr_pages, result;
	loff_t i_size;

	trace_nfs_write_internal_buffer(inode);

	result = generic_write_checks(iocb, from);
	if (result <= 0)
		return result;

	aw = kmalloc(sizeof(*aw), GFP_KERNEL);
	if (!aw)
		return -ENOMEM;

	/* Map the userspace memory */
#if defined(COMPAT_DETECT_IOV_ITER_GET_PAGES_2_6_0)
	bytes = iov_iter_get_pages_alloc2(from, &user_pages, total_size, &offset);
	if (bytes < 0) {
		count = bytes;
		goto out_free;
	}

	iov_iter_revert(from, bytes);
#else
	/* This is a bit more efficient */
	bytes = iov_iter_get_pages_alloc(from, &user_pages, total_size, &offset);
	if (bytes < 0) {
		count = bytes;
		goto out_free;
	}
#endif
	user_nr_pages = (bytes + offset + PAGE_SIZE - 1) / PAGE_SIZE;
	nr_pages = (bytes + PAGE_SIZE - 1) / PAGE_SIZE;

	/* We need two kvec arrays, allocation them at one go */
	vec = kvmalloc_array(nr_pages + user_nr_pages, sizeof(*vec), GFP_KERNEL);
	if (!vec) {
		count = -ENOMEM;
		goto out_put_pages;
	}

	count = nfs_alloc_write_buffer(&buf_pages, nr_pages);
	if (count < 0)
		goto out_free_vec;

	aw->file = file;

	/*
	 * Pages to hold the copy of the written data. Will be added to page
	 * before the write ends.
	 */
	aw->buf_pages = buf_pages;
	aw->nr_pages = nr_pages;
	aw->vec = vec;

	/* kvec for the user pages to copy from (can be non-paged aligned) */
	aw->user_count = bytes;
	aw->user_pages = user_pages;
	aw->user_nr_pages = user_nr_pages;
	aw->user_vec = vec + nr_pages;
	aw->page_offset = offset;

	/* Perform copy in a workqueue */
	aw->count = 0;
	INIT_WORK(&aw->work, nfs_async_direct_copy);
	queue_work(nfs_async_write_workqueue, &aw->work);

	bcount = bytes;
	for (i = 0; i < nr_pages; i++) {
		ssize_t len = min_t(ssize_t, PAGE_SIZE, bcount);

		vec[i].iov_base = page_address(buf_pages[i]);
		vec[i].iov_len = len;
		bcount -= len;
	}
	flush_work(&aw->work);

	if (aw->count < 0) {
		count = aw->count;
		goto out_put_kvfree;
	}

	/* Add the copy to page cache */
	for (i = 0; i < nr_pages; i++) {
		pgoff_t index = (iocb->ki_pos >> PAGE_SHIFT) + i;
		int err;

		struct page *page = buf_pages[i], *old;

retry:
		err = add_to_page_cache_lru(page, inode->i_mapping, index, GFP_KERNEL);
		if (err) {
			/*
			 * Fallback to copy. New files are more interesting in
			 * term of performance.
			 */
			old = find_lock_page(inode->i_mapping, index);
			if (IS_ERR(old) && ERR_PTR(-ENOENT) == old)
				goto retry;
			if (IS_ERR(old))
				continue;

			copy_page_kmap_atomic(old, aw->buf_pages[i]);
			unlock_page(old);
			put_page(old);
			continue;
		}

		SetPageUptodate(page);
		unlock_page(page);
	}

	/* Send the write to the server, in the background */
	kiocb_clone(&aw->iocb, iocb, file);
	compat_iov_iter_kvec(&aw->iov, WRITE, vec, nr_pages, bytes);
	iov_iter_truncate(&aw->iov, total_size);
	iov_iter_advance(from, total_size);
	INIT_WORK(&aw->work, nfs_async_direct_write);

	/* Add to list */
	spin_lock(&nfsi->async_write_lock);
	list_add_tail(&aw->node, &nfsi->async_write_list);
	spin_unlock(&nfsi->async_write_lock);
	atomic_inc(&nfsi->async_write_count);

	queue_work(nfs_async_write_workqueue, &aw->work);

	/* Control queue size */
	nfs_async_write_flush(nfsi, 256);

	iocb->ki_pos += total_size;

	spin_lock(&inode->i_lock);
	i_size = i_size_read(inode);
	if (i_size < iocb->ki_pos) {
		/* This is similar to nfs_grow_file which happens in writeback */
		NFS_I(inode)->cache_validity &= ~NFS_INO_INVALID_SIZE;
		nfs_inc_stats(inode, NFSIOS_EXTENDWRITE);
		i_size_write(inode, iocb->ki_pos);
	}
	spin_unlock(&inode->i_lock);

	return total_size;

out_free_vec:
	kvfree(vec);

out_put_pages:
	for (i = 0; i < user_nr_pages; i++)
		put_page(user_pages[i]);
out_put_kvfree:
	kvfree(user_pages);
out_free:
	kfree(aw);
	return count;
}
#else
void nfs_async_write_flush(struct nfs_inode *nfsi, int count)
{
}
#endif

ssize_t nfs_file_write(struct kiocb *iocb, struct iov_iter *from)
{
	struct file *file = iocb->ki_filp;
	struct inode *inode = file_inode(file);
	unsigned int mntflags = NFS_SERVER(inode)->flags;
	ssize_t result, written;
	errseq_t since;
	int error;
	size_t count;
	bool internal_buffered = false;

	result = nfs_key_timeout_notify(file, inode);
	if (result)
		return result;

	if (iocb->ki_flags & IOCB_DIRECT)
		return nfs_file_direct_write(iocb, from, false, false);

	count = iov_iter_count(from);

	dprintk("NFS: write(%pD2, %zu@%Ld)\n",
		file, iov_iter_count(from), (long long) iocb->ki_pos);

	if (IS_SWAPFILE(inode))
		goto out_swapfile;
	/*
	 * O_APPEND implies that we must revalidate the file length.
	 */
	if (iocb->ki_flags & IOCB_APPEND || iocb->ki_pos > i_size_read(inode)) {
		result = nfs_revalidate_file_size(inode, file);
		if (result)
			return result;
	}

	internal_buffered = (nfs_buffered_internal_writeback &&
			     count > SZ_1M &&
			     (count & ~PAGE_MASK) == 0 &&
			     (iocb->ki_pos & ~PAGE_MASK) == 0);

	if (!internal_buffered) /* Not a read-modify-write, see v5.11-rc6-21-g28aa2f9e73e7 */
		nfs_clear_invalid_mapping(file->f_mapping);

	since = filemap_sample_wb_err(file->f_mapping);
	nfs_start_io_write(inode);
	result = generic_write_checks(iocb, from);
	if (result > 0) {
#if defined(COMPAT_DETECT_CURRENT_BDI_BACKING_DEV_6_5)
		current->backing_dev_info = inode_to_bdi(inode);
#endif
		/*
		 * Large page-aligned and sized writes go to special internally
		 * buffered write to skip pagecache inefficiencies on older
		 * kernels.
		 */
#if !defined(COMPAT_DETECT_FOLIOS_5_16)
		if (internal_buffered) {
			result = nfs_file_async_write(file, iocb, from);
		} else {
#else
		if (false) {
		} else {
#endif
#if !defined(COMPAT_DETECT_GENERIC_PERFORM_WRITE_IOCB)
			result = generic_perform_write(file, from, iocb->ki_pos);
#else
			result = generic_perform_write(iocb, from);
#endif
		}

#if defined(COMPAT_DETECT_CURRENT_BDI_BACKING_DEV_6_5)
		current->backing_dev_info = NULL;
#endif
	}
	nfs_end_io_write(inode);
	if (result <= 0)
		goto out;

	written = result;
#if !defined(COMPAT_DETECT_KIOCB_WRITE_AND_WAIT_DEFINED)
	if (!internal_buffered)
		iocb->ki_pos += written;
#endif
	nfs_add_stats(inode, NFSIOS_NORMALWRITTENBYTES, written);

	if (!internal_buffered) {
		if (mntflags & NFS_MOUNT_WRITE_EAGER) {
			result = filemap_fdatawrite_range(file->f_mapping,
							  iocb->ki_pos - written,
							  iocb->ki_pos - 1);
			if (result < 0)
				goto out;
		}
		if (mntflags & NFS_MOUNT_WRITE_WAIT) {
			result = filemap_fdatawait_range(file->f_mapping,
							 iocb->ki_pos - written,
							 iocb->ki_pos - 1);
			if (result < 0)
				goto out;
		}
	}

	result = generic_write_sync(iocb, written);
	if (result < 0)
		return result;

out:
	/* Return error values */
	error = filemap_check_wb_err(file->f_mapping, since);
	switch (error) {
	default:
		break;
	case -EDQUOT:
	case -EFBIG:
	case -ENOSPC:
		nfs_wb_all(inode);
		error = file_check_and_advance_wb_err(file);
		if (error < 0)
			result = error;
	}
	return result;

out_swapfile:
	printk(KERN_INFO "NFS: attempt to write to active swap file!\n");
	return -ETXTBSY;
}
EXPORT_SYMBOL_GPL(nfs_file_write);

static int
do_getlk(struct file *filp, int cmd, struct file_lock *fl, int is_local)
{
	struct inode *inode = filp->f_mapping->host;
	int status = 0;
	unsigned int saved_type = fl->fl_type;

	/* Try local locking first */
	posix_test_lock(filp, fl);
	if (fl->fl_type != F_UNLCK) {
		/* found a conflict */
		goto out;
	}
	fl->fl_type = saved_type;

	if (NFS_PROTO(inode)->have_delegation(inode, FMODE_READ))
		goto out_noconflict;

	if (is_local)
		goto out_noconflict;

	status = NFS_PROTO(inode)->lock(filp, cmd, fl);
out:
	return status;
out_noconflict:
	fl->fl_type = F_UNLCK;
	goto out;
}

static int
do_unlk(struct file *filp, int cmd, struct file_lock *fl, int is_local)
{
	struct inode *inode = filp->f_mapping->host;
	struct nfs_lock_context *l_ctx;
	unsigned int mntflags = NFS_SERVER(inode)->flags;
	int status;

	/*
	 * Flush all pending writes before doing anything
	 * with locks..
	 */
	nfs_wb_all(inode);

	l_ctx = nfs_get_lock_context(nfs_file_open_context(filp));
	if (!IS_ERR(l_ctx)) {
		status = nfs_iocounter_wait(l_ctx);
		nfs_put_lock_context(l_ctx);
		/*  NOTE: special case
		 * 	If we're signalled while cleaning up locks on process exit, we
		 * 	still need to complete the unlock.
		 */
		if (status < 0 && !(fl->fl_flags & FL_CLOSE))
			return status;
	}

	/*
	 * Use local locking if mounted with "-onolock" or with appropriate
	 * "-olocal_lock="
	 */
	if (!is_local) {
		if (mntflags & NFS_MOUNT_OPT_LOCK_FLUSH) {
			/* Save the time we did unlock */
			unsigned long cache_validity;
			struct nfs_inode *nfsi = NFS_I(inode);

			cache_validity = READ_ONCE(NFS_I(inode)->cache_validity);
			if (!(cache_validity & NFS_INO_INVALID_MTIME))
				nfsi->unlock_mtime = inode_get_mtime(inode);
		}

		status = NFS_PROTO(inode)->lock(filp, cmd, fl);
	} else {
		status = locks_lock_file_wait(filp, fl);
	}

	return status;
}

static int
do_setlk(struct file *filp, int cmd, struct file_lock *fl, int is_local)
{
	struct inode *inode = filp->f_mapping->host;
	unsigned int mntflags = NFS_SERVER(inode)->flags;
	int status;
	bool zap = true;

	/*
	 * Flush all pending writes before doing anything
	 * with locks..
	 */
	status = nfs_sync_mapping(filp->f_mapping);
	if (status != 0)
		goto out;

	/*
	 * Use local locking if mounted with "-onolock" or with appropriate
	 * "-olocal_lock="
	 */
	if (!is_local)
		status = NFS_PROTO(inode)->lock(filp, cmd, fl);
	else
		status = locks_lock_file_wait(filp, fl);
	if (status < 0)
		goto out;

	if (mntflags & NFS_MOUNT_OPT_LOCK_FLUSH && !is_local) {
		struct timespec64 mtime = inode_get_mtime(inode);
		struct nfs_inode *nfsi = NFS_I(inode);

		if (!timespec64_compare(&nfsi->unlock_mtime, &mtime)) {
			int res;

			res = __nfs_revalidate_inode(NFS_SERVER(inode), inode);
			mtime = inode_get_mtime(inode);
			if (!res &&
			    !timespec64_compare(&nfsi->unlock_mtime, &mtime))
			{
				/*
				 * Convinced that the file was not modified
				 * while the lock was held let's retain our
				 * read cache.
				 */
				zap = false;
				trace_nfs_optlockflush_zap_avoided(inode);
			}
		}
	}

	/*
	 * Invalidate cache to prevent missing any changes.  If
	 * the file is mapped, clear the page cache as well so
	 * those mappings will be loaded.
	 *
	 * This makes locking act as a cache coherency point.
	 */
	nfs_sync_mapping(filp->f_mapping);
	if (zap && !NFS_PROTO(inode)->have_delegation(inode, FMODE_READ)) {
		nfs_zap_caches(inode);
		if (mapping_mapped(filp->f_mapping))
			nfs_revalidate_mapping(inode, filp->f_mapping);
	}
out:
	return status;
}

/*
 * Lock a (portion of) a file
 */
int nfs_lock(struct file *filp, int cmd, struct file_lock *fl)
{
	struct inode *inode = filp->f_mapping->host;
	int ret = -ENOLCK;
	int is_local = 0;

	dprintk("NFS: lock(%pD2, t=%x, fl=%x, r=%lld:%lld)\n",
			filp, fl->fl_type, fl->fl_flags,
			(long long)fl->fl_start, (long long)fl->fl_end);

	nfs_inc_stats(inode, NFSIOS_VFSLOCK);

#if defined(COMPAT_DETECT_FL_RECLAIM)
	if (fl->fl_flags & FL_RECLAIM)
		return -ENOGRACE;
#endif

	if (NFS_SERVER(inode)->flags & NFS_MOUNT_LOCAL_FCNTL)
		is_local = 1;

	if (NFS_PROTO(inode)->lock_check_bounds != NULL) {
		ret = NFS_PROTO(inode)->lock_check_bounds(fl);
		if (ret < 0)
			goto out_err;
	}

	if (IS_GETLK(cmd))
		ret = do_getlk(filp, cmd, fl, is_local);
	else if (fl->fl_type == F_UNLCK)
		ret = do_unlk(filp, cmd, fl, is_local);
	else
		ret = do_setlk(filp, cmd, fl, is_local);
out_err:
	return ret;
}
EXPORT_SYMBOL_GPL(nfs_lock);

/*
 * Lock a (portion of) a file
 */
int nfs_flock(struct file *filp, int cmd, struct file_lock *fl)
{
	struct inode *inode = filp->f_mapping->host;
	int is_local = 0;

	dprintk("NFS: flock(%pD2, t=%x, fl=%x)\n",
			filp, fl->fl_type, fl->fl_flags);

	if (!(fl->fl_flags & FL_FLOCK))
		return -ENOLCK;

	/*
	 * The NFSv4 protocol doesn't support LOCK_MAND, which is not part of
	 * any standard. In principle we might be able to support LOCK_MAND
	 * on NFSv2/3 since NLMv3/4 support DOS share modes, but for now the
	 * NFS code is not set up for it.
	 */
	if (fl->fl_type & LOCK_MAND)
		return -EINVAL;

	if (NFS_SERVER(inode)->flags & NFS_MOUNT_LOCAL_FLOCK)
		is_local = 1;

	/* We're simulating flock() locks using posix locks on the server */
	if (fl->fl_type == F_UNLCK)
		return do_unlk(filp, cmd, fl, is_local);
	return do_setlk(filp, cmd, fl, is_local);
}
EXPORT_SYMBOL_GPL(nfs_flock);

const struct file_operations nfs_file_operations = {
	.llseek		= nfs_file_llseek,
	.read_iter	= nfs_file_read,
	.write_iter	= nfs_file_write,
	.mmap		= nfs_file_mmap,
	.open		= nfs_file_open,
	.flush		= nfs_file_flush,
	.release	= nfs_file_release,
	.fsync		= nfs_file_fsync,
	.lock		= nfs_lock,
	.flock		= nfs_flock,
	.splice_read	= nfs_file_splice_read,
	.splice_write	= iter_file_splice_write,
	.check_flags	= nfs_check_flags,
	.setlease	= simple_nosetlease,
};
EXPORT_SYMBOL_GPL(nfs_file_operations);
