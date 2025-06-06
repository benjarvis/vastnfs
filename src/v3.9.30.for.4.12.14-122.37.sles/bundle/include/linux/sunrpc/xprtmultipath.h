/* SPDX-License-Identifier: GPL-2.0 */
/*
 * RPC client multipathing definitions
 *
 * Copyright (c) 2015, 2016, Primary Data, Inc. All rights reserved.
 *
 * Trond Myklebust <trond.myklebust@primarydata.com>
 */
#ifndef _NET_SUNRPC_XPRTMULTIPATH_H
#define _NET_SUNRPC_XPRTMULTIPATH_H

struct rpc_xprt_iter_ops;
struct rpc_xprt_switch {
	spinlock_t		xps_lock;
	struct kref		xps_kref;
	struct rpc_xids	*	xps_xid_space;

	unsigned int		xps_nxprts;
	unsigned int		xps_nactive;
	atomic_long_t		xps_queuelen;
	struct list_head	xps_xprt_list;
	struct list_head	xps_remoteport_xprt_list[XPRT_MAX_PORTS];
	struct xprt_portusage  *xps_localports_usage;

	struct net *		xps_net;

	const struct rpc_xprt_iter_ops *xps_iter_ops;

	struct rcu_head		xps_rcu;
};

struct rpc_xprt_iter {
	struct rpc_xprt_switch __rcu *xpi_xpswitch;
	struct rpc_xprt *	xpi_cursor;

	const struct rpc_xprt_iter_ops *xpi_ops;
};

enum rpc_xprt_iter_flags {
	RPC_XPRT_FLAGS_SKIP_UNCONNECTED=1,
	RPC_XPRT_FLAGS_LOCAL_GPU=2,
	RPC_XPRT_FLAGS_REMOTE_PORT_IDX=4,
	RPC_XPRT_FLAGS_IGNORE_CONGESTION=8,
};

struct rpc_xprt_iter_conf {
	enum rpc_xprt_iter_flags flags:4;
	unsigned int best_priority:30;
	unsigned int gpu_idx:22;
	unsigned int remote_port_idx:8;
};

struct rpc_xprt_iter_ops {
	void (*xpi_rewind)(struct rpc_xprt_iter *);
	struct rpc_xprt *(*xpi_xprt)(struct rpc_xprt_iter *,
				     struct rpc_xprt_iter_conf conf);
	struct rpc_xprt *(*xpi_next)(struct rpc_xprt_iter *,
				     struct rpc_xprt_iter_conf conf);
};

extern struct rpc_xprt_switch *xprt_switch_alloc(struct rpc_xprt *xprt,
		gfp_t gfp_flags);

extern struct rpc_xprt_switch *xprt_switch_get(struct rpc_xprt_switch *xps);
extern void xprt_switch_put(struct rpc_xprt_switch *xps);

extern void rpc_xprt_switch_set_roundrobin(struct rpc_xprt_switch *xps);

extern void rpc_xprt_switch_add_xprt(struct rpc_xprt_switch *xps,
		struct rpc_xprt *xprt);
extern void rpc_xprt_switch_remove_xprt(struct rpc_xprt_switch *xps,
		struct rpc_xprt *xprt);

extern void xprt_iter_init(struct rpc_xprt_iter *xpi,
		struct rpc_xprt_switch *xps);

extern void xprt_iter_init_listall(struct rpc_xprt_iter *xpi,
		struct rpc_xprt_switch *xps);

extern void xprt_iter_destroy(struct rpc_xprt_iter *xpi);

extern struct rpc_xprt_switch *xprt_iter_xchg_switch(
		struct rpc_xprt_iter *xpi,
		struct rpc_xprt_switch *newswitch);

extern struct rpc_xprt *xprt_iter_xprt(struct rpc_xprt_iter *xpi,
				       struct rpc_xprt_iter_conf conf);
extern struct rpc_xprt *xprt_iter_get_xprt(struct rpc_xprt_iter *xpi,
					   struct rpc_xprt_iter_conf conf);
extern struct rpc_xprt *xprt_iter_get_next(struct rpc_xprt_iter *xpi,
					   struct rpc_xprt_iter_conf conf);

extern bool rpc_xprt_switch_has_addr(struct rpc_xprt_switch *xps,
		const struct sockaddr *sap);

#ifdef CONFIG_NVFS
bool xprt_find_best_priority(struct rpc_xprt_iter *xpi, unsigned int gpu_idx,
			     unsigned int *priority);
#endif

#endif
