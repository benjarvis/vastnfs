/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  linux/include/linux/sunrpc/xprt.h
 *
 *  Declarations for the RPC transport interface.
 *
 *  Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de>
 */

#ifndef _LINUX_SUNRPC_XPRT_H
#define _LINUX_SUNRPC_XPRT_H

#include <linux/uio.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/ktime.h>
#include <linux/kref.h>
#include <linux/sunrpc/sched.h>
#include <linux/sunrpc/xdr.h>
#include <linux/sunrpc/msg_prot.h>
#include <linux/sunrpc/debug.h>

#define RPC_MIN_SLOT_TABLE	(2U)
#define RPC_DEF_SLOT_TABLE	(16U)
#define RPC_MAX_SLOT_TABLE_LIMIT	(65536U)
#define RPC_MAX_SLOT_TABLE	RPC_MAX_SLOT_TABLE_LIMIT

#define RPC_CWNDSHIFT		(8U)
#define RPC_CWNDSCALE		(1U << RPC_CWNDSHIFT)
#define RPC_INITCWND		RPC_CWNDSCALE
#define RPC_MAXCWND(xprt)	((xprt)->max_reqs << RPC_CWNDSHIFT)
#define RPCXPRT_CONGESTED(xprt) ((xprt)->cong >= (xprt)->cwnd)

/*
 * This describes a timeout strategy
 */
struct rpc_timeout {
	unsigned long		to_initval,		/* initial timeout */
				to_maxval,		/* max timeout */
				to_increment;		/* if !exponential */
	unsigned int		to_retries;		/* max # of retries */
	unsigned char		to_exponential;
};

enum rpc_display_format_t {
	RPC_DISPLAY_ADDR = 0,
	RPC_DISPLAY_PORT,
	RPC_DISPLAY_PROTO,
	RPC_DISPLAY_HEX_ADDR,
	RPC_DISPLAY_HEX_PORT,
	RPC_DISPLAY_NETID,
	RPC_DISPLAY_MAX,
};

struct rpc_task;
struct rpc_xprt;
struct xprt_class;
struct seq_file;
struct svc_serv;
struct net;

/*
 * This describes a complete RPC request
 */
struct rpc_rqst {
	/*
	 * This is the user-visible part
	 */
	struct rpc_xprt *	rq_xprt;		/* RPC client */
	struct xdr_buf		rq_snd_buf;		/* send buffer */
	struct xdr_buf		rq_rcv_buf;		/* recv buffer */

	/*
	 * This is the private part
	 */
	struct rpc_task *	rq_task;	/* RPC task data */
	struct rpc_cred *	rq_cred;	/* Bound cred */
	__be32			rq_xid;		/* request XID */
	int			rq_cong;	/* has incremented xprt->cong */
	u32			rq_seqno;	/* gss seq no. used on req. */
	int			rq_enc_pages_num;
	struct page		**rq_enc_pages;	/* scratch pages for use by
						   gss privacy code */
	void (*rq_release_snd_buf)(struct rpc_rqst *); /* release rq_enc_pages */

	union {
		struct list_head	rq_list;	/* Slot allocation list */
		struct rb_node		rq_recv;	/* Receive queue */
	};

	struct list_head	rq_xmit;	/* Send queue */
	struct list_head	rq_xmit2;	/* Send queue */

	void			*rq_buffer;	/* Call XDR encode buffer */
	size_t			rq_callsize;
	void			*rq_rbuffer;	/* Reply XDR decode buffer */
	size_t			rq_rcvsize;
	size_t			rq_xmit_bytes_sent;	/* total bytes sent */
	size_t			rq_reply_bytes_recvd;	/* total reply bytes */
							/* received */

	struct xdr_buf		rq_private_buf;		/* The receive buffer
							 * used in the softirq.
							 */
	unsigned long		rq_majortimeo;	/* major timeout alarm */
	unsigned long		rq_minortimeo;	/* minor timeout alarm */
	unsigned long		rq_timeout;	/* Current timeout value */
	ktime_t			rq_rtt;		/* round-trip time */
	unsigned int		rq_retries;	/* # of retries */
	unsigned int		rq_connect_cookie;
						/* A cookie used to track the
						   state of the transport
						   connection */
	atomic_t		rq_pin;
	
	/*
	 * Partial send handling
	 */
	u32			rq_bytes_sent;	/* Bytes we have sent */

