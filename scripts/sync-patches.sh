#!/bin/bash

set -eu
set -o pipefail
set +o posix

main() {
    local kernel_repo=""
    local sync_target=""
    local show=0
    local base
    local cherry_pick=""

    base=$(cat $(dirname ${BASH_SOURCE})/BASE | head -n 1)

    while [[ $# != 0 ]] ; do
	if [[ "$1" == "--kernel-repo" ]] ; then
	    shift
	    kernel_repo=$1
	    shift
            continue
        elif [[ "$1" == "--sync-target" ]] ; then
	    shift
	    sync_target=$1
	    shift
	    continue
        elif [[ "$1" == "--show" ]] ; then
	    show=1
	    shift
	    continue
        elif [[ "$1" == "--cherry-pick" ]] ; then
	    shift
	    cherry_pick="$1"
	    shift
	    continue
	fi
	break # Continue to positional arguments
    done

    if [[ "${kernel_repo}" == "" ]] ; then
	echo "Please pass --kernel-repo"
	exit -1
    fi

    if [[ "${cherry_pick}" == "" ]] ; then
	if [[ "${sync_target}" == "" ]] ; then
	    echo "Neither --cherry-picke pasesed nor --sync-target"
	    exit -1
	fi
    fi

    if [[ "${show}" == "0" ]] ; then
	if [[ -n "$(git status --porcelain)" ]]; then
	    echo "Aborting - there are uncommmited changes"
	    echo
	    git status
	    exit -1
	fi
    fi

    local tmp_dir=$(mktemp -d -t vastnfs-sync-patches-XXXXXXXXXX)

    files=(fs/nfs/ fs/nfsd/ fs/nfs_common fs/lockd net/sunrpc \
	include/linux/lockd/* \
	include/linux/nfs* include/linux/sunrpc/ \
	include/trace/events/sunrpc.h include/trace/events/rpcrdma.h \
	include/trace/events/rpcgss.h)

    local range

    if [[ "${cherry_pick}" == "" ]] ; then
	range="${base}..${sync_target}"
    else
	range="${cherry_pick}~1..${cherry_pick}"
    fi

    echo Checking ${range} for "${files[@]}"
    git --git-dir ${kernel_repo}/.git format-patch -q ${range} -o ${tmp_dir} -- \
	"${files[@]}"

    local save=$(git rev-parse HEAD)
    local fails=0

    set +e
    git log --pretty="%b" | \
	grep -E '^[(]cherry-picked from commit [a-f0-9]+[)]$' > ${tmp_dir}/cherry-picked.txt
    cat scripts/BASE | grep ^skipped-commit >> ${tmp_dir}/skipped.txt
    set -e

    local nr_patches=0
    for i in $(ls ${tmp_dir}/*.patch 2> /dev/null) ; do
	echo $(basename ${i})
	local commithash=$(cat ${i} | head -n 1 | awk '{print $2}')

	if [[ ! -n ${commithash} ]] ; then
	    echo "error obtaining commit hash from ${i}"
	    exit -1
	fi

	if grep -q ${commithash} ${tmp_dir}/cherry-picked.txt ; then
	    echo "Already applied"
	    continue
	fi

	if grep -q ${commithash} ${tmp_dir}/skipped.txt ; then
	    echo "Skipped"
	    continue
	fi

	nr_patches=$((${nr_patches} + 1))

	cat ${i} \
	   | awk '{if ($0 !~ /^[[:space:]]*$/) {print} else {exit} }' \
	    > ${tmp_dir}/patch.txt
	echo "" >> ${tmp_dir}/patch.txt
	echo "(cherry-picked from commit ${commithash})" >> ${tmp_dir}/patch.txt
	cat ${i} | \
	   awk '{if ($0 ~ /^[[:space:]]*$/) {a=1}; if (a) {print} }' \
	   >> ${tmp_dir}/patch.txt

	local e=0
	if [[ "${show}" == "0" ]] ; then
	    set +e
	    cat ${tmp_dir}/patch.txt | git am --directory=bundle/
	    local e=$?
	    set -e
	fi

	if [[ $e != 0 ]] ; then
	    echo "Error applying patch ${commithash}"
	    echo
	    echo -e "\033[1;37mApply it manually using\033[0m:"
	    echo
	    echo "	git am --show-current-patch | (cd bundle && patch -p1)"
	    echo
	    echo "To commit, 'git add' the necessary changes and 'git am --continue'".
	    echo
	    fails=1
	    break
	fi
    done

    if [[ ${fails} == "1" ]] ; then
	echo "Please perform conflict resolution. "
	echo
	echo -e "\033[1;33mWe are currently in 'git am' mode!\033[0m"
    fi

    rm -rf ${tmp_dir}
}

describe-applied() {
    local kernel_repo=""
    local sync_target=""
    local show=0
    local base=$(cat $(dirname ${BASH_SOURCE})/BASE | head -n 1)

    while [[ $# != 0 ]] ; do
	if [[ "$1" == "--kernel-repo" ]] ; then
	    shift
	    kernel_repo=$1
	    shift
            continue
	fi
	break # Continue to positional arguments
    done

    if [[ "${kernel_repo}" == "" ]] ; then
	echo "Please pass --kernel-repo"
	exit -1
    fi

    commit=""
    while read a b c d ; do
	if [[ "$a" == "(cherry-picked" ]] ; then
	    orig_commit=$(echo ${d} | tr -d \))
	    contained=$(cd ${kernel_repo} && git describe --contains ${orig_commit})
	    printf "%15s | " ${contained}
	    echo $(cd ${kernel_repo} && git log --abbrev-commit --abbrev=12 \
		--date=format:%Y-%m-%d \
    	        --format="%cd | %h | %s" -n 1 ${orig_commit});
	else
	    commit=${b}
	fi
    done < <(git log --pretty=fuller | \
	grep -E '^((commit.*)|( *[(]cherry-picked from commit [a-f0-9]+[)]))$')

}

if [[ "$1" == "describe-applied" ]] ; then
    shift
    describe-applied "$@"
    exit 0
fi

main "$@"
