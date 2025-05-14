#!/bin/bash

set -eu
set -o pipefail
set +o posix

# Set VASTNFS_CI_DRIVER_OPS_GITREF to the desired driver-ops gitref.
# otherwise we take latest origin/master, which should know how to
# test our older versions too.

header() {
    local section=$1
    shift
    echo -e "\e[0Ksection_start:`date +%s`:${section}\r\e[0K$@"
}

footer() {
    local section=$1
    shift
    echo -e "\e[0Ksection_end:`date +%s`:${section}\r\e[0K"
}

log() {
    echo "$@"
}

git-remote-rev-parse() {
    # Resolve branches on a remote, or it may also be a local
    # tag or branch.
    if git rev-parse origin/${1} 2>/dev/null >/dev/null ; then
	git rev-parse origin/${1}
	return
    fi
    if git rev-parse ${1} 2>/dev/null >/dev/null ; then
	git rev-parse ${1}
	return
    fi
}

drivers-ops-prep() {
    header driver-ops-build "Checking driver-ops build"

    cd ~/workspace
    mkdir -p ~/workspace/cache

    local giturl="https://gitlab-ci-token:${CI_JOB_TOKEN}@${CI_SERVER_HOST}/${DRIVER_OPS_REPO_SUBURL}"
    if [[ ! -x driver-ops ]] ; then
	git clone --recursive ${giturl} driver-ops.tmp
	mv driver-ops.tmp driver-ops
	cd driver-ops
    else
	cd driver-ops
	git remote set-url origin ${giturl}

    fi

    git fetch

    local r=${VASTNFS_CI_OPS_GITREF:-master}
    echo "Driver ops ref ${r}"
    local v=$(git-remote-rev-parse ${r})

    if [[ "${v}" == "" ]] ; then
	echo "Unable to resolve ${r}"
	exit -1
    fi

    if [[ ! -d ~/workspace/cache/${v} ]] ; then
	log "Driver ops ${v} is building"

	git reset --hard ${v}
	git submodule update --init
	rm -f ~/workspace/cache/${v}.tmp
	./helpers/pack-ops ~/workspace/cache/${v}.tmp
	mv ~/workspace/cache/${v}.tmp ~/workspace/cache/${v}
    else
	log "Driver ops ${v} cached build found"
    fi

    echo ${HOME}/workspace/cache/${v}/bin > ${CI_TEST_DIR}/ops_bin_path
    footer driver-ops-build
}

main() {
    local repopath=$(pwd)
    local scriptpath=$(realpath ${BASH_SOURCE})

    cd ~/workspace
    mkdir -p ~/workspace

    local tmp_dir=$(mktemp -d -t ci-test-XXXXXXXXXX)
    export CI_TEST_DIR=${tmp_dir}
    set +e
    flock ~/workspace ${scriptpath} drivers-ops-prep
    local e=$?
    set -e
    if [[ -e ${CI_TEST_DIR}/ops_bin_path ]] ; then
	local ops_bin_path=$(cat ${CI_TEST_DIR}/ops_bin_path)
    else
	local ops_bin_path=""
    fi
    rm -rf ${tmp_dir}

    if [[ "${e}" != "0" ]] ; then
	exit ${e}
    fi
    if [[ ! -e ${ops_bin_path} ]] ; then
	exit -1
    fi

    export PATH=${ops_bin_path}:$PATH
    cd "${repopath}"
    export KERNEL_OPS_RESOURCES_ROOT_PATH=~/workspace
    exec ops ci-entry
}

if [[ "$@" == "" ]] ; then
    main
else
    "$@"
fi
