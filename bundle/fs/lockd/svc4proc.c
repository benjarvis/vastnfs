// SPDX-License-Identifier: GPL-2.0
/*
 * linux/fs/lockd/svc4proc.c
 *
 * Lockd server procedures. We don't implement the NLM_*_RES 
 * procedures because we don't use the async procedures.
 *
 * Copyright (C) 1996, Olaf Kirch <okir@monad.swb.de>
 */

#include <linux/types.h>
#include <linux/time.h>
#include <linux/lockd/lockd.h>
#include <linux/lockd/share.h>
#include <linux/sunrpc/svc_xprt.h>

#define NLMDBG_FACILITY		NLMDBG_CLIENT

/*
 * Obtain client and file from arguments
 */
static __be32
nlm4svc_retrieve_args(struct svc_rqst *rqstp, struct nlm_args *argp,
			struct nlm_host **hostp, struct nlm_file **filp)
{
	struct nlm_host		*host = NULL;
	struct nlm_file		*file = NULL;
	struct nlm_lock		*lock = &argp->lock;
	__be32			error = 0;

	/* nfsd callbacks must have been installed for this procedure */
	if (!nlmsvc_ops)
		return nlm_lck_denied_nolocks;

	if (lock->lock_start > OFFSET_MAX ||
	    (lock->lock_len && ((lock->lock_len - 1) > (OFFSET_MAX - lock->lock_start))))
		return nlm4_fbig;

	/* Obtain host handle */
	if (!(host = nlmsvc_lookup_host(rqstp, lock->caller, lock->len))
	 || (argp->monitor && nsm_monitor(host) < 0))
		goto no_locks;
	*hostp = host;

	/* Obtain file pointer. Not used by FREE_ALL call. */
	if (filp != NULL) {
		int mode = lock_to_openmode(&lock->fl);

		error = nlm_lookup_file(rqstp, &file, lock);
		if (error)
			goto no_locks;
		*filp = file;

		/* Set up the missing parts of the file_lock structure */
		lock->fl.fl_file  = file->f_file[mode];
		lock->fl.fl_pid = current->tgid;
		lock->fl.fl_start = (loff_t)lock->lock_start;
		lock->fl.fl_end = lock->lock_len ?
				   (loff_t)(lock->lock_start + lock->lock_len - 1) :
				   OFFSET_MAX;
		lock->fl.fl_lmops = &nlmsvc_lock_operations;
		nlmsvc_locks_init_private(&lock->fl, host, (pid_t)lock->svid);
		if (!lock->fl.fl_owner) {
			/* lockowner allocation has failed */
			nlmsvc_release_host(host);
			return nlm_lck_denied_nolocks;
		}
	}

	return 0;

no_locks:
	nlmsvc_release_host(host);
 	if (error)
		return error;	
	return nlm_lck_denied_nolocks;
}

/*
 * NULL: Test for presence of service
 */
static __be32
nlm4svc_proc_null(struct svc_rqst *rqstp)
{
	dprintk("lockd: NULL          called\n");
	return rpc_success;
}

/*
 * TEST: Check for conflicting lock
 */
static __be32
__nlm4svc_proc_test(struct svc_rqst *rqstp, struct nlm_res *resp)
{
	struct nlm_args *argp = rqstp->rq_argp;
	struct nlm_host	*host;
	struct nlm_file	*file;
	__be32 rc = rpc_success;

	dprintk("lockd: TEST4        called\n");
	resp->cookie = argp->cookie;

	/* Obtain client and file */
	if ((resp->status = nlm4svc_retrieve_args(rqstp, argp, &host, &file)))
		return resp->status == nlm_drop_reply ? rpc_drop_reply :rpc_success;

	/* Now check for conflicting locks */
	resp->status = nlmsvc_testlock(rqstp, file, host, &argp->lock, &resp->lock, &resp->cookie);
	if (resp->status == nlm_drop_reply)
		rc = rpc_drop_reply;
	else
		dprintk("lockd: TEST4        status %d\n", ntohl(resp->status));

	nlmsvc_release_lockowner(&argp->lock);
	nlmsvc_release_host(host);
	nlm_release_file(file);
	return rc;
}

