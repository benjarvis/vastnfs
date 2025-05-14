// SPDX-License-Identifier: GPL-2.0
/*
 * Multipath support for RPC
 *
 * Copyright (c) 2015, 2016, Primary Data, Inc. All rights reserved.
 *
 * Trond Myklebust <trond.myklebust@primarydata.com>
 *
 */
#include <linux/types.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/rcupdate.h>
#include <linux/rculist.h>
#include <linux/slab.h>
#include <asm/cmpxchg.h>
#include <linux/spinlock.h>
#include <linux/sunrpc/clnt.h>
#include <linux/sunrpc/xprt.h>
#include <linux/sunrpc/kref.h>
#include <linux/sunrpc/addr.h>
#include <linux/sunrpc/xprtmultipath.h>

typedef struct rpc_xprt *(*xprt_switch_find_xprt_t)(struct rpc_xprt_switch *xps,
		const struct rpc_xprt *cur, struct rpc_xprt_iter_conf conf);

static const struct rpc_xprt_iter_ops rpc_xprt_iter_singular;
static const struct rpc_xprt_iter_ops rpc_xprt_iter_roundrobin;
static const struct rpc_xprt_iter_ops rpc_xprt_iter_listall;

static void xprt_switch_add_xprt_locked(struct rpc_xprt_switch *xps,
		struct rpc_xprt *xprt, struct rpc_xids **old_xids)
{
	struct rpc_xids	*xids;

	*old_xids = NULL;

	if (unlikely(xprt_get(xprt) == NULL))
		return;
	xids = xid_space_get(xps->xps_xid_space);
	*old_xids = xchg(&xprt->xid_space, xids);

	list_add_tail_rcu(&xprt->xprt_switch, &xps->xps_xprt_list);
	list_add_tail_rcu(&xprt->remote_port_xprt_switch,
			  &xps->xps_remoteport_xprt_list[xprt->remote_port_idx]);
	smp_wmb();
	if (xps->xps_nxprts == 0)
		xps->xps_net = xprt->xprt_net;
	xps->xps_nxprts++;
	xps->xps_nactive++;
}

/**
 * rpc_xprt_switch_add_xprt - Add a new rpc_xprt to an rpc_xprt_switch
 * @xps: pointer to struct rpc_xprt_switch
 * @xprt: pointer to struct rpc_xprt
 *
 * Adds xprt to the end of the list of struct rpc_xprt in xps.
 */
void rpc_xprt_switch_add_xprt(struct rpc_xprt_switch *xps,
		struct rpc_xprt *xprt)
{
	struct rpc_xids	*xids = NULL;
	if (xprt == NULL)
		return;
	spin_lock(&xps->xps_lock);
	if (xps->xps_net == xprt->xprt_net || xps->xps_net == NULL)
		xprt_switch_add_xprt_locked(xps, xprt, &xids);
	spin_unlock(&xps->xps_lock);

	xid_space_put(xids);
}

static void xprt_switch_remove_xprt_locked(struct rpc_xprt_switch *xps,
		struct rpc_xprt *xprt)
{
	if (unlikely(xprt == NULL))
		return;
	xps->xps_nactive--;
	xps->xps_nxprts--;
	if (xps->xps_nxprts == 0)
		xps->xps_net = NULL;
	smp_wmb();
	list_del_rcu(&xprt->xprt_switch);
	list_del_rcu(&xprt->remote_port_xprt_switch);
}

/**
 * rpc_xprt_switch_remove_xprt - Removes an rpc_xprt from a rpc_xprt_switch
 * @xps: pointer to struct rpc_xprt_switch
 * @xprt: pointer to struct rpc_xprt
 *
 * Removes xprt from the list of struct rpc_xprt in xps.
 */
void rpc_xprt_switch_remove_xprt(struct rpc_xprt_switch *xps,
		struct rpc_xprt *xprt)
{
	spin_lock(&xps->xps_lock);
	xprt_switch_remove_xprt_locked(xps, xprt);
	spin_unlock(&xps->xps_lock);
	xprt_put(xprt);
}

/**
 * xprt_switch_alloc - Allocate a new struct rpc_xprt_switch
 * @xprt: pointer to struct rpc_xprt
 * @gfp_flags: allocation flags
 *
 * On success, returns an initialised struct rpc_xprt_switch, containing
 * the entry xprt. Returns NULL on failure.
 */
