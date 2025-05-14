/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linux/include/linux/sunrpc/types.h
 *
 * Generic types and misc stuff for RPC.
 *
 * Copyright (C) 1996, Olaf Kirch <okir@monad.swb.de>
 */

#ifndef _LINUX_SUNRPC_TYPES_H_
#define _LINUX_SUNRPC_TYPES_H_

#include <linux/timer.h>
#include <linux/sched/signal.h>
#include <linux/workqueue.h>
#include <linux/sunrpc/debug.h>
#include <linux/list.h>

/*
 * Shorthands
 */
#define signalled()		(signal_pending(current))

#define RPC_TASK_STATE_TRANSITION		\
       EM(RPC_TASK_ST_QUEUE_RECV, "QUEUE_RECV") \
       EM(RPC_TASK_ST_READY_QUEUE_XMIT, "READY_QUEUE_XMIT") \
       EM(RPC_TASK_ST_QUEUE_XMIT_CONG, "QUEUE_XMIT_CONG") \
       EM(RPC_TASK_ST_QUEUE_XMIT, "QUEUE_XMIT") \
       EM(RPC_TASK_ST_QUEUE_XMIT_NOSEQ, "QUEUE_XMIT_NOSEQ") \
       EM(RPC_TASK_ST_REFRESH_DELAY, "REFRESH_DELAY") \
       EM(RPC_TASK_ST_REFRESH_AGAIN, "REFRESH_AGAIN") \
       EM(RPC_TASK_ST_REFRESH_EXPIRED, "REFRESH_EXPIRED") \
       EM(RPC_TASK_ST_ENCODE_AGAIN, "ENCODE_AGAIN") \
       EM(RPC_TASK_ST_ENCODE_EXPIRED, "ENCODE_EXPIRED") \
       EM(RPC_TASK_ST_TIMEOUT_CHECK, "TIMEOUT_CHECK") \
       EM(RPC_TASK_ST_TIMEOUT_RETRY, "TIMEOUT_RETRY") \
       EM(RPC_TASK_ST_TIMEOUT_MAJOR, "TIMEOUT_MAJOR") \
       EM(RPC_TASK_ST_XMIT_RECV_DATA, "XMIT_RECV_DATA") \
       EM(RPC_TASK_ST_DELAY_3_DONE, "DELAY_3_DONE") \
       EMe(RPC_TASK_ST_DELAY_SMALL_DONE, "DELAY_SMALL_DONE") \

#undef EM
#undef EMe
#define EM(a, b)	a,
#define EMe(a, b)	a

enum rpc_task_state_transition {
	RPC_TASK_STATE_TRANSITION,
};

#endif /* _LINUX_SUNRPC_TYPES_H_ */
