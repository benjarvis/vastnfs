# SPDX-License-Identifier: GPL-2.0
obj-$(CONFIG_SUNRPC_XPRT_RDMA) += rpcrdma.o

ifeq (a,)

rpcrdma-y := transport.o rpc_rdma.o verbs.o frwr_ops.o \
	svc_rdma.o svc_rdma_backchannel.o svc_rdma_transport.o \
	svc_rdma_sendto.o svc_rdma_recvfrom.o svc_rdma_rw.o \
	module.o ib_compat.o nvfs_rpc_rdma.o debugfs.o
rpcrdma-$(CONFIG_SUNRPC_BACKCHANNEL) += backchannel.o

else

rpcrdma-y := dummy.o

endif
