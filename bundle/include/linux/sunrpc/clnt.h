/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  linux/include/linux/sunrpc/clnt.h
 *
 *  Declarations for the high-level RPC client interface
 *
 *  Copyright (C) 1995, 1996, Olaf Kirch <okir@monad.swb.de>
 */

#ifndef _LINUX_SUNRPC_CLNT_H
#define _LINUX_SUNRPC_CLNT_H

#include <linux/types.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/refcount.h>

#include <linux/sunrpc/msg_prot.h>
#include <linux/sunrpc/sched.h>
#include <linux/sunrpc/xprt.h>
#include <linux/sunrpc/auth.h>
#include <linux/sunrpc/stats.h>
#include <linux/sunrpc/xdr.h>
#include <linux/sunrpc/timer.h>
#include <linux/sunrpc/rpc_pipe_fs.h>
#include <linux/sunrpc/debug.h>
#include <asm/signal.h>
#include <linux/path.h>
#include <net/ipv6.h>
#include <linux/sunrpc/xprtmultipath.h>

struct rpc_inode;
struct rpc_sysfs_client;

/*
 * The high-level client handle
 */
struct rpc_clnt {
	refcount_t		cl_count;	/* Number of references */
	unsigned int		cl_clid;	/* client id */
	struct list_head	cl_clients;	/* Global list of clients */
	struct list_head	cl_tasks;	/* List of tasks */
	spinlock_t		cl_lock;	/* spinlock */
	struct rpc_xprt __rcu *	cl_xprt;	/* transport */
	const struct rpc_procinfo *cl_procinfo;	/* procedure info */
	u32			cl_prog,	/* RPC program number */
				cl_vers,	/* RPC version number */
				cl_maxproc;	/* max procedure number */

	struct rpc_auth *	cl_auth;	/* authenticator */
	struct rpc_stat *	cl_stats;	/* per-program statistics */
	struct rpc_iostats *	cl_metrics;	/* per-client statistics */

	unsigned int		cl_softrtry : 1,/* soft timeouts */
				cl_softerr  : 1,/* Timeouts return errors */
				cl_discrtry : 1,/* disconnect before retry */
				cl_noretranstimeo: 1,/* No retransmit timeouts */
				cl_autobind : 1,/* use getport() */
				cl_chatty   : 1;/* be verbose */

	struct rpc_rtt *	cl_rtt;		/* RTO estimator data */
	const struct rpc_timeout *cl_timeout;	/* Timeout strategy */

	struct xprt_portusage * cl_localports_usage;
	struct xprt_portusage  *cl_remoteports_usage;
	u32                     cl_remoteports_offset;
	bool                    cl_remoteports_offset_provided;

	bool                    cl_spread_reads;
  	bool                    cl_spread_writes;
	unsigned int		cl_mdconnect;

	atomic_t		cl_swapper;	/* swapfile count */
	int			cl_nodelen;	/* nodename length */
	char 			cl_nodename[UNX_MAXNODENAME+1];
	struct rpc_pipe_dir_head cl_pipedir_objects;
	struct rpc_clnt *	cl_parent;	/* Points to parent of clones */
	struct rpc_rtt		cl_rtt_default;
	struct rpc_timeout	cl_timeout_default;
	const struct rpc_program *cl_program;
	const char *		cl_principal;	/* use for machine cred */
#if IS_ENABLED(CONFIG_SUNRPC_DEBUG)
	struct dentry		*cl_debugfs;	/* debugfs directory */
#endif
	struct rpc_sysfs_client *cl_sysfs;	/* sysfs directory */
	/* cl_work is only needed after cl_xpi is no longer used,
	 * and that are of similar size
	 */
	union {
		struct rpc_xprt_iter	cl_xpi;
		struct work_struct	cl_work;
	};
	const struct cred	*cl_cred;
	unsigned int		cl_max_connect; /* max number of transports not to the same IP */
	struct super_block *pipefs_sb;
};

/*
 * General RPC program info
 */
#define RPC_MAXVERSION		4
struct rpc_program {
	const char *		name;		/* protocol name */
	u32			number;		/* program number */
	unsigned int		nrvers;		/* number of versions */
	const struct rpc_version **	version;	/* version array */
	struct rpc_stat *	stats;		/* statistics */
	const char *		pipe_dir_name;	/* path to rpc_pipefs dir */
};

struct rpc_version {
	u32			number;		/* version number */
	unsigned int		nrprocs;	/* number of procs */
	const struct rpc_procinfo *procs;	/* procedure array */
	unsigned int		*counts;	/* call counts */
};

/*
 * Procedure information
 */
struct rpc_procinfo {
	u32			p_proc;		/* RPC procedure number */
	kxdreproc_t		p_encode;	/* XDR encode function */
	kxdrdproc_t		p_decode;	/* XDR decode function */
	unsigned int		p_arglen;	/* argument hdr length (u32) */
	unsigned int		p_replen;	/* reply hdr length (u32) */
	unsigned int		p_timer;	/* Which RTT timer to use */
	u32			p_statidx;	/* Which procedure to account */
	const char *		p_name;		/* name of procedure */
};

