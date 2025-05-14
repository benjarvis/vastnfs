obj-$(CONFIG_NFS_ACL_SUPPORT) += nfs_acl.o
nfs_acl-objs := nfsacl.o

# Because nfs_ssc may be builtin in the target kernel, it is better to have
# a separate name for it.
obj-$(CONFIG_NFS_V4_2_SSC_HELPER) += compat_nfs_ssc.o
compat_nfs_ssc-objs := nfs_ssc.o