	ktime_t			rq_xtime;	/* transmit time stamp */
	int			rq_ntrans;

#if defined(CONFIG_SUNRPC_BACKCHANNEL)
	struct list_head	rq_bc_list;	/* Callback service list */
	unsigned long		rq_bc_pa_state;	/* Backchannel prealloc state */
	struct list_head	rq_bc_pa_list;	/* Backchannel prealloc list */
#endif /* CONFIG_SUNRPC_BACKCHANEL */
};
#define rq_svec			rq_snd_buf.head
#define rq_slen			rq_snd_buf.len

struct rpc_xprt_ops {
	void		(*set_buffer_size)(struct rpc_xprt *xprt, size_t sndsize, size_t rcvsize);
	int		(*reserve_xprt)(struct rpc_xprt *xprt, struct rpc_task *task);
	void		(*release_xprt)(struct rpc_xprt *xprt, struct rpc_task *task);
	void		(*alloc_slot)(struct rpc_xprt *xprt, struct rpc_task *task);
	void		(*free_slot)(struct rpc_xprt *xprt,
				     struct rpc_rqst *req);
	void		(*rpcbind)(struct rpc_task *task);
	void		(*set_port)(struct rpc_xprt *xprt, unsigned short port);
	void		(*connect)(struct rpc_xprt *xprt, struct rpc_task *task);
	int		(*get_srcaddr)(struct rpc_xprt *xprt, char *buf,
				       size_t buflen);
	unsigned short	(*get_srcport)(struct rpc_xprt *xprt);
	int		(*buf_alloc)(struct rpc_task *task);
	void		(*buf_free)(struct rpc_task *task);
	void		(*prepare_request)(struct rpc_rqst *req);
	int		(*send_request)(struct rpc_rqst *req);
	void		(*wait_for_reply_request)(struct rpc_task *task);
	void		(*timer)(struct rpc_xprt *xprt, struct rpc_task *task);
	void		(*release_request)(struct rpc_task *task);
	void		(*close)(struct rpc_xprt *xprt);
	void		(*destroy)(struct rpc_xprt *xprt);
	void		(*set_connect_timeout)(struct rpc_xprt *xprt,
					unsigned long connect_timeout,
					unsigned long reconnect_timeout);
	void		(*trace_transport_state)(struct rpc_xprt *xprt);
	void		(*print_stats)(struct rpc_xprt *xprt, struct seq_file *seq);
	int		(*enable_swap)(struct rpc_xprt *xprt);
	void		(*disable_swap)(struct rpc_xprt *xprt);
	void		(*inject_disconnect)(struct rpc_xprt *xprt);
	int		(*bc_setup)(struct rpc_xprt *xprt,
				    unsigned int min_reqs);
	size_t		(*bc_maxpayload)(struct rpc_xprt *xprt);
	unsigned int	(*bc_num_slots)(struct rpc_xprt *xprt);
	void		(*bc_free_rqst)(struct rpc_rqst *rqst);
	void		(*bc_destroy)(struct rpc_xprt *xprt,
				      unsigned int max_reqs);
};

/*
 * RPC transport identifiers
 *
 * To preserve compatibility with the historical use of raw IP protocol
 * id's for transport selection, UDP and TCP identifiers are specified
 * with the previous values. No such restriction exists for new transports,
 * except that they may not collide with these values (17 and 6,
 * respectively).
 */
#define XPRT_TRANSPORT_BC       (1 << 31)
enum xprt_transports {
	XPRT_TRANSPORT_UDP	= IPPROTO_UDP,
	XPRT_TRANSPORT_TCP	= IPPROTO_TCP,
	XPRT_TRANSPORT_BC_TCP	= IPPROTO_TCP | XPRT_TRANSPORT_BC,
	XPRT_TRANSPORT_RDMA	= 256,
	XPRT_TRANSPORT_BC_RDMA	= XPRT_TRANSPORT_RDMA | XPRT_TRANSPORT_BC,
	XPRT_TRANSPORT_LOCAL	= 257,
};

#define XPRT_MAX_GPU_IDX  64

enum rpc_xprt_role_t {
	RPC_XPRT_ROLE_REG = 0, /* Assumed for initialization */
	RPC_XPRT_ROLE_MD,
};

struct rpc_sysfs_xprt;
struct rpc_xprt {
	struct kref		kref;		/* Reference count */
	const struct rpc_xprt_ops *ops;		/* transport methods */
	unsigned int		id;		/* transport id */