struct rpc_xprt_switch *xprt_switch_alloc(struct rpc_xprt *xprt,
		gfp_t gfp_flags)
{
	struct rpc_xprt_switch *xps;
	struct rpc_xids	*xids;
	int i;

	xids = xid_space_alloc();
	if (IS_ERR(xids))
		return NULL;

	xps = kmalloc(sizeof(*xps), gfp_flags);
	if (xps != NULL) {
		xps->xps_xid_space = xids;
		xps->xps_localports_usage = NULL;
		spin_lock_init(&xps->xps_lock);
		kref_init(&xps->xps_kref);
		xps->xps_nxprts = xps->xps_nactive = 0;
		atomic_long_set(&xps->xps_queuelen, 0);
		xps->xps_net = NULL;
		INIT_LIST_HEAD(&xps->xps_xprt_list);
		for (i = 0; i < XPRT_MAX_PORTS; i++)
			INIT_LIST_HEAD(&xps->xps_remoteport_xprt_list[i]);
		xps->xps_iter_ops = &rpc_xprt_iter_singular;
		xprt_switch_add_xprt_locked(xps, xprt, &xids);
	}

	xid_space_put(xids);

	return xps;
}

static void xprt_switch_free_entries(struct rpc_xprt_switch *xps)
{
	spin_lock(&xps->xps_lock);
	while (!list_empty(&xps->xps_xprt_list)) {
		struct rpc_xprt *xprt;

		xprt = list_first_entry(&xps->xps_xprt_list,
				struct rpc_xprt, xprt_switch);
		xprt_switch_remove_xprt_locked(xps, xprt);
		spin_unlock(&xps->xps_lock);
		xprt_put(xprt);
		spin_lock(&xps->xps_lock);
	}
	spin_unlock(&xps->xps_lock);
}

static void xprt_switch_free(struct kref *kref)
{
	struct rpc_xprt_switch *xps = container_of(kref,
			struct rpc_xprt_switch, xps_kref);

	xprt_switch_free_entries(xps);
	xprt_portusage_put(xps->xps_localports_usage);
	xid_space_put(xps->xps_xid_space);
	kfree_rcu(xps, xps_rcu);
}

/**
 * xprt_switch_get - Return a reference to a rpc_xprt_switch
 * @xps: pointer to struct rpc_xprt_switch
 *
 * Returns a reference to xps unless the refcount is already zero.
 */
struct rpc_xprt_switch *xprt_switch_get(struct rpc_xprt_switch *xps)
{
	if (xps != NULL && kref_get_unless_zero(&xps->xps_kref))
		return xps;
	return NULL;
}

/**
 * xprt_switch_put - Release a reference to a rpc_xprt_switch
 * @xps: pointer to struct rpc_xprt_switch
 *
 * Release the reference to xps, and free it once the refcount is zero.
 */
void xprt_switch_put(struct rpc_xprt_switch *xps)
{
	if (xps != NULL)
		kref_put(&xps->xps_kref, xprt_switch_free);
}

/**
 * rpc_xprt_switch_set_roundrobin - Set a round-robin policy on rpc_xprt_switch
 * @xps: pointer to struct rpc_xprt_switch
 *
 * Sets a round-robin default policy for iterators acting on xps.
 */
void rpc_xprt_switch_set_roundrobin(struct rpc_xprt_switch *xps)
{
	if (READ_ONCE(xps->xps_iter_ops) != &rpc_xprt_iter_roundrobin)
		WRITE_ONCE(xps->xps_iter_ops, &rpc_xprt_iter_roundrobin);
}

static
const struct rpc_xprt_iter_ops *xprt_iter_ops(const struct rpc_xprt_iter *xpi)
{
	if (xpi->xpi_ops != NULL)
		return xpi->xpi_ops;
	return rcu_dereference(xpi->xpi_xpswitch)->xps_iter_ops;
}

static
void xprt_iter_no_rewind(struct rpc_xprt_iter *xpi)
{
}

static
void xprt_iter_default_rewind(struct rpc_xprt_iter *xpi)
{
	WRITE_ONCE(xpi->xpi_cursor, NULL);
}

