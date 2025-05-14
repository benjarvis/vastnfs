/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linux/include/linux/sunrpc/debug.h
 *
 * Debugging support for sunrpc module
 *
 * Copyright (C) 1996, Olaf Kirch <okir@monad.swb.de>
 */
#ifndef _LINUX_SUNRPC_DEBUG_H_
#define _LINUX_SUNRPC_DEBUG_H_

#if defined(CONFIG_DYNAMIC_DEBUG)
# define dyn_printk(fmt, ...) dynamic_pr_debug(fmt, ##__VA_ARGS__)
#else
# define dyn_printk printk
#endif

#include <uapi/linux/sunrpc/debug.h>

/*
 * Debugging macros etc
 */
#if IS_ENABLED(CONFIG_SUNRPC_DEBUG)
extern unsigned int		rpc_debug;
extern unsigned int		nfs_debug;
extern unsigned int		nfsd_debug;
extern unsigned int		nlm_debug;
#endif

#define dprintk(fmt, ...)						\
	dfprintk(FACILITY, fmt, ##__VA_ARGS__)
#define dprintk_cont(fmt, ...)						\
	dfprintk_cont(FACILITY, fmt, ##__VA_ARGS__)
#define dprintk_rcu(fmt, ...)						\
	dfprintk_rcu(FACILITY, fmt, ##__VA_ARGS__)
#define dprintk_rcu_cont(fmt, ...)					\
	dfprintk_rcu_cont(FACILITY, fmt, ##__VA_ARGS__)

#undef ifdebug
#if IS_ENABLED(CONFIG_SUNRPC_DEBUG)
# define ifdebug(fac)		if (unlikely(rpc_debug & RPCDBG_##fac))

# define dfprintk(fac, fmt, ...)					\
do {									\
	ifdebug(fac)							\
		dyn_printk(KERN_DEFAULT fmt, ##__VA_ARGS__);		\
} while (0)

# define dfprintk_cont(fac, fmt, ...)					\
do {									\
	ifdebug(fac)							\
		dyn_printk(KERN_CONT fmt, ##__VA_ARGS__);			\
} while (0)

# define dfprintk_rcu(fac, fmt, ...)					\
do {									\
	ifdebug(fac) {							\
		rcu_read_lock();					\
		dyn_printk(KERN_DEFAULT fmt, ##__VA_ARGS__);		\
		rcu_read_unlock();					\
	}								\
} while (0)

# define dfprintk_rcu_cont(fac, fmt, ...)				\
do {									\
	ifdebug(fac) {							\
		rcu_read_lock();					\
		dyn_printk(KERN_CONT fmt, ##__VA_ARGS__);			\
		rcu_read_unlock();					\
	}								\
} while (0)

# define RPC_IFDEBUG(x)		x
#else
# define ifdebug(fac)		if (0)
# define dfprintk(fac, fmt, ...)	do {} while (0)
# define dfprintk_cont(fac, fmt, ...)	do {} while (0)
# define dfprintk_rcu(fac, fmt, ...)	do {} while (0)
# define RPC_IFDEBUG(x)
#endif

/*
 * Sysctl interface for RPC debugging
 */

struct rpc_clnt;
struct rpc_xprt;

#if IS_ENABLED(CONFIG_SUNRPC_DEBUG)
void		rpc_register_sysctl(void);
void		rpc_unregister_sysctl(void);
void		sunrpc_debugfs_init(void);
void		sunrpc_debugfs_exit(void);
void		rpc_clnt_debugfs_register(struct rpc_clnt *);
void		rpc_clnt_debugfs_unregister(struct rpc_clnt *);
void		rpc_xprt_debugfs_register(struct rpc_xprt *);
void		rpc_xprt_debugfs_unregister(struct rpc_xprt *);
void		sunrpc_debug_check_walltime_correlation(void);
#else
static inline void
sunrpc_debugfs_init(void)
{
	return;
}

static inline void
sunrpc_debugfs_exit(void)
{
	return;
}

static inline void
rpc_clnt_debugfs_register(struct rpc_clnt *clnt)
{
	return;
}

static inline void
rpc_clnt_debugfs_unregister(struct rpc_clnt *clnt)
{
	return;
}

static inline void
rpc_xprt_debugfs_register(struct rpc_xprt *xprt)
{
	return;
}

static inline void
rpc_xprt_debugfs_unregister(struct rpc_xprt *xprt)
{
	return;
}

static inline void sunrpc_debug_check_walltime_correlation(void)
{
}

#endif


/* Further assurance to to prevent misconfigured system */

#include "build-info.h"

const char *sunrpc_get_version(void);

#define VERSION_CHECK_MAY_RETURN do {\
	if (strcmp(sunrpc_get_version(), NFS_BUNDLE_VERSION)) { \
		printk(KERN_DEBUG "nfs-bundle mismatch: sunrpc is %s but %s is %s", \
		       sunrpc_get_version(), THIS_MODULE->name, NFS_BUNDLE_VERSION); \
		return -EIDRM; \
	} \
	printk(KERN_DEBUG "nfs-bundle: %s loading", THIS_MODULE->name); \
} while (0)

#define SUNRPC_CONCAT__(a,b) a ## b
#define SUNRPC_CONCAT_(a,b)  SUNRPC_CONCAT__(a,b)

/*
 * Invisible modification of old symbols names to private ones, to prevent old
 * versions of the dependent kernels modules of sunrpc from being accidently
 * loaded.
 */
#define xprt_create_transport \
    SUNRPC_CONCAT_(SUNRPC_CONCAT_(xprt_create_transport, _), NFS_BUNDLE_VERSION_ID)
#define rpc_shutdown_client \
    SUNRPC_CONCAT_(SUNRPC_CONCAT_(rpc_shutdown_client, _), NFS_BUNDLE_VERSION_ID)
#define svc_destroy \
    SUNRPC_CONCAT_(SUNRPC_CONCAT_(svc_destroy, _), NFS_BUNDLE_VERSION_ID)

#endif /* _LINUX_SUNRPC_DEBUG_H_ */
