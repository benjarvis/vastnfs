# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the linux lock manager stuff
#

obj-$(CONFIG_LOCKD) += lockd.o

lockd-objs-y := clntlock.o clntproc.o clntxdr.o host.o svc.o svclock.o \
	        svcshare.o svcproc.o svcsubs.o mon.o xdr.o
lockd-objs-$(CONFIG_LOCKD_V4) += clnt4xdr.o xdr4.o svc4proc.o
lockd-objs-$(CONFIG_PROC_FS) += procfs.o
lockd-objs		      := $(lockd-objs-y)

include $(src)/../../net/sunrpc/Makefile.lib
$(eval $(call XDR_TRACE_CLASS,NLM4,clnt4xdr.o,nlm4trace_classes.h,../../))
$(eval $(call XDR_TRACE_CLASS,NLM,clntxdr.o,nlmtrace_classes.h,../../))
$(eval $(call XDR_TRACE_CLASS,NSM,clntxdr.o,nsmtrace_classes.h,../../))