static
bool xprt_is_active(const struct rpc_xprt *xprt, struct rpc_xprt_iter_conf conf)
{
	if (conf.flags & RPC_XPRT_FLAGS_SKIP_UNCONNECTED)
		if (xprt_n_diversion(xprt) || xprt_n_recovery(xprt))
			return false;

#ifdef CONFIG_NVFS
	if (conf.flags & RPC_XPRT_FLAGS_LOCAL_GPU) {
		unsigned int priority = xprt->gpu_priority[conf.gpu_idx];
		if ((priority != 0 && priority != UINT_MAX) && priority > conf.best_priority)
			return false;
	}
#endif

	if (conf.flags & RPC_XPRT_FLAGS_REMOTE_PORT_IDX) {
		if (xprt->remote_port_idx != conf.remote_port_idx)
			return false;
	}

	return kref_read(&xprt->kref) != 0;
}

static inline
struct rpc_xprt *xprt_switch_next_by_conf(struct rpc_xprt *pos,
					  struct rpc_xprt_iter_conf conf)
{
	if (conf.flags & RPC_XPRT_FLAGS_REMOTE_PORT_IDX)
		return list_entry_rcu(pos->remote_port_xprt_switch.next,
				      typeof(*pos), remote_port_xprt_switch);
	else
		return list_entry_rcu(pos->xprt_switch.next,
				      typeof(*pos), xprt_switch);
}

static inline
struct rpc_xprt *xprt_switch_first_by_conf(struct rpc_xprt_switch *xps,
					  struct rpc_xprt_iter_conf conf)
{
	if (conf.flags & RPC_XPRT_FLAGS_REMOTE_PORT_IDX)
		return list_entry_rcu(xps->xps_remoteport_xprt_list[
				      conf.remote_port_idx].next,
				      struct rpc_xprt, remote_port_xprt_switch);
	else
		return list_entry_rcu(xps->xps_xprt_list.next,
				      struct rpc_xprt, xprt_switch);
}

static inline
bool xprt_switch_non_empty_by_conf(struct rpc_xprt_switch *xps,
				   struct rpc_xprt *pos,
				   struct rpc_xprt_iter_conf conf)
{
	if (conf.flags & RPC_XPRT_FLAGS_REMOTE_PORT_IDX)
		return &pos->remote_port_xprt_switch !=
			&xps->xps_remoteport_xprt_list[conf.remote_port_idx];
	else
		return &pos->xprt_switch != &xps->xps_xprt_list;
}

#ifndef  __list_check_rcu
#define __list_check_rcu(a, b, c) ({;})
#endif

#define list_for_each_xprt(pos, xps, conf)				\
        for (__list_check_rcu(dummy, false, 0),                         \
             pos = xprt_switch_first_by_conf(xps, conf);		\
                xprt_switch_non_empty_by_conf(xps, pos, conf);          \
                pos = xprt_switch_next_by_conf(pos, conf))


static
struct rpc_xprt *xprt_switch_find_first_entry(struct rpc_xprt_switch *xps,
					      struct rpc_xprt_iter_conf conf)
{
	struct rpc_xprt *pos;

	list_for_each_xprt(pos, xps, conf) {
		if (xprt_is_active(pos, conf))
			return pos;
	}
	return NULL;
}

static
struct rpc_xprt *xprt_iter_first_entry(struct rpc_xprt_iter *xpi,
				       struct rpc_xprt_iter_conf conf)
{
	struct rpc_xprt_switch *xps = rcu_dereference(xpi->xpi_xpswitch);

	if (xps == NULL)
		return NULL;
	return xprt_switch_find_first_entry(xps, conf);
}

static
struct rpc_xprt *xprt_switch_find_current_entry(struct rpc_xprt_switch *xps,
		const struct rpc_xprt *cur,
		struct rpc_xprt_iter_conf conf)
{
	struct rpc_xprt *pos;
	bool found = false;

	list_for_each_xprt(pos, xps, conf) {
		if (cur == pos)
			found = true;
		if (found && xprt_is_active(pos, conf))
			return pos;
	}
	return NULL;
}

