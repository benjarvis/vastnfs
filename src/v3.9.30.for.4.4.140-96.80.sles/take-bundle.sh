#!/bin/bash

mode=$3

set -eux

commit_msg_file=$(pwd)/.commit-msg

linux_source=$1
bundle_base=$2

if [[ -n "$(git status --porcelain)" ]]; then
    echo "nfsrdma; aborting - there are uncommmited changes"
    exit -1
fi

cd $linux_source
if [[ -n "$(git status --porcelain)" ]]; then
    echo "${linux_source}: aborting - there are uncommmited changes"
    exit -1
fi

save_log() {
	unset GIT_DIR
	unset GIT_EXEC_PATH
	unset GIT_PREFIX
	unset GIT_REFLOG_ACTION
	unset GIT_WORK_TREE

	REV=$(git rev-parse HEAD)
	echo "Sync bundle to ${REV}" > ${commit_msg_file}
	echo "Base ${bundle_base}" >> ${commit_msg_file}
	echo "" >> ${commit_msg_file}
	git log --oneline ${bundle_base}.. --no-decorate >> ${commit_msg_file}
}

save_log &
wait

cd - >/dev/null

new_bundle=.tmp-bundle

CP_CMD="cp -a"

if [[ "$mode" == "links" ]] ; then
	CP_CMD="cp -as"
fi

rm -rf ${new_bundle}
mkdir -p ${new_bundle}
mkdir -p ${new_bundle}/net/
if [[ "$mode" == "links" ]] ; then
	ln -s ${linux_source}/net/sunrpc ${new_bundle}/net/
else
	${CP_CMD} ${linux_source}/net/sunrpc ${new_bundle}/net/
fi
mkdir -p ${new_bundle}/include/{linux,trace/events}
if [[ "$mode" == "links" ]] ; then
	ln -s ${linux_source}/include/linux/sunrpc ${new_bundle}/include/linux/sunrpc
else
	${CP_CMD} ${linux_source}/include/linux/sunrpc ${new_bundle}/include/linux/
fi
if [[ "$mode" == "links" ]] ; then
	ln -s ${linux_source}/include/linux/lockd ${new_bundle}/include/linux/lockd
else
	${CP_CMD} ${linux_source}/include/linux/lockd ${new_bundle}/include/linux/
fi
${CP_CMD} ${linux_source}/include/linux/nfs*.h ${new_bundle}/include/linux/
if [[ -e ${linux_source}/include/linux/old-kernel.h ]] ; then
    ${CP_CMD} ${linux_source}/include/linux/old-kernel.h ${new_bundle}/include/linux/
fi
${CP_CMD} ${linux_source}/include/trace/events/{rdma.h,rpcgss.h,rpcrdma.h,sunrpc.h} ${new_bundle}/include/trace/events/
mkdir -p ${new_bundle}/fs/
for subdir in nfs nfsd lockd ; do
    if [[ "$mode" == "links" ]] ; then
       ln -s ${linux_source}/fs/${subdir} ${new_bundle}/fs/
    else
       ${CP_CMD} ${linux_source}/fs/${subdir} ${new_bundle}/fs/
    fi
done
mkdir -p ${new_bundle}/fs/nfs_common
${CP_CMD} ${linux_source}/fs/nfs_common/nfsacl.c ${new_bundle}/fs/nfs_common
${CP_CMD} ${linux_source}/fs/internal.h ${new_bundle}/fs/
cp ${linux_source}/.gitignore ${new_bundle}/

echo '''
obj-y                           += net/ fs/
''' > ${new_bundle}/Makefile

echo '''
obj-$(CONFIG_SUNRPC)            += sunrpc/
''' > ${new_bundle}/net/Makefile

echo '''
obj-$(CONFIG_NFS_FS)            += nfs/
obj-$(CONFIG_NFSD)              += nfsd/
obj-$(CONFIG_NFS_ACL_SUPPORT)   += nfs_common/
obj-$(CONFIG_LOCKD)             += lockd/
''' > ${new_bundle}/fs/Makefile


echo '''
obj-$(CONFIG_NFS_ACL_SUPPORT) += nfs_acl.o
nfs_acl-objs := nfsacl.o
''' > ${new_bundle}/fs/nfs_common/Makefile

rm -rf bundle
mv ${new_bundle} bundle

if [[ "$mode" == "stg" ]] ; then
	stg refresh
	stg edit -f ${commit_msg_file}
elif [[ "$mode" == "git" ]] ; then
	git add bundle/
	git commit -F ${commit_msg_file}
elif [[ "$mode" == "links" ]] ; then
	echo "Not committing links"
fi

rm -f ${commit_msg_file}
