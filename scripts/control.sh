#!/bin/bash

set -eux

inbox=0

mod-load() {
    local base=$(basename $1)
    if [[ "${inbox}" == "1" ]] ; then
	sudo modprobe ${base%.ko}
    else
	if [[ "${base}" == "sunrpc.ko" ]] ; then
	    for dep in $(modinfo $1 | grep depends: | awk -F' ' '{print $2}') ; do
		echo modprobe ${dep}
		sudo modprobe ${dep}
	    done
	fi

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
    sudo modprobe fscache || true
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
    mod-load bundle/fs/nfs_common/compat_nfs_ssc.ko
    mod-load bundle/fs/lockd/lockd.ko
    mod-load bundle/fs/nfs/nfs.ko
    mod-load bundle/fs/nfs/nfsv3.ko
    mod-load bundle/fs/nfs/nfsv4.ko
    mod-load bundle/fs/nfsd/nfsd.ko

    sleep 0.5

    # Reinstate various services

    if [[ -x /var/lib/nfs/rpc_pipefs ]] ; then
	sudo mount -t rpc_pipefs sunrpc /var/lib/nfs/rpc_pipefs
    fi

    sudo systemctl start rpcbind.service 2>/dev/null
    sudo systemctl start rpc-statd.service 2>/dev/null
    sudo systemctl start rpc-gssd.service 2>/dev/null || true

    if ! dmesg | grep -q "nfsd stub loaded" ; then
	sudo systemctl start nfs-server 2>/dev/null
    fi

    echo Loaded.
}

eager-umount() {
    local path_to_umount
    local t=0

    path_to_umount="$1"

    while cat /proc/mounts | grep -qE '^[^ ]+ '${path_to_umount}' ' ; do
	if [[ "$t" != 0 ]] ; then
	    sleep 0.5
	fi
	t=$(( $t + 1))
	sudo umount ${path_to_umount} 2>/dev/null || true
    done
}

unload() {
    set +e

    sudo umount -a -t nfs4
    sudo umount -a -t nfs

    # NFSD stuff
    sudo systemctl stop nfs-server 2>/dev/null
    sudo systemctl stop rpc-gssd.service 2>/dev/null
    sudo systemctl stop rpcbind.socket 2>/dev/null
    sudo pkill -f /usr/sbin/nfsdcld
    eager-umount /proc/fs/nfsd
    eager-umount /run/rpc_pipefs
    eager-umount /var/lib/nfs/rpc_pipefs

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
    mod-unload compat_nfs_ssc

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
