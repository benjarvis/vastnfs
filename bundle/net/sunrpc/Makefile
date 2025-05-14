# SPDX-License-Identifier: GPL-2.0
#
# Makefile for Linux kernel SUN RPC
#


obj-$(CONFIG_SUNRPC) += sunrpc.o
obj-$(CONFIG_SUNRPC_GSS) += auth_gss/

ifeq ($(CONFIG_SUNRPC_XPRT_RDMA),m)
obj-m += xprtrdma/
else
obj-m += rpcrdma.o
rpcrdma-y := rpcrdma_dummy.o
endif

sunrpc-y := clnt.o xprt.o socklib.o xprtsock.o sched.o \
	    auth.o auth_null.o auth_unix.o \
	    svc.o svcsock.o svcauth.o svcauth_unix.o \
	    addr.o addr_external.o rpcb_clnt.o timer.o xdr.o \
	    sunrpc_syms.o cache.o rpc_pipe.o sysfs.o \
	    svc_xprt.o \
	    xprtmultipath.o
sunrpc-$(CONFIG_SUNRPC_DEBUG) += debugfs.o
sunrpc-$(CONFIG_SUNRPC_BACKCHANNEL) += backchannel_rqst.o
sunrpc-$(CONFIG_PROC_FS) += stats.o
sunrpc-$(CONFIG_SYSCTL) += sysctl.o

include $(src)/../../net/sunrpc/Makefile.lib
$(eval $(call XDR_TRACE_CLASS,RPCB,rpcb_clnt.o,rpcb_trace_classes.h,../../))

sunrpc-y += compat.o