#define RPC_MAX_PORTS 2048

struct rpc_portgroup {
	int nr;
	bool dns;
	struct sockaddr_storage addrs[RPC_MAX_PORTS];
	size_t lens[RPC_MAX_PORTS];
};

bool rpc_portgroup_eq(const struct rpc_portgroup *pg1, const struct rpc_portgroup *pg2);
uint32_t rpc_calc_portgroup_offset(struct net *net,
				   uint32_t localport_addr,
				   const struct rpc_portgroup *ports,
				   bool remoteports_offset_provided,
				   uint32_t remoteports_offset);

struct rpc_create_args {
	struct net		*net;
	int			protocol;
	struct sockaddr		*address;
	size_t			addrsize;
	struct rpc_portgroup    *localports;
	bool			localports_failover;
	struct rpc_portgroup    *remoteports;
	struct sockaddr		*saddress;
	const struct rpc_timeout *timeout;
	const char		*servername;
	const char		*nodename;
	const struct rpc_program *program;
	u32			prognumber;	/* overrides program->number */
	u32			version;
	rpc_authflavor_t	authflavor;
	u32			nconnect;
	u32                     remoteports_offset;
	bool                    remoteports_offset_provided;
  	bool                    spread_reads;
  	bool                    spread_writes;
	unsigned int		mdconnect;
	unsigned long		flags;
	char			*client_name;
	struct svc_xprt		*bc_xprt;	/* NFSv4.1 backchannel */
	const struct cred	*cred;
	unsigned int		max_connect;
};

struct rpc_add_xprt_test {
	void (*add_xprt_test)(struct rpc_clnt *clnt,
		struct rpc_xprt *xprt,
		void *calldata);
	void *data;
};

/* Values for "flags" field */
#define RPC_CLNT_CREATE_HARDRTRY	(1UL << 0)
#define RPC_CLNT_CREATE_AUTOBIND	(1UL << 2)
#define RPC_CLNT_CREATE_NONPRIVPORT	(1UL << 3)
#define RPC_CLNT_CREATE_NOPING		(1UL << 4)
#define RPC_CLNT_CREATE_DISCRTRY	(1UL << 5)
#define RPC_CLNT_CREATE_QUIET		(1UL << 6)
#define RPC_CLNT_CREATE_INFINITE_SLOTS	(1UL << 7)
#define RPC_CLNT_CREATE_NO_IDLE_TIMEOUT	(1UL << 8)
#define RPC_CLNT_CREATE_NO_RETRANS_TIMEOUT	(1UL << 9)
#define RPC_CLNT_CREATE_SOFTERR		(1UL << 10)
#define RPC_CLNT_CREATE_REUSEPORT	(1UL << 11)
#define RPC_CLNT_CREATE_CONNECTED	(1UL << 12)

struct rpc_clnt *rpc_create(struct rpc_create_args *args);
struct rpc_clnt	*rpc_bind_new_program(struct rpc_clnt *,
				const struct rpc_program *, u32);
struct rpc_clnt *rpc_clone_client(struct rpc_clnt *);
struct rpc_clnt *rpc_clone_client_set_auth(struct rpc_clnt *,
				rpc_authflavor_t);
int		rpc_switch_client_transport(struct rpc_clnt *,
				struct xprt_create *,
				const struct rpc_timeout *);

void		rpc_shutdown_client(struct rpc_clnt *);
void		rpc_release_client(struct rpc_clnt *);
void		rpc_task_release_transport(struct rpc_task *);
void		rpc_task_release_client(struct rpc_task *);
struct rpc_xprt	*rpc_task_get_xprt(struct rpc_clnt *clnt,
		struct rpc_xprt *xprt);

int		rpcb_create_local(struct net *);
void		rpcb_put_local(struct net *);
int		rpcb_register(struct net *, u32, u32, int, unsigned short);
int		rpcb_v4_register(struct net *net, const u32 program,
				 const u32 version,
				 const struct sockaddr *address,
				 const char *netid);
void		rpcb_getport_async(struct rpc_task *);

void rpc_prepare_reply_pages(struct rpc_rqst *req, struct page **pages,
			     unsigned int base, unsigned int len,
			     unsigned int hdrsize);
void		rpc_call_start(struct rpc_task *);
int		rpc_call_async(struct rpc_clnt *clnt,
			       const struct rpc_message *msg, int flags,
			       const struct rpc_call_ops *tk_ops,
			       void *calldata);

extern const struct rpc_call_ops rpc_default_ops;

