# vastnfs-ctl

This helper script is installed with the package, and can assist in various
situations.

The usage of this script is optional in most cases.


## Status

Observe the status of the loaded NFS services and kernel modules

`vastnfs-ctl status`

Example output:

```
version: v4.0-pre4-11-ge520dd8eb9cb
kernel modules: sunrpc rpcrdma compat_nfs_ssc lockd nfs_acl auth_rpcgss nfsd rpcsec_gss_krb5 nfs nfsv3 nfsv4
services: rpcbind.socket rpcbind
rpc_pipefs: /var/lib/nfs/rpc_pipefs
```

## Reload

The following command will try to reload the NFS stack. It will first try to
unmount all NFS file systems, and then reload all the necessary kernel modules.

`vastnfs-ctl reload`

Example output:

```
vastnfs-ctl: stopping service rpcbind.socket
vastnfs-ctl: umounting fs /var/lib/nfs/rpc_pipefs
vastnfs-ctl: unloading kmod nfsv4
vastnfs-ctl: unloading kmod nfsv3
vastnfs-ctl: unloading kmod nfs
vastnfs-ctl: unloading kmod rpcsec_gss_krb5
vastnfs-ctl: unloading kmod nfsd
vastnfs-ctl: unloading kmod auth_rpcgss
vastnfs-ctl: unloading kmod nfs_acl
vastnfs-ctl: unloading kmod lockd
vastnfs-ctl: unloading kmod compat_nfs_ssc
vastnfs-ctl: unloading kmod rpcrdma
vastnfs-ctl: unloading kmod sunrpc
vastnfs-ctl: loading kmod sunrpc
vastnfs-ctl: loading kmod rpcsec_gss_krb5
vastnfs-ctl: loading kmod rpcrdma
vastnfs-ctl: loading kmod nfsv4
vastnfs-ctl: loading kmod nfsv3
vastnfs-ctl: loading kmod nfsd
vastnfs-ctl: loading kmod nfs_acl
vastnfs-ctl: loading kmod nfs
vastnfs-ctl: loading kmod lockd
vastnfs-ctl: loading kmod compat_nfs_ssc
vastnfs-ctl: loading kmod auth_rpcgss
vastnfs-ctl: mounting fs /var/lib/nfs/rpc_pipefs
vastnfs-ctl: starting service rpcbind.socket
```

## Tracing

The `vastnfs-ctl` utility comes with a helper command for tracing the NFS
stack. Please see a [short tutorial on using vastnfs-ctl trace](../monitoring/tracing.md).