	const struct rpc_timeout *timeout;	/* timeout parms */
	struct sockaddr_storage	addr;		/* server address */
	size_t			addrlen;	/* size of server address */
	int			prot;		/* IP protocol */

	unsigned long		cong;		/* current congestion */
	unsigned long		cwnd;		/* congestion window */

	size_t			max_payload;	/* largest RPC payload size,
						   in bytes */

	struct rpc_wait_queue	binding;	/* requests waiting on rpcbind */
	struct rpc_wait_queue	sending;	/* requests waiting to send */
	struct rpc_wait_queue	pending;	/* requests in flight */
	struct rpc_wait_queue	backlog;	/* waiting for slot */
	struct list_head	free;		/* free slots */
	unsigned int		max_reqs;	/* max number of slots */
	unsigned int		min_reqs;	/* min number of slots */
	unsigned int		num_reqs;	/* total slots */
	unsigned long		state;		/* transport state */
	unsigned char		resvport   : 1,	/* use a reserved port */
				reuseport  : 1; /* reuse port on reconnect */
	atomic_t		swapper;	/* we're swapping over this
						   transport */
	unsigned int		bind_index;	/* bind function index */

	/*
	 * Multipath
	 */
	struct list_head	xprt_switch;
	struct list_head	remote_port_xprt_switch;
	enum rpc_xprt_role_t    role;

#ifdef CONFIG_NVFS
	char                    pci_devname_address[0x40];
	/* Route priority of this xprt indexed by gpu index */
	unsigned int            gpu_priority[XPRT_MAX_GPU_IDX];
#endif

	/*
	 * Connection of transports
	 */
	unsigned long		bind_timeout,
				reestablish_timeout;
	unsigned int		connect_cookie;	/* A cookie that gets bumped
						   every time the transport
						   is reconnected */

	/*
	 * Disconnection of idle transports
	 */
	struct work_struct	task_cleanup;
	struct timer_list	timer;
	unsigned long		last_used,
				idle_timeout,
				connect_timeout,
				max_reconnect_timeout;

	/*
	 * Send stuff
	 */
	atomic_long_t		queuelen;
	spinlock_t		transport_lock;	/* lock transport info */
	spinlock_t		reserve_lock;	/* lock slot table */
	spinlock_t		queue_lock;	/* send/receive queue lock */
	struct rpc_xids	*	xid_space;	/* XID space */
	struct rpc_task *	snd_task;	/* Task blocked in send */

	struct list_head	xmit_queue;	/* Send queue */
	atomic_long_t		xmit_queuelen;

	struct svc_xprt		*bc_xprt;	/* NFSv4.1 backchannel */
#if defined(CONFIG_SUNRPC_BACKCHANNEL)
	struct svc_serv		*bc_serv;       /* The RPC service which will */
						/* process the callback */
	unsigned int		bc_alloc_max;
	unsigned int		bc_alloc_count;	/* Total number of preallocs */
	atomic_t		bc_slot_count;	/* Number of allocated slots */
	spinlock_t		bc_pa_lock;	/* Protects the preallocated
						 * items */
	struct list_head	bc_pa_list;	/* List of preallocated
						 * backchannel rpc_rqst's */
#endif /* CONFIG_SUNRPC_BACKCHANNEL */

	struct rb_root		recv_queue;	/* Receive queue */

	struct {
		unsigned long		bind_count,	/* total number of binds */
					connect_count,	/* total number of connects */
					connect_start,	/* connect start timestamp */
					connect_time,	/* jiffies waiting for connect */
					sends,		/* how many complete requests */
					recvs,		/* how many complete requests */
					bad_xids,	/* lookup_rqst didn't find XID */
					max_slots;	/* max rpc_slots used */

		unsigned long long	req_u,		/* average requests on the wire */
					bklog_u,	/* backlog queue utilization */
					sending_u,	/* send q utilization */
					pending_u;	/* pend q utilization */
	} stat;

	struct net		*xprt_net;
	struct sockaddr_storage localport;
	size_t			localport_len;
	struct sockaddr_storage orig_localport;
	size_t			orig_localport_len;
	bool                    localport_changed;
	struct xprt_portusage  *localport_usage;
	int                     localport_diversion;
	int			remote_port_idx;
	unsigned long		reconnection_attempts;

