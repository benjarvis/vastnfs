#!/bin/bash

set -eux

inbox=0

mod-load() {
    if [[ "${inbox}" == "1" ]] ; then
	local base=$(basename $1)
	sudo modprobe ${base%.ko}
    else
	sudo insmod $1
    fi
}

mod-unload() {
    sudo rmmod $1
}

load() {
    mod-load ./bundle/net/sunrpc/sunrpc.ko

    sudo modprobe rdma_cm
    sudo modprobe grace
    sudo modprobe fscache
    sudo modprobe dns_resolver

    if [[ "${inbox}" == "1" ]] ; then
	set +e
	mod-load bundle/net/sunrpc/xprtrdma/rpcrdma.ko
	set -e
    else
	mod-load bundle/net/sunrpc/xprtrdma/rpcrdma.ko
    fi

    mod-load bundle/net/sunrpc/auth_gss/auth_rpcgss.ko
    mod-load bundle/net/sunrpc/auth_gss/rpcsec_gss_krb5.ko
    mod-load bundle/fs/nfs_common/nfs_acl.ko
    mod-load bundle/fs/lockd/lockd.ko
    mod-load bundle/fs/nfs/nfs.ko
    mod-load bundle/fs/nfs/nfsv3.ko
    mod-load bundle/fs/nfs/nfsv4.ko
    mod-load bundle/fs/nfsd/nfsd.ko

    sleep 0.5

    sudo systemctl start nfs-server 2>/dev/null

    echo Loaded.
}

unload() {
    set +e

    # NFSD stuff
    sudo systemctl stop nfs-server 2>/dev/null
    sudo umount /proc/fs/nfsd 2>/dev/null
    sudo umount /run/rpc_pipefs 2>/dev/null
    sudo umount /var/lib/nfs/rpc_pipefs 2>/dev/null

    # Modules
    mod-unload rpcrdma
    mod-unload nfsv4
    mod-unload nfsv3
    mod-unload nfs
    mod-unload rpcsec_gss_krb5
    mod-unload nfsd
    mod-unload auth_rpcgss
    mod-unload nfs_acl
    mod-unload lockd

    # Safe sunrpc removal
    if [[ -d /sys/module/sunrpc ]] ; then
	sudo bash -c "echo 2 > /proc/sys/vm/drop_caches"
	sleep 1
	while [ -e /sys/module/sunrpc ] ; do
	    mod-unload sunrpc 2>/dev/null
	    if [[ "$?" == "0" ]] ; then
		break
	    else
		sleep 1
	    fi
	done
    fi
    set -e
}

dev-reload() {
    unload
    load
}

if [[ "$1" == "inbox" ]] ; then
    inbox=1
    shift
fi

"$@"