static
struct rpc_xprt *xprt_iter_current_entry(struct rpc_xprt_iter *xpi,
					 struct rpc_xprt_iter_conf conf)
{
	struct rpc_xprt_switch *xps = rcu_dereference(xpi->xpi_xpswitch);
	struct rpc_xprt *xprt;

	if (xps == NULL)
		return NULL;
	if (xpi->xpi_cursor == NULL || xps->xps_nxprts < 2)
		return xprt_switch_find_first_entry(xps, conf);
	xprt = xprt_switch_find_current_entry(xps, xpi->xpi_cursor, conf);
	if (!xprt && conf.flags != 0)
		return xprt_switch_find_first_entry(xps, conf);

	return xprt;
}

bool rpc_xprt_switch_has_addr(struct rpc_xprt_switch *xps,
			      const struct sockaddr *sap)
{
	struct rpc_xprt *pos;

	if (xps == NULL || sap == NULL)
		return false;

	list_for_each_entry_rcu(pos, &xps->xps_xprt_list, xprt_switch) {
		if (rpc_cmp_addr_port(sap, (struct sockaddr *)&pos->addr)) {
			pr_info("RPC:   addr %s already in xprt switch\n",
				pos->address_strings[RPC_DISPLAY_ADDR]);
			return true;
		}
	}
	return false;
}

static
struct rpc_xprt *xprt_switch_find_next_entry(struct rpc_xprt_switch *xps,
		const struct rpc_xprt *cur,
		struct rpc_xprt_iter_conf conf)
{
	struct rpc_xprt *pos, *prev = NULL;
	bool found = false;

	list_for_each_xprt(pos, xps, conf) {
		if (cur == prev)
			found = true;
		if (found && xprt_is_active(pos, conf))
			return pos;
		prev = pos;
	}
	return NULL;
}

static
struct rpc_xprt *xprt_switch_set_next_cursor(struct rpc_xprt_switch *xps,
		struct rpc_xprt **cursor,
		xprt_switch_find_xprt_t find_next,
		struct rpc_xprt_iter_conf conf)
{
	struct rpc_xprt *pos, *old;

	old = smp_load_acquire(cursor);
	pos = find_next(xps, old, conf);
	smp_store_release(cursor, pos);
	return pos;
}

static
struct rpc_xprt *xprt_iter_next_entry_multiple(struct rpc_xprt_iter *xpi,
		xprt_switch_find_xprt_t find_next,
		struct rpc_xprt_iter_conf conf)
{
	struct rpc_xprt_switch *xps = rcu_dereference(xpi->xpi_xpswitch);

	if (xps == NULL)
		return NULL;
	return xprt_switch_set_next_cursor(xps, &xpi->xpi_cursor, find_next, conf);
}

static
struct rpc_xprt *__xprt_switch_find_next_entry_roundrobin(struct rpc_xprt_switch *xps,
		const struct rpc_xprt *cur, struct rpc_xprt_iter_conf conf)
{
	struct rpc_xprt *ret;

	ret = xprt_switch_find_next_entry(xps, cur, conf);
	if (ret != NULL)
		return ret;
	return xprt_switch_find_first_entry(xps, conf);
}

static
struct rpc_xprt *xprt_switch_find_next_entry_roundrobin(struct rpc_xprt_switch *xps,
		const struct rpc_xprt *cur, struct rpc_xprt_iter_conf conf)
{
	struct rpc_xprt *xprt;
	unsigned int nactive;
	unsigned int count = 0;

	for (;;) {
		unsigned long xprt_queuelen, xps_queuelen;

		xprt = __xprt_switch_find_next_entry_roundrobin(xps, cur, conf);
		if (!xprt)
			break;
		xprt_queuelen = atomic_long_read(&xprt->queuelen);
		xps_queuelen = atomic_long_read(&xps->xps_queuelen);
		nactive = READ_ONCE(xps->xps_nactive);
		/* Exit loop if xprt_queuelen <= average queue length */
		if (!(conf.flags & RPC_XPRT_FLAGS_IGNORE_CONGESTION) &&
		    xprt_queuelen * nactive <= xps_queuelen)
			break;
		cur = xprt;
		count += 1;

		if (conf.flags != 0 && count > nactive)
			break;
	}

	return xprt;
}