	const char		*servername;
	const char		*address_strings[RPC_DISPLAY_MAX];
#if IS_ENABLED(CONFIG_SUNRPC_DEBUG)
	struct dentry		*debugfs;		/* debugfs directory */
#endif
	struct rcu_head		rcu;
	const struct xprt_class	*xprt_class;
	struct rpc_sysfs_xprt	*xprt_sysfs;
	bool			main; /*mark if this is the 1st transport */
};

#if defined(CONFIG_SUNRPC_BACKCHANNEL)
/*
 * Backchannel flags
 */
#define	RPC_BC_PA_IN_USE	0x0001		/* Preallocated backchannel */
						/* buffer in use */
#endif /* CONFIG_SUNRPC_BACKCHANNEL */

#if defined(CONFIG_SUNRPC_BACKCHANNEL)
static inline int bc_prealloc(struct rpc_rqst *req)
{
	return test_bit(RPC_BC_PA_IN_USE, &req->rq_bc_pa_state);
}
#else
static inline int bc_prealloc(struct rpc_rqst *req)
{
	return 0;
}
#endif /* CONFIG_SUNRPC_BACKCHANNEL */

#define XPRT_CREATE_INFINITE_SLOTS	(1U)
#define XPRT_CREATE_NO_IDLE_TIMEOUT	(1U << 1)
#define XPRT_MAX_PORTS                  64

struct xprt_port {
	struct sockaddr_storage addr;
	size_t len;
	unsigned int xprt_diversions;
	unsigned int xprt_connections;
};

struct xprt_portusage {
	struct kref		kref;	/* Reference count */
	spinlock_t		lock;
	struct xprt_port        ports[XPRT_MAX_PORTS];
	bool			diversion_enabled;
	int			nr;
};

struct xprt_create {
	int			ident;		/* XPRT_TRANSPORT identifier */
	struct net *		net;
	struct sockaddr *	srcaddr;	/* optional local address */
	struct sockaddr *	dstaddr;	/* remote peer address */
	size_t			addrlen;
	struct sockaddr *       localport;
	size_t			localport_len;
	struct xprt_portusage   *localport_usage;
	struct xprt_portusage   *remoteport_usage;
	int			remote_port_idx;
	enum rpc_xprt_role_t    role;
	const char		*servername;
	struct svc_xprt		*bc_xprt;	/* NFSv4.1 backchannel */
	struct rpc_xprt_switch	*bc_xps;
	unsigned int		flags;
};

struct xprt_class {
	struct list_head	list;
	int			ident;		/* XPRT_TRANSPORT identifier */
	struct rpc_xprt *	(*setup)(struct xprt_create *);
#ifdef CONFIG_NVFS
	bool			(*page_gpu_idx)(struct page *, unsigned int *idx);
#endif
	struct module		*owner;
	char			name[32];
	const char *		netid[];
};

struct rpc_xids {
	atomic_t                next;
	struct kref		kref;		/* Reference count */
};

/*
 * Generic internal XID management functions
 */
struct rpc_xids		*xid_space_alloc(void);
struct rpc_xids		*xid_space_get(struct rpc_xids *xids);
uint32_t		xid_space_new_xid(struct rpc_xids *xids);
void			xid_space_put(struct rpc_xids *xids);

/*
 * Generic internal transport functions
 */
struct rpc_xprt		*xprt_create_transport(struct xprt_create *args);
void			xprt_connect(struct rpc_task *task);
unsigned long		xprt_reconnect_delay(const struct rpc_xprt *xprt);
void			xprt_reconnect_backoff(struct rpc_xprt *xprt,
					       unsigned long init_to);
void			xprt_reserve(struct rpc_task *task);
void			xprt_retry_reserve(struct rpc_task *task);
int			xprt_reserve_xprt(struct rpc_xprt *xprt, struct rpc_task *task);
int			xprt_reserve_xprt_cong(struct rpc_xprt *xprt, struct rpc_task *task);
void			xprt_alloc_slot(struct rpc_xprt *xprt, struct rpc_task *task);
void			xprt_free_slot(struct rpc_xprt *xprt,
				       struct rpc_rqst *req);