static __be32
nlm4svc_proc_test(struct svc_rqst *rqstp)
{
	return __nlm4svc_proc_test(rqstp, rqstp->rq_resp);
}

static __be32
__nlm4svc_proc_lock(struct svc_rqst *rqstp, struct nlm_res *resp)
{
	struct nlm_args *argp = rqstp->rq_argp;
	struct nlm_host	*host;
	struct nlm_file	*file;
	__be32 rc = rpc_success;

	dprintk("lockd: LOCK          called\n");

	resp->cookie = argp->cookie;

	/* Obtain client and file */
	if ((resp->status = nlm4svc_retrieve_args(rqstp, argp, &host, &file)))
		return resp->status == nlm_drop_reply ? rpc_drop_reply :rpc_success;

#if 0
	/* If supplied state doesn't match current state, we assume it's
	 * an old request that time-warped somehow. Any error return would
	 * do in this case because it's irrelevant anyway.
	 *
	 * NB: We don't retrieve the remote host's state yet.
	 */
	if (host->h_nsmstate && host->h_nsmstate != argp->state) {
		resp->status = nlm_lck_denied_nolocks;
	} else
#endif

	/* Now try to lock the file */
	resp->status = nlmsvc_lock(rqstp, file, host, &argp->lock,
					argp->block, &argp->cookie,
					argp->reclaim);
	if (resp->status == nlm_drop_reply)
		rc = rpc_drop_reply;
	else
		dprintk("lockd: LOCK         status %d\n", ntohl(resp->status));

	nlmsvc_release_lockowner(&argp->lock);
	nlmsvc_release_host(host);
	nlm_release_file(file);
	return rc;
}

static __be32
nlm4svc_proc_lock(struct svc_rqst *rqstp)
{
	return __nlm4svc_proc_lock(rqstp, rqstp->rq_resp);
}

static __be32
__nlm4svc_proc_cancel(struct svc_rqst *rqstp, struct nlm_res *resp)
{
	struct nlm_args *argp = rqstp->rq_argp;
	struct nlm_host	*host;
	struct nlm_file	*file;

	dprintk("lockd: CANCEL        called\n");

	resp->cookie = argp->cookie;

	/* Don't accept requests during grace period */
	if (locks_in_grace(SVC_NET(rqstp))) {
		resp->status = nlm_lck_denied_grace_period;
		return rpc_success;
	}

	/* Obtain client and file */
	if ((resp->status = nlm4svc_retrieve_args(rqstp, argp, &host, &file)))
		return resp->status == nlm_drop_reply ? rpc_drop_reply :rpc_success;

	/* Try to cancel request. */
	resp->status = nlmsvc_cancel_blocked(SVC_NET(rqstp), file, &argp->lock);

	dprintk("lockd: CANCEL        status %d\n", ntohl(resp->status));
	nlmsvc_release_lockowner(&argp->lock);
	nlmsvc_release_host(host);
	nlm_release_file(file);
	return rpc_success;
}

static __be32
nlm4svc_proc_cancel(struct svc_rqst *rqstp)
{
	return __nlm4svc_proc_cancel(rqstp, rqstp->rq_resp);
}

/*
 * UNLOCK: release a lock
 */
static __be32
__nlm4svc_proc_unlock(struct svc_rqst *rqstp, struct nlm_res *resp)
{
	struct nlm_args *argp = rqstp->rq_argp;
	struct nlm_host	*host;
	struct nlm_file	*file;

	dprintk("lockd: UNLOCK        called\n");

	resp->cookie = argp->cookie;

	/* Don't accept new lock requests during grace period */
	if (locks_in_grace(SVC_NET(rqstp))) {
		resp->status = nlm_lck_denied_grace_period;
		return rpc_success;
	}

	/* Obtain client and file */
	if ((resp->status = nlm4svc_retrieve_args(rqstp, argp, &host, &file)))
		return resp->status == nlm_drop_reply ? rpc_drop_reply :rpc_success;

	/* Now try to remove the lock */
	resp->status = nlmsvc_unlock(SVC_NET(rqstp), file, &argp->lock);

	dprintk("lockd: UNLOCK        status %d\n", ntohl(resp->status));
	nlmsvc_release_lockowner(&argp->lock);
	nlmsvc_release_host(host);
	nlm_release_file(file);
	return rpc_success;
}