#define	rpc_call_sync_raw_multipath(clnt, msg, _flags_, hash, tracecode, pid) ({ \
		struct rpc_task_setup _task_setup_data_ = { \
			.rpc_client = clnt, \
			.rpc_message = msg, \
			.callback_ops = &rpc_default_ops, \
			.flags = _flags_, \
			.multipath_hash = hash, \
			.owner_pid = pid ? get_pid(pid) : NULL, \
		}; \
		int _status_; \
	        \
		WARN_ON_ONCE(_task_setup_data_.flags & RPC_TASK_ASYNC); \
		if (_task_setup_data_.flags & RPC_TASK_ASYNC) { \
			rpc_release_calldata(_task_setup_data_.callback_ops, \
				_task_setup_data_.callback_data); \
			_status_ = -EINVAL; \
		} else { \
			struct rpc_task	*_task_ = \
				rpc_build_task(&_task_setup_data_); \
			if (IS_ERR(_task_)) { \
				_status_ = PTR_ERR(_task_); \
			} else { \
				tracecode; \
				rpc_start_task(_task_); \
				_status_ = _task_->tk_status; \
				rpc_put_task(_task_); \
			} \
		} \
		if (_task_setup_data_.owner_pid) \
			put_pid(_task_setup_data_.owner_pid); \
		_status_; \
	})

#define	rpc_call_sync(clnt, msg, flags, tracecode) \
	rpc_call_sync_raw_multipath(clnt, msg, flags, 0, tracecode, NULL)

#define	rpc_call_sync_multipath(clnt, msg, flags, hash, tracecode) \
	rpc_call_sync_raw_multipath(clnt, msg, flags, hash, tracecode, NULL)

#define	rpc_call_sync_pid(clnt, msg, flags, tracecode, pid) \
	rpc_call_sync_raw_multipath(clnt, msg, flags, 0, tracecode, pid)

#define	rpc_call_sync_multipath_pid(clnt, msg, flags, hash, tracecode, pid) \
	rpc_call_sync_raw_multipath(clnt, msg, flags, hash, tracecode, pid)

#ifdef CONFIG_NVFS
bool		rpc_page_gpu_idx(struct page *page, unsigned int *gpu_idx);
#endif

struct rpc_task *rpc_call_null_helper(struct rpc_clnt *clnt,
		struct rpc_xprt *xprt, struct rpc_cred *cred, int flags,
		const struct rpc_call_ops *ops, void *data);
struct rpc_task *rpc_call_null(struct rpc_clnt *clnt, struct rpc_cred *cred,
			       int flags);
int		rpc_restart_call_prepare(struct rpc_task *);
int		rpc_restart_call(struct rpc_task *);
void		rpc_setbufsize(struct rpc_clnt *, unsigned int, unsigned int);
struct net *	rpc_net_ns(struct rpc_clnt *);
size_t		rpc_max_payload(struct rpc_clnt *);
size_t		rpc_max_bc_payload(struct rpc_clnt *);
unsigned int	rpc_num_bc_slots(struct rpc_clnt *);
void		rpc_force_rebind(struct rpc_clnt *);
size_t		rpc_peeraddr(struct rpc_clnt *, struct sockaddr *, size_t);
const char	*rpc_peeraddr2str(struct rpc_clnt *, enum rpc_display_format_t);
int		rpc_localaddr(struct rpc_clnt *, struct sockaddr *, size_t);

int 		rpc_clnt_iterate_for_each_xprt(struct rpc_clnt *clnt,
			int (*fn)(struct rpc_clnt *, struct rpc_xprt *, void *),
			void *data);

struct rpc_clnt_test_and_add_xprt_flags {
	bool keep_and_divert_on_error;
	bool allow_remote_dups;
	void *in_max_connect;
};

int 		rpc_clnt_test_and_add_xprt(struct rpc_clnt *clnt,
			struct rpc_xprt_switch *xps,
			struct rpc_xprt *xprt,
			void *dummy);
int		rpc_clnt_add_xprt(struct rpc_clnt *, struct xprt_create *,
			int (*setup)(struct rpc_clnt *,
				struct rpc_xprt_switch *,
				struct rpc_xprt *,
				void *),
			void *data);
void		rpc_set_connect_timeout(struct rpc_clnt *clnt,
			unsigned long connect_timeout,
			unsigned long reconnect_timeout);

int		rpc_clnt_setup_test_and_add_xprt(struct rpc_clnt *,
			struct rpc_xprt_switch *,
			struct rpc_xprt *,
			void *);

const char *rpc_proc_name(const struct rpc_task *task);

void rpc_clnt_xprt_switch_put(struct rpc_clnt *);
void rpc_clnt_xprt_switch_add_xprt(struct rpc_clnt *, struct rpc_xprt *);
bool rpc_clnt_xprt_switch_has_addr(struct rpc_clnt *clnt,
			const struct sockaddr *sap);
void rpc_cleanup_clids(void);

static inline int rpc_reply_expected(struct rpc_task *task)
{
	return (task->tk_msg.rpc_proc != NULL) &&
		(task->tk_msg.rpc_proc->p_decode != NULL);
}

static inline void rpc_task_close_connection(struct rpc_task *task)
{
	if (task->tk_xprt)
		xprt_force_disconnect(task->tk_xprt);
}

void rpc_clnt_show_localports(struct rpc_clnt *clnt, struct seq_file *seq);
extern bool sunrpc_expose_id_in_mountstats;

#endif /* _LINUX_SUNRPC_CLNT_H */