static
struct rpc_xprt *xprt_iter_next_entry_roundrobin(struct rpc_xprt_iter *xpi,
						 struct rpc_xprt_iter_conf conf)
{
	return xprt_iter_next_entry_multiple(xpi,
			xprt_switch_find_next_entry_roundrobin, conf);
}

static
struct rpc_xprt *xprt_switch_find_next_entry_all(struct rpc_xprt_switch *xps,
		const struct rpc_xprt *cur,
		struct rpc_xprt_iter_conf conf)
{
	return xprt_switch_find_next_entry(xps, cur, conf);
}

static
struct rpc_xprt *xprt_iter_next_entry_all(struct rpc_xprt_iter *xpi,
					  struct rpc_xprt_iter_conf conf)
{
	return xprt_iter_next_entry_multiple(xpi,
			xprt_switch_find_next_entry_all, conf);
}

/*
 * xprt_iter_rewind - Resets the xprt iterator
 * @xpi: pointer to rpc_xprt_iter
 *
 * Resets xpi to ensure that it points to the first entry in the list
 * of transports.
 */
static
void xprt_iter_rewind(struct rpc_xprt_iter *xpi)
{
	rcu_read_lock();
	xprt_iter_ops(xpi)->xpi_rewind(xpi);
	rcu_read_unlock();
}

static void __xprt_iter_init(struct rpc_xprt_iter *xpi,
		struct rpc_xprt_switch *xps,
		const struct rpc_xprt_iter_ops *ops)
{
	rcu_assign_pointer(xpi->xpi_xpswitch, xprt_switch_get(xps));
	xpi->xpi_cursor = NULL;
	xpi->xpi_ops = ops;
}

/**
 * xprt_iter_init - Initialise an xprt iterator
 * @xpi: pointer to rpc_xprt_iter
 * @xps: pointer to rpc_xprt_switch
 *
 * Initialises the iterator to use the default iterator ops
 * as set in xps. This function is mainly intended for internal
 * use in the rpc_client.
 */
void xprt_iter_init(struct rpc_xprt_iter *xpi,
		struct rpc_xprt_switch *xps)
{
	__xprt_iter_init(xpi, xps, NULL);
}

/**
 * xprt_iter_init_listall - Initialise an xprt iterator
 * @xpi: pointer to rpc_xprt_iter
 * @xps: pointer to rpc_xprt_switch
 *
 * Initialises the iterator to iterate once through the entire list
 * of entries in xps.
 */
void xprt_iter_init_listall(struct rpc_xprt_iter *xpi,
		struct rpc_xprt_switch *xps)
{
	__xprt_iter_init(xpi, xps, &rpc_xprt_iter_listall);
}

/**
 * xprt_iter_xchg_switch - Atomically swap out the rpc_xprt_switch
 * @xpi: pointer to rpc_xprt_iter
 * @newswitch: pointer to a new rpc_xprt_switch or NULL
 *
 * Swaps out the existing xpi->xpi_xpswitch with a new value.
 */
struct rpc_xprt_switch *xprt_iter_xchg_switch(struct rpc_xprt_iter *xpi,
		struct rpc_xprt_switch *newswitch)
{
	struct rpc_xprt_switch __rcu *oldswitch;

	/* Atomically swap out the old xpswitch */
	oldswitch = xchg(&xpi->xpi_xpswitch, RCU_INITIALIZER(newswitch));
	if (newswitch != NULL)
		xprt_iter_rewind(xpi);
	return rcu_dereference_protected(oldswitch, true);
}

/**
 * xprt_iter_destroy - Destroys the xprt iterator
 * @xpi: pointer to rpc_xprt_iter
 */
void xprt_iter_destroy(struct rpc_xprt_iter *xpi)
{
	xprt_switch_put(xprt_iter_xchg_switch(xpi, NULL));
}

/**
 * xprt_iter_xprt - Returns the rpc_xprt pointed to by the cursor
 * @xpi: pointer to rpc_xprt_iter
 *
 * Returns a pointer to the struct rpc_xprt that is currently
 * pointed to by the cursor.
 * Caller must be holding rcu_read_lock().
 */
struct rpc_xprt *xprt_iter_xprt(struct rpc_xprt_iter *xpi,
				struct rpc_xprt_iter_conf conf)
{
	WARN_ON_ONCE(!rcu_read_lock_held());
	return xprt_iter_ops(xpi)->xpi_xprt(xpi, conf);
}