static __be32
nlm4svc_proc_unlock(struct svc_rqst *rqstp)
{
	return __nlm4svc_proc_unlock(rqstp, rqstp->rq_resp);
}

/*
 * GRANTED: A server calls us to tell that a process' lock request
 * was granted
 */
static __be32
__nlm4svc_proc_granted(struct svc_rqst *rqstp, struct nlm_res *resp)
{
	struct nlm_args *argp = rqstp->rq_argp;

	resp->cookie = argp->cookie;

	dprintk("lockd: GRANTED       called\n");
	resp->status = nlmclnt_grant(svc_addr(rqstp), &argp->lock);
	dprintk("lockd: GRANTED       status %d\n", ntohl(resp->status));
	return rpc_success;
}

static __be32
nlm4svc_proc_granted(struct svc_rqst *rqstp)
{
	return __nlm4svc_proc_granted(rqstp, rqstp->rq_resp);
}

/*
 * This is the generic lockd callback for async RPC calls
 */
static void nlm4svc_callback_exit(struct rpc_task *task, void *data)
{
	dprintk("lockd: task %llx callback returned %d\n", task->tk_pid,
			-task->tk_status);
}

static void nlm4svc_callback_release(void *data)
{
	nlmsvc_release_call(data);
}

static const struct rpc_call_ops nlm4svc_callback_ops = {
	.rpc_call_done = nlm4svc_callback_exit,
	.rpc_release = nlm4svc_callback_release,
};

/*
 * `Async' versions of the above service routines. They aren't really,
 * because we send the callback before the reply proper. I hope this
 * doesn't break any clients.
 */
static __be32 nlm4svc_callback(struct svc_rqst *rqstp, u32 proc,
		__be32 (*func)(struct svc_rqst *,  struct nlm_res *))
{
	struct nlm_args *argp = rqstp->rq_argp;
	struct nlm_host	*host;
	struct nlm_rqst	*call;
	__be32 stat;

	host = nlmsvc_lookup_host(rqstp,
				  argp->lock.caller,
				  argp->lock.len);
	if (host == NULL)
		return rpc_system_err;

	call = nlm_alloc_call(host);
	nlmsvc_release_host(host);
	if (call == NULL)
		return rpc_system_err;

	stat = func(rqstp, &call->a_res);
	if (stat != 0) {
		nlmsvc_release_call(call);
		return stat;
	}

	call->a_flags = RPC_TASK_ASYNC;
	if (nlm_async_reply(call, proc, &nlm4svc_callback_ops) < 0)
		return rpc_system_err;
	return rpc_success;
}

static __be32 nlm4svc_proc_test_msg(struct svc_rqst *rqstp)
{
	dprintk("lockd: TEST_MSG      called\n");
	return nlm4svc_callback(rqstp, NLMPROC_TEST_RES, __nlm4svc_proc_test);
}

static __be32 nlm4svc_proc_lock_msg(struct svc_rqst *rqstp)
{
	dprintk("lockd: LOCK_MSG      called\n");
	return nlm4svc_callback(rqstp, NLMPROC_LOCK_RES, __nlm4svc_proc_lock);
}

static __be32 nlm4svc_proc_cancel_msg(struct svc_rqst *rqstp)
{
	dprintk("lockd: CANCEL_MSG    called\n");
	return nlm4svc_callback(rqstp, NLMPROC_CANCEL_RES, __nlm4svc_proc_cancel);
}

static __be32 nlm4svc_proc_unlock_msg(struct svc_rqst *rqstp)
{
	dprintk("lockd: UNLOCK_MSG    called\n");
	return nlm4svc_callback(rqstp, NLMPROC_UNLOCK_RES, __nlm4svc_proc_unlock);
}

static __be32 nlm4svc_proc_granted_msg(struct svc_rqst *rqstp)
{
	dprintk("lockd: GRANTED_MSG   called\n");
	return nlm4svc_callback(rqstp, NLMPROC_GRANTED_RES, __nlm4svc_proc_granted);
}