void			xprt_request_prepare(struct rpc_rqst *req);
bool			xprt_prepare_transmit(struct rpc_task *task);
void			xprt_request_enqueue_transmit(struct rpc_task *task);
void			xprt_request_enqueue_receive(struct rpc_task *task);
void			xprt_request_wait_receive(struct rpc_task *task);
void			xprt_request_dequeue_xprt(struct rpc_task *task);
bool			xprt_request_need_retransmit(struct rpc_task *task);
void			xprt_transmit(struct rpc_task *task);
void			xprt_end_transmit(struct rpc_task *task);
int			xprt_adjust_timeout(struct rpc_rqst *req);
void			xprt_release_xprt(struct rpc_xprt *xprt, struct rpc_task *task);
void			xprt_release_xprt_cong(struct rpc_xprt *xprt, struct rpc_task *task);
void			xprt_release(struct rpc_task *task);
struct rpc_xprt *	xprt_get(struct rpc_xprt *xprt);
void			xprt_put(struct rpc_xprt *xprt);
struct rpc_xprt *	xprt_alloc(struct net *net, size_t size,
				unsigned int num_prealloc,
				unsigned int max_req);
void			xprt_free(struct rpc_xprt *);
void			xprt_add_backlog(struct rpc_xprt *xprt, struct rpc_task *task);
bool			xprt_wake_up_backlog(struct rpc_xprt *xprt, struct rpc_rqst *req);
void			xprt_cleanup_ids(void);
const char *		xprt_type_to_string(int type);


static inline int
xprt_enable_swap(struct rpc_xprt *xprt)
{
	return xprt->ops->enable_swap(xprt);
}

static inline void
xprt_disable_swap(struct rpc_xprt *xprt)
{
	xprt->ops->disable_swap(xprt);
}

/*
 * Transport switch helper functions
 */
int			xprt_register_transport(struct xprt_class *type);
int			xprt_unregister_transport(struct xprt_class *type);
int			xprt_find_transport_ident(const char *);
void			xprt_wait_for_reply_request_def(struct rpc_task *task);
void			xprt_wait_for_reply_request_rtt(struct rpc_task *task);
void			xprt_wake_pending_tasks(struct rpc_xprt *xprt, int status);
void			xprt_wait_for_buffer_space(struct rpc_xprt *xprt);
bool			xprt_write_space(struct rpc_xprt *xprt);
void			xprt_adjust_cwnd(struct rpc_xprt *xprt, struct rpc_task *task, int result);
struct rpc_rqst *	xprt_lookup_rqst(struct rpc_xprt *xprt, __be32 xid);
void			xprt_update_rtt(struct rpc_task *task);
void			xprt_complete_rqst(struct rpc_task *task, int copied);
void			xprt_pin_rqst(struct rpc_rqst *req);
void			xprt_unpin_rqst(struct rpc_rqst *req);
void			xprt_release_rqst_cong(struct rpc_task *task);
bool			xprt_request_get_cong(struct rpc_xprt *xprt, struct rpc_rqst *req);
void			xprt_disconnect_done(struct rpc_xprt *xprt);
void			xprt_force_disconnect(struct rpc_xprt *xprt);
void			xprt_conditional_disconnect(struct rpc_xprt *xprt, unsigned int cookie);

bool			xprt_lock_connect(struct rpc_xprt *, struct rpc_task *, void *);
void			xprt_unlock_connect(struct rpc_xprt *, void *);
void			xprt_release_write(struct rpc_xprt *, struct rpc_task *);

#ifdef CONFIG_NVFS
bool			xprt_page_gpu_idx(struct page *page, unsigned int *gpu_idx);
#endif

void xprt_portusage_connect(struct xprt_portusage *usage, struct sockaddr *sockaddr,
			    int *diversion_idx);
void xprt_portusage_disconnect(struct xprt_portusage *usage, struct sockaddr *sockaddr);
int xprt_portusage_divert(struct xprt_portusage *usage, struct sockaddr *in_sockaddr,
			  struct sockaddr_storage *out_sockaddr, size_t *out_sockaddr_len);
void xprt_portusage_undivert(struct xprt_portusage *usage, int *idx);

struct rpc_portgroup;
struct xprt_portusage *xprt_portusage_new(void);
int xprt_portusage_push_address(struct xprt_portusage *usage,
				const struct sockaddr *addr,
				int addr_len);
struct xprt_portusage *xprt_portusage_get(struct xprt_portusage *usage);
void xprt_portusage_put(struct xprt_portusage *usage);
void xprt_diversion_recovery_kickoff(struct rpc_clnt *client, struct rpc_xprt *xprt);

/*
 * Reserved bit positions in xprt->state
 */
