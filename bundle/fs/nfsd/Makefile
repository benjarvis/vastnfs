# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the Linux nfs server
#

ccflags-y += -I$(src)			# needed for trace events

obj-$(CONFIG_NFSD)	+= nfsd.o

ifeq ($(COMPAT_DETECT_IDR_GET_NEXT_UL)$(COMPAT_DETECT_IOV_ITER_DISCARD),yy)
# this one should be compiled first, as the tracing macros can easily blow up
nfsd-y			+= trace.o

nfsd-y 			+= nfssvc.o nfsctl.o nfsproc.o nfsfh.o vfs.o \
			   export.o auth.o lockd.o nfscache.o nfsxdr.o \
			   stats.o filecache.o
nfsd-$(CONFIG_NFSD_V2_ACL) += nfs2acl.o
nfsd-$(CONFIG_NFSD_V3)	+= nfs3proc.o nfs3xdr.o
nfsd-$(CONFIG_NFSD_V3_ACL) += nfs3acl.o
nfsd-$(CONFIG_NFSD_V4)	+= nfs4proc.o nfs4xdr.o nfs4state.o nfs4idmap.o \
			   nfs4acl.o nfs4callback.o nfs4recover.o
nfsd-$(CONFIG_NFSD_PNFS) += nfs4layouts.o
nfsd-$(CONFIG_NFSD_BLOCKLAYOUT) += blocklayout.o blocklayoutxdr.o
nfsd-$(CONFIG_NFSD_SCSILAYOUT) += blocklayout.o blocklayoutxdr.o
nfsd-$(CONFIG_NFSD_FLEXFILELAYOUT) += flexfilelayout.o flexfilelayoutxdr.o
nfsd-$(CONFIG_NFSD_NEW_FAULT_INJECTION) += fault_inject.o

else
nfsd-y += stub.o
endif