/*
 * SHARE: create a DOS share or alter existing share.
 */
static __be32
nlm4svc_proc_share(struct svc_rqst *rqstp)
{
	struct nlm_args *argp = rqstp->rq_argp;
	struct nlm_res *resp = rqstp->rq_resp;
	struct nlm_host	*host;
	struct nlm_file	*file;

	dprintk("lockd: SHARE         called\n");

	resp->cookie = argp->cookie;

	/* Don't accept new lock requests during grace period */
	if (locks_in_grace(SVC_NET(rqstp)) && !argp->reclaim) {
		resp->status = nlm_lck_denied_grace_period;
		return rpc_success;
	}

	/* Obtain client and file */
	if ((resp->status = nlm4svc_retrieve_args(rqstp, argp, &host, &file)))
		return resp->status == nlm_drop_reply ? rpc_drop_reply :rpc_success;

	/* Now try to create the share */
	resp->status = nlmsvc_share_file(host, file, argp);

	dprintk("lockd: SHARE         status %d\n", ntohl(resp->status));
	nlmsvc_release_lockowner(&argp->lock);
	nlmsvc_release_host(host);
	nlm_release_file(file);
	return rpc_success;
}

/*
 * UNSHARE: Release a DOS share.
 */
static __be32
nlm4svc_proc_unshare(struct svc_rqst *rqstp)
{
	struct nlm_args *argp = rqstp->rq_argp;
	struct nlm_res *resp = rqstp->rq_resp;
	struct nlm_host	*host;
	struct nlm_file	*file;

	dprintk("lockd: UNSHARE       called\n");

	resp->cookie = argp->cookie;

	/* Don't accept requests during grace period */
	if (locks_in_grace(SVC_NET(rqstp))) {
		resp->status = nlm_lck_denied_grace_period;
		return rpc_success;
	}

	/* Obtain client and file */
	if ((resp->status = nlm4svc_retrieve_args(rqstp, argp, &host, &file)))
		return resp->status == nlm_drop_reply ? rpc_drop_reply :rpc_success;

	/* Now try to lock the file */
	resp->status = nlmsvc_unshare_file(host, file, argp);

	dprintk("lockd: UNSHARE       status %d\n", ntohl(resp->status));
	nlmsvc_release_lockowner(&argp->lock);
	nlmsvc_release_host(host);
	nlm_release_file(file);
	return rpc_success;
}

/*
 * NM_LOCK: Create an unmonitored lock
 */
static __be32
nlm4svc_proc_nm_lock(struct svc_rqst *rqstp)
{
	struct nlm_args *argp = rqstp->rq_argp;

	dprintk("lockd: NM_LOCK       called\n");

	argp->monitor = 0;		/* just clean the monitor flag */
	return nlm4svc_proc_lock(rqstp);
}

/*
 * FREE_ALL: Release all locks and shares held by client
 */
static __be32
nlm4svc_proc_free_all(struct svc_rqst *rqstp)
{
	struct nlm_args *argp = rqstp->rq_argp;
	struct nlm_host	*host;

	/* Obtain client */
	if (nlm4svc_retrieve_args(rqstp, argp, &host, NULL))
		return rpc_success;

	nlmsvc_free_host_resources(host);
	nlmsvc_release_host(host);
	return rpc_success;
}

/*
 * SM_NOTIFY: private callback from statd (not part of official NLM proto)
 */
static __be32
nlm4svc_proc_sm_notify(struct svc_rqst *rqstp)
{
	struct nlm_reboot *argp = rqstp->rq_argp;

	dprintk("lockd: SM_NOTIFY     called\n");

	if (!nlm_privileged_requester(rqstp)) {
		char buf[RPC_MAX_ADDRBUFLEN];
		printk(KERN_WARNING "lockd: rejected NSM callback from %s\n",
				svc_print_addr(rqstp, buf, sizeof(buf)));
		return rpc_system_err;
	}

	nlm_host_rebooted(SVC_NET(rqstp), argp);
	return rpc_success;
}