static
struct rpc_xprt *xprt_iter_get_helper(struct rpc_xprt_iter *xpi,
		struct rpc_xprt *(*fn)(struct rpc_xprt_iter *,
				       struct rpc_xprt_iter_conf),
		struct rpc_xprt_iter_conf conf)
{
	struct rpc_xprt *ret;

	do {
		ret = fn(xpi, conf);
		if (ret == NULL)
			break;
		ret = xprt_get(ret);
	} while (ret == NULL);
	return ret;
}

/**
 * xprt_iter_get_xprt - Returns the rpc_xprt pointed to by the cursor
 * @xpi: pointer to rpc_xprt_iter
 *
 * Returns a reference to the struct rpc_xprt that is currently
 * pointed to by the cursor.
 */
struct rpc_xprt *xprt_iter_get_xprt(struct rpc_xprt_iter *xpi,
				    struct rpc_xprt_iter_conf conf)
{
	struct rpc_xprt *xprt;

	rcu_read_lock();
	xprt = xprt_iter_get_helper(xpi, xprt_iter_ops(xpi)->xpi_xprt, conf);
	rcu_read_unlock();
	return xprt;
}

/**
 * xprt_iter_get_next - Returns the next rpc_xprt following the cursor
 * @xpi: pointer to rpc_xprt_iter
 *
 * Returns a reference to the struct rpc_xprt that immediately follows the
 * entry pointed to by the cursor.
 */
struct rpc_xprt *xprt_iter_get_next(struct rpc_xprt_iter *xpi,
				    struct rpc_xprt_iter_conf conf)
{
	struct rpc_xprt *xprt;

	rcu_read_lock();
	xprt = xprt_iter_get_helper(xpi, xprt_iter_ops(xpi)->xpi_next, conf);
	rcu_read_unlock();
	return xprt;
}

#ifdef CONFIG_NVFS
/*
 * xprt_find_best_priority - Returns the lowest priority that can be used
 * for filtering xprts that are closest to the gpu_idx.
 */
bool xprt_find_best_priority(struct rpc_xprt_iter *xpi, unsigned int gpu_idx,
			     unsigned int *priority)
{
	struct rpc_xprt_switch *xps;
	struct rpc_xprt *pos;
	struct rpc_xprt_iter_conf conf = {
		.flags = RPC_XPRT_FLAGS_SKIP_UNCONNECTED |
			RPC_XPRT_FLAGS_IGNORE_CONGESTION,
	};
	bool ret = false;

	*priority = UINT_MAX;

	rcu_read_lock();
	xps = rcu_dereference(xpi->xpi_xpswitch);
	list_for_each_entry_rcu(pos, &xps->xps_xprt_list, xprt_switch) {
		unsigned int pos_priority;

		if (!xprt_is_active(pos, conf))
			continue;

		pos_priority = pos->gpu_priority[gpu_idx];
		if (pos_priority == 0 || pos_priority == UINT_MAX)
			continue;

		if (pos_priority < *priority) {
			*priority = pos_priority;
			ret = true;
		}
	}
	rcu_read_unlock();

	return ret;
}
#endif

/* Policy for always returning the first entry in the rpc_xprt_switch */
static
const struct rpc_xprt_iter_ops rpc_xprt_iter_singular = {
	.xpi_rewind = xprt_iter_no_rewind,
	.xpi_xprt = xprt_iter_first_entry,
	.xpi_next = xprt_iter_first_entry,
};

/* Policy for round-robin iteration of entries in the rpc_xprt_switch */
static
const struct rpc_xprt_iter_ops rpc_xprt_iter_roundrobin = {
	.xpi_rewind = xprt_iter_default_rewind,
	.xpi_xprt = xprt_iter_current_entry,
	.xpi_next = xprt_iter_next_entry_roundrobin,
};

/* Policy for once-through iteration of entries in the rpc_xprt_switch */
static
const struct rpc_xprt_iter_ops rpc_xprt_iter_listall = {
	.xpi_rewind = xprt_iter_default_rewind,
	.xpi_xprt = xprt_iter_current_entry,
	.xpi_next = xprt_iter_next_entry_all,
};
