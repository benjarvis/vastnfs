#define %DEFINE_PROG_EVENT%_ENCODE(encname) \
	DEFINE_EVENT(%TRACE_SUNRPC_PROG_ENCODE_CLASS_NAME%, encname, \
		TP_PROTO( \
			 const struct xdr_stream *xdr, \
			 const u8 *buf, \
			 size_t len, \
			 size_t orig_len \
		),\
		TP_ARGS(xdr, buf, len, orig_len)); \

#define %DEFINE_PROG_EVENT%_DECODE(decname) \
	DEFINE_EVENT(%TRACE_SUNRPC_PROG_DECODE_CLASS_NAME%, decname, \
		TP_PROTO( \
			 const struct xdr_stream *xdr, \
			 const u8 *buf, \
			 size_t len, \
			 size_t orig_len \
		),\
		TP_ARGS(xdr, buf, len, orig_len));


#define %DEFINE_PROG_EVENT%(encname, decname) \
	%DEFINE_PROG_EVENT%_ENCODE(encname) \
	%DEFINE_PROG_EVENT%_DECODE(decname) \


DECLARE_EVENT_CLASS(%TRACE_SUNRPC_PROG_ENCODE_CLASS_NAME%,
		TP_PROTO(
			 const struct xdr_stream *xdr,
			 const u8 *buf,
			 size_t len,
			 size_t orig_len
		),

		TP_ARGS(xdr, buf, len, orig_len),

		TP_STRUCT__entry(
			__field(u32, orig_len)
			__field(u32, len)
			__field(u64, task_id)
			__field(unsigned int, client_id)
			__field(unsigned int, xprt_id)
			__field(unsigned int, prog_id)
			__field(u8, prog_ver)
			__field(u8, proc_num)
			__field(u32, xid)
			__dynamic_array(u8, buf, len)
		),

		TP_fast_assign(
			const struct rpc_rqst *rqstp = xdr->rqst;
			const struct rpc_task *task = rqstp->rq_task;
			struct rpc_clnt *clnt = task->tk_client;

			__entry->task_id = task->tk_pid;
			__entry->client_id = clnt->cl_clid;
			__entry->xprt_id = task->tk_xprt->id;
			__entry->xid = be32_to_cpu(rqstp->rq_xid);
			__entry->prog_id = clnt->cl_prog;
			__entry->prog_ver = clnt->cl_vers;
			__entry->proc_num = task->tk_msg.rpc_proc->p_proc;
			__entry->orig_len = orig_len;
			__entry->len = len;
			memcpy(__get_dynamic_array(buf), buf, len);
		),

		TP_printk(
			"task:%llx@%u xprt=%d rpctype=%d/%d:%d xid=0x%08x orig_len=%d buf=%s",
			__entry->task_id, __entry->client_id, __entry->xprt_id,
			__entry->prog_id, __entry->prog_ver, __entry->proc_num,
			__entry->xid, __entry->orig_len,
			__print_array(__get_dynamic_array(buf), __entry->len, 1)
		)
);

DECLARE_EVENT_CLASS(%TRACE_SUNRPC_PROG_DECODE_CLASS_NAME%,
		TP_PROTO(
			 const struct xdr_stream *xdr,
			 const u8 *buf,
			 size_t len,
			 size_t orig_len
		),

		TP_ARGS(xdr, buf, len, orig_len),

		TP_STRUCT__entry(
			__field(u32, orig_len)
			__field(u32, len)
			__field(u64, task_id)
			__field(unsigned int, client_id)
			__field(unsigned int, xprt_id)
			__field(unsigned int, prog_id)
			__field(u8, prog_ver)
			__field(u8, proc_num)
			__field(u32, xid)
			__dynamic_array(u8, buf, len)
		),

		TP_fast_assign(
			const struct rpc_rqst *rqstp = xdr->rqst;
			const struct rpc_task *task = rqstp->rq_task;
			struct rpc_clnt *clnt = task->tk_client;

			__entry->task_id = task->tk_pid;
			__entry->client_id = clnt->cl_clid;
			__entry->xprt_id = task->tk_xprt->id;
			__entry->xid = be32_to_cpu(rqstp->rq_xid);
			__entry->prog_id = clnt->cl_prog;
			__entry->prog_ver = clnt->cl_vers;
			__entry->proc_num = task->tk_msg.rpc_proc->p_proc;
			__entry->orig_len = orig_len;
			__entry->len = len;
			memcpy(__get_dynamic_array(buf), buf, len);
		),

		TP_printk(
			"task:%llx@%u xprt=%d rpctype=%d/%d:%d xid=0x%08x orig_len=%d buf=%s",
			__entry->task_id, __entry->client_id, __entry->xprt_id,
			__entry->prog_id, __entry->prog_ver, __entry->proc_num,
			__entry->xid, __entry->orig_len,
			__print_array(__get_dynamic_array(buf), __entry->len, 1)
		)
);