/*
 * client sent a GRANTED_RES, let's remove the associated block
 */
static __be32
nlm4svc_proc_granted_res(struct svc_rqst *rqstp)
{
	struct nlm_res *argp = rqstp->rq_argp;

        if (!nlmsvc_ops)
                return rpc_success;

        dprintk("lockd: GRANTED_RES   called\n");

        nlmsvc_grant_reply(&argp->cookie, argp->status);
        return rpc_success;
}

static __be32
nlm4svc_proc_unused(struct svc_rqst *rqstp)
{
	return rpc_proc_unavail;
}


/*
 * NLM Server procedures.
 */

struct nlm_void			{ int dummy; };

#define	Ck	(1+XDR_QUADLEN(NLM_MAXCOOKIELEN))	/* cookie */
#define	No	(1+1024/4)				/* netobj */
#define	St	1					/* status */
#define	Rg	4					/* range (offset + length) */

const struct svc_procedure nlmsvc_procedures4[24] = {
	[NLMPROC_NULL] = {
		.pc_func = nlm4svc_proc_null,
		.pc_decode = nlm4svc_decode_void,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_void),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = St,
		.pc_name = "NULL",
	},
	[NLMPROC_TEST] = {
		.pc_func = nlm4svc_proc_test,
		.pc_decode = nlm4svc_decode_testargs,
		.pc_encode = nlm4svc_encode_testres,
		.pc_argsize = sizeof(struct nlm_args),
		.pc_ressize = sizeof(struct nlm_res),
		.pc_xdrressize = Ck+St+2+No+Rg,
		.pc_name = "TEST",
	},
	[NLMPROC_LOCK] = {
		.pc_func = nlm4svc_proc_lock,
		.pc_decode = nlm4svc_decode_lockargs,
		.pc_encode = nlm4svc_encode_res,
		.pc_argsize = sizeof(struct nlm_args),
		.pc_ressize = sizeof(struct nlm_res),
		.pc_xdrressize = Ck+St,
		.pc_name = "LOCK",
	},
	[NLMPROC_CANCEL] = {
		.pc_func = nlm4svc_proc_cancel,
		.pc_decode = nlm4svc_decode_cancargs,
		.pc_encode = nlm4svc_encode_res,
		.pc_argsize = sizeof(struct nlm_args),
		.pc_ressize = sizeof(struct nlm_res),
		.pc_xdrressize = Ck+St,
		.pc_name = "CANCEL",
	},
	[NLMPROC_UNLOCK] = {
		.pc_func = nlm4svc_proc_unlock,
		.pc_decode = nlm4svc_decode_unlockargs,
		.pc_encode = nlm4svc_encode_res,
		.pc_argsize = sizeof(struct nlm_args),
		.pc_ressize = sizeof(struct nlm_res),
		.pc_xdrressize = Ck+St,
		.pc_name = "UNLOCK",
	},
	[NLMPROC_GRANTED] = {
		.pc_func = nlm4svc_proc_granted,
		.pc_decode = nlm4svc_decode_testargs,
		.pc_encode = nlm4svc_encode_res,
		.pc_argsize = sizeof(struct nlm_args),
		.pc_ressize = sizeof(struct nlm_res),
		.pc_xdrressize = Ck+St,
		.pc_name = "GRANTED",
	},
	[NLMPROC_TEST_MSG] = {
		.pc_func = nlm4svc_proc_test_msg,
		.pc_decode = nlm4svc_decode_testargs,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_args),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = St,
		.pc_name = "TEST_MSG",
	},
	[NLMPROC_LOCK_MSG] = {
		.pc_func = nlm4svc_proc_lock_msg,
		.pc_decode = nlm4svc_decode_lockargs,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_args),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = St,
		.pc_name = "LOCK_MSG",
	},
	[NLMPROC_CANCEL_MSG] = {
		.pc_func = nlm4svc_proc_cancel_msg,
		.pc_decode = nlm4svc_decode_cancargs,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_args),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = St,
		.pc_name = "CANCEL_MSG",
	},
	[NLMPROC_UNLOCK_MSG] = {
		.pc_func = nlm4svc_proc_unlock_msg,
		.pc_decode = nlm4svc_decode_unlockargs,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_args),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = St,
		.pc_name = "UNLOCK_MSG",
	},
	[NLMPROC_GRANTED_MSG] = {
		.pc_func = nlm4svc_proc_granted_msg,
		.pc_decode = nlm4svc_decode_testargs,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_args),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = St,
		.pc_name = "GRANTED_MSG",
	},
	[NLMPROC_TEST_RES] = {
		.pc_func = nlm4svc_proc_null,
		.pc_decode = nlm4svc_decode_void,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_res),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = St,
		.pc_name = "TEST_RES",
	},
	[NLMPROC_LOCK_RES] = {
		.pc_func = nlm4svc_proc_null,
		.pc_decode = nlm4svc_decode_void,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_res),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = St,
		.pc_name = "LOCK_RES",
	},
	[NLMPROC_CANCEL_RES] = {
		.pc_func = nlm4svc_proc_null,
		.pc_decode = nlm4svc_decode_void,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_res),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = St,
		.pc_name = "CANCEL_RES",
	},
	[NLMPROC_UNLOCK_RES] = {
		.pc_func = nlm4svc_proc_null,
		.pc_decode = nlm4svc_decode_void,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_res),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = St,
		.pc_name = "UNLOCK_RES",
	},
	[NLMPROC_GRANTED_RES] = {
		.pc_func = nlm4svc_proc_granted_res,
		.pc_decode = nlm4svc_decode_res,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_res),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = St,
		.pc_name = "GRANTED_RES",
	},
	[NLMPROC_NSM_NOTIFY] = {
		.pc_func = nlm4svc_proc_sm_notify,
		.pc_decode = nlm4svc_decode_reboot,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_reboot),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = St,
		.pc_name = "SM_NOTIFY",
	},
	[17] = {
		.pc_func = nlm4svc_proc_unused,
		.pc_decode = nlm4svc_decode_void,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_void),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = 0,
		.pc_name = "UNUSED",
	},
	[18] = {
		.pc_func = nlm4svc_proc_unused,
		.pc_decode = nlm4svc_decode_void,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_void),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = 0,
		.pc_name = "UNUSED",
	},
	[19] = {
		.pc_func = nlm4svc_proc_unused,
		.pc_decode = nlm4svc_decode_void,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_void),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = 0,
		.pc_name = "UNUSED",
	},
	[NLMPROC_SHARE] = {
		.pc_func = nlm4svc_proc_share,
		.pc_decode = nlm4svc_decode_shareargs,
		.pc_encode = nlm4svc_encode_shareres,
		.pc_argsize = sizeof(struct nlm_args),
		.pc_ressize = sizeof(struct nlm_res),
		.pc_xdrressize = Ck+St+1,
		.pc_name = "SHARE",
	},
	[NLMPROC_UNSHARE] = {
		.pc_func = nlm4svc_proc_unshare,
		.pc_decode = nlm4svc_decode_shareargs,
		.pc_encode = nlm4svc_encode_shareres,
		.pc_argsize = sizeof(struct nlm_args),
		.pc_ressize = sizeof(struct nlm_res),
		.pc_xdrressize = Ck+St+1,
		.pc_name = "UNSHARE",
	},
	[NLMPROC_NM_LOCK] = {
		.pc_func = nlm4svc_proc_nm_lock,
		.pc_decode = nlm4svc_decode_lockargs,
		.pc_encode = nlm4svc_encode_res,
		.pc_argsize = sizeof(struct nlm_args),
		.pc_ressize = sizeof(struct nlm_res),
		.pc_xdrressize = Ck+St,
		.pc_name = "NM_LOCK",
	},
	[NLMPROC_FREE_ALL] = {
		.pc_func = nlm4svc_proc_free_all,
		.pc_decode = nlm4svc_decode_notify,
		.pc_encode = nlm4svc_encode_void,
		.pc_argsize = sizeof(struct nlm_args),
		.pc_ressize = sizeof(struct nlm_void),
		.pc_xdrressize = St,
		.pc_name = "FREE_ALL",
	},
};