#define XPRT_LOCKED		(0)
#define XPRT_CONNECTED		(1)
#define XPRT_CONNECTING		(2)
#define XPRT_CLOSE_WAIT		(3)
#define XPRT_BOUND		(4)
#define XPRT_BINDING		(5)
#define XPRT_CLOSING		(6)
#define XPRT_OFFLINE		(7)
#define XPRT_REMOVE		(8)
#define XPRT_CONGESTED		(9)
#define XPRT_CWND_WAIT		(10)
#define XPRT_WRITE_SPACE	(11)
#define XPRT_SND_IS_COOKIE	(12)
#define XPRT_N_RECOVERY		(13)
#define XPRT_N_DIVERSION	(14)

static inline int xprt_connected(struct rpc_xprt *xprt)
{
	return test_bit(XPRT_CONNECTED, &xprt->state);
}

static inline void xprt_set_connected(struct rpc_xprt *xprt)
{
	bool was_connected = xprt_connected(xprt);

	set_bit(XPRT_CONNECTED, &xprt->state);

	if (!was_connected && xprt->localport_usage && xprt->localport_len > 0)
		xprt_portusage_connect(xprt->localport_usage,
				       (struct sockaddr *)&xprt->localport,
				       &xprt->localport_diversion);
}

static inline void xprt_clear_connected(struct rpc_xprt *xprt)
{
	bool was_connected = xprt_connected(xprt);

	clear_bit(XPRT_CONNECTED, &xprt->state);

	if (was_connected && xprt->localport_usage && xprt->localport_len > 0)
		xprt_portusage_disconnect(xprt->localport_usage,
					  (struct sockaddr *)&xprt->localport);
}

static inline int xprt_n_diversion(const struct rpc_xprt *xprt)
{
	return test_bit(XPRT_N_DIVERSION, &xprt->state);
}

static inline void xprt_set_n_diversion(struct rpc_xprt *xprt)
{
	set_bit(XPRT_N_DIVERSION, &xprt->state);
}

static inline void xprt_clear_n_diversion(struct rpc_xprt *xprt)
{
	clear_bit(XPRT_N_DIVERSION, &xprt->state);
}

static inline int xprt_n_recovery(const struct rpc_xprt *xprt)
{
	return test_bit(XPRT_N_RECOVERY, &xprt->state);
}

static inline void xprt_set_n_recovery(struct rpc_xprt *xprt)
{
	set_bit(XPRT_N_RECOVERY, &xprt->state);
}

static inline void xprt_clear_n_recovery(struct rpc_xprt *xprt)
{
	clear_bit(XPRT_N_RECOVERY, &xprt->state);
}

static inline int xprt_test_and_set_connected(struct rpc_xprt *xprt)
{
	return test_and_set_bit(XPRT_CONNECTED, &xprt->state);
}

static inline int xprt_test_and_clear_connected(struct rpc_xprt *xprt)
{
	return test_and_clear_bit(XPRT_CONNECTED, &xprt->state);
}

static inline void xprt_clear_connecting(struct rpc_xprt *xprt)
{
	smp_mb__before_atomic();
	clear_bit(XPRT_CONNECTING, &xprt->state);
	smp_mb__after_atomic();
}

static inline int xprt_connecting(struct rpc_xprt *xprt)
{
	return test_bit(XPRT_CONNECTING, &xprt->state);
}

static inline int xprt_test_and_set_connecting(struct rpc_xprt *xprt)
{
	return test_and_set_bit(XPRT_CONNECTING, &xprt->state);
}

static inline void xprt_set_bound(struct rpc_xprt *xprt)
{
	test_and_set_bit(XPRT_BOUND, &xprt->state);
}

static inline int xprt_bound(struct rpc_xprt *xprt)
{
	return test_bit(XPRT_BOUND, &xprt->state);
}

static inline void xprt_clear_bound(struct rpc_xprt *xprt)
{
	clear_bit(XPRT_BOUND, &xprt->state);
}

static inline void xprt_clear_binding(struct rpc_xprt *xprt)
{
	smp_mb__before_atomic();
	clear_bit(XPRT_BINDING, &xprt->state);
	smp_mb__after_atomic();
}

static inline int xprt_test_and_set_binding(struct rpc_xprt *xprt)
{
	return test_and_set_bit(XPRT_BINDING, &xprt->state);
}

#endif /* _LINUX_SUNRPC_XPRT_H */
