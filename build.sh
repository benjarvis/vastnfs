#!/bin/bash

set +o posix # So that /bin/sh <script> works

declare -A KVER2TAG

described_tags=0
if [[ -e tags ]] ; then
    source tags
    described_tags=1
else
    declare -A DESCRIBED_TAGS
fi

# Version of the source package
VASTNFS_VERSION=$(./git-version.sh HEAD)
DOCS_VERSION=4.0
proc=$(uname -p)

### Legacy kernel branches (these will build from the respective tags)
LEGACY_VERSION=3.9.30

# CentOS 7.x
KVER2TAG[3.10.0-693.el7.${proc}.centos]=v${LEGACY_VERSION}.for.3.10.0-693.21.1.centos
KVER2TAG[3.10.0-862.el7.${proc}.centos]=v${LEGACY_VERSION}.for.3.10.0-862.14.4.centos
KVER2TAG[3.10.0-957.27.2.el7.${proc}.centos]=v${LEGACY_VERSION}.for.3.10.0-957.27.2.centos
KVER2TAG[3.10.0-957.el7.${proc}.centos]=v${LEGACY_VERSION}.for.3.10.0-957.10.1.centos
KVER2TAG[3.10.0-1062.el7.${proc}.centos]=v${LEGACY_VERSION}.for.3.10.0-1062.12.1.centos
KVER2TAG[3.10.0-1127.el7.${proc}.centos]=v${LEGACY_VERSION}.for.3.10.0-1127.centos
KVER2TAG[3.10.0-1127.13.1.el7.${proc}.centos]=v${LEGACY_VERSION}.for.3.10.0-1127.19.1.centos
KVER2TAG[3.10.0-1160.el7.${proc}.centos]=v${LEGACY_VERSION}.for.3.10.0-1160.15.2.centos
KVER2TAG[4.14.0-115.el7a.${proc}.centos]=v${LEGACY_VERSION}.for.4.14.0-115.33.1.centos

# 1chaos kernels
KVER2TAG[3.10.0-1127.18.2.1chaos.el7.${proc}.chaos-centos]=v${LEGACY_VERSION}.for.3.10.0-1127.19.1.chaos.centos
KVER2TAG[3.10.0-1160.21.1.1chaos.ch6.${proc}.chaos-centos]=v${LEGACY_VERSION}.for.3.10.0-1160.15.2.chaos.centos

# SLES 12
KVER2TAG[4.4.140-96.80-default.sles]=v${LEGACY_VERSION}.for.4.4.140-96.80.sles
KVER2TAG[4.12.14-122.37-default.sles]=v${LEGACY_VERSION}.for.4.12.14-122.37.sles
KVER2TAG[4.12.14-122.66-default.sles]=v${LEGACY_VERSION}.for.4.12.14-122.66.sles

# SLES 15
KVER2TAG[4.12.14-lp150.12.82-default.sles]=v${LEGACY_VERSION}.for.4.12.14-lp150.12.82.sles

### Backport branch (these will build from the current branch)

KVER2TAG[4.15.0.ubuntu]=HEAD
KVER2TAG[4.15.0.centos]=HEAD
KVER2TAG[4.15.0.sles]=HEAD
KVER2TAG[4.15.0.centos]=HEAD

TAGS=()

_collect_tags() {
	declare -A UNSORTED_TAGS
	declare -A EXISTING_TAGS

	if [[ "${described_tags}" == "0" ]] ; then
		while read -r tag ; do
			EXISTING_TAGS[${tag}]=1
		done < <(git tag)
	fi

	for key in ${!KVER2TAG[@]} ; do
  	        local tag=${KVER2TAG[${key}]}
		if [[ "${tag}" == "HEAD" ]] ; then
		    continue
		fi

		if [[ "${described_tags}" == "0" ]] ; then
		    # Resolve all versions to tags if they are not tags
		    if [[ ${EXISTING_TAGS[${tag}]} == "" ]] ; then
			git describe ${tag} 2>/dev/null >/dev/null
			if [[ "$?" != "0" ]] ; then
		            echo "Git ref not found: ${tag}"
			    exit -1
		        fi
		        local described=$(git describe ${tag})
		        if [[ ${described} != ${tag} ]] ; then
		   	    DESCRIBED_TAGS[${tag}]=${described}
			    KVER2TAG[${key}]=${described}
		        fi
		    fi
		else
		    # Load resolved Git tags
		    local described=${DESCRIBED_TAGS[${tag}]}
		    if [[ ${described} != "" ]] ; then
			KVER2TAG[${key}]=${described}
		    fi
		fi

		UNSORTED_TAGS[${KVER2TAG[${key}]}]=1
	done

	while read -r tag ; do
		TAGS+=($tag)
	done < <(echo ${!UNSORTED_TAGS[@]} | tr ' ' '\n' | sort -V)
}
_collect_tags

_match_closest_version() {
	# Convert an arbitrary version string to a version that is contained
	# in KVER2TAG, based sorting the versions in the array and finding the
	# closest one from the past.

	local kver=$1
	local distro=$2

	local candidates=($(echo ${!KVER2TAG[@]} ${kver}.${distro} | tr ' ' '\n' | grep -E '[.]'${distro}'$' |\
		awk '{split($0,a,/[.-]/); print sprintf("%05d.%05d.%05d.%05d.%05d.%05d.%05d %s", int(a[1]), int(a[2]), int(a[3]), int(a[4]), int(a[5]), int(a[6]), int(a[7]), $0)}' | sort -n | grep ${kver}.${distro} -B 1 | awk -F" " '{print $2}'))

	if ((${#candidates[@]} == 2)) ; then
		# Not found, take previous
		echo ${candidates[0]}
	else
		# Found
		echo ${kver}.${distro}
	fi
}

_mk_short_tag() {
	local dist=$(echo $1 | awk -F. '{print $NF}')
	echo $(echo $1 | sed -E 's/(.*)-[^-]*$/\1/g').$dist
}

_is_git_repo() {
	unset GIT_DIR

	GDIR=$(git rev-parse --git-dir 2>/dev/null)
	if [[ "$GDIR" == "" ]] ; then
		return 1
	fi

	return 0
}

src() {
	if ! _is_git_repo ; then
		echo "Only supported on a git repo "
		exit -1
	fi

	if ! git rev-parse --show-toplevel >/dev/null 2>/dev/null ; then
		echo "Not a git repository (or Git not installed)"
		exit -1
	fi

	if [[ -n "$(git status --porcelain)" ]]; then
		echo "'git status' needs to be clean for build-src.sh"
		exit -1
	fi

	set -e

	RDIR=$(mktemp -d -t vastnfs-buildsh-XXXXXXXXXX)
	NAME=vastnfs-${VASTNFS_VERSION#v}

	echo Stand by - creating ${NAME}.tar.xz from Git repository versions...

	TDIR=${RDIR}/${NAME}

	rm -rf src-dist
	mkdir src-dist

	mkdir ${TDIR}
	git archive --format=tar HEAD | tar -xf - -C ${TDIR}

	mkdir -p ${TDIR}/src
	local prev_tag=""
	local prev_short_tag=""
	for tag in ${TAGS[@]} ; do
		short_tag=$(_mk_short_tag $tag)
		if [[ "${prev_short_tag}" != "${short_tag}" ]] ; then
			git archive --format=tar --prefix=src/${tag}/ ${tag} |\
				tar -xf - -C ${TDIR}
		else
			git diff ${prev_tag}..${tag} > ${TDIR}/src/${tag}.diff
		fi
		prev_tag=${tag}
		prev_short_tag=${short_tag}

		echo ${tag} > ${TDIR}/src/${tag}.version
	done

	DF=$(realpath src-dist)/${NAME}.tar.xz

	echo 'declare -A DESCRIBED_TAGS' > ${TDIR}/tags
	for key in ${!DESCRIBED_TAGS[@]} ; do
		local described=${DESCRIBED_TAGS[${key}]}
		echo 'DESCRIBED_TAGS['${key}']='$described >> ${TDIR}/tags
	done

	echo """
# VAST NFS source

This is a source package for VAST NFS, that can build based on kernel and
distribution detection. Multiple kernels are supported by this package.

See [Change Log](docs/src/ChangeLog.md).

See [INSTALL](INSTALL.md) file for list of supported kernels and instructions
relevant to execution on client machines.


## Binary package

To build a binary from this source pacakge for the current kernel and OFED
versions, run \`./build.sh bin\`, and take the output from the \`dist\` directory.
If the kernel, distribution, or OFED are not supported, an error will be
printed.

   $ ./build.sh bin

   Output in dist/

   total 760
   -rw-r--r-- 1 user user 777512 Jul 20 14:25 vastnfs-modules_3.3.for.5.3.0.53.ubuntu-vastdata-5.0-OFED.5.0.2.1.8.1.kver.5.3.0-53-generic_amd64.deb
	""" > ${TDIR}/README.md
	./git-version.sh HEAD save ${TDIR}/.git-version-save
	tar -cf - -C ${RDIR} ${NAME} | xz -T 16 -9 > ${DF}

	echo
	echo "Output in src-dist directory:"
	echo
	ls -l src-dist

	rm -rf ${TDIR}
	rmdir ${RDIR}
}

check_missing_packages_centos() {
	local current_kernel=$1
	local PACKAGES=()
	if [[ "$(which rpmbuild 2> /dev/null)" == "" ]] ; then
		PACKAGES+=(rpm-build)
	fi
	if [[ "$current_kernel" == "1" ]] && [[ ! -d /lib/modules/$(uname -r)/build ]] ; then
		PACKAGES+=(kernel-devel-`uname -r`)
	fi

	if [[ "${#PACKAGES[@]}" != "0" ]] ; then
		echo "Some dependencies are missing, please run:"
		echo
		echo "sudo yum install -y ${PACKAGES[@]}"
		exit -1
	fi
}

bin() {
	source /etc/os-release

	local script
	local only_extract=0
	local build_params=("")
	local base_kver=""
	local stage_git_version=""

	while [[ "$#" != "0" ]] ; do
	     if [[ "$1" == "--base-kver" ]] ; then
	     	 shift
		 if [[ "$#" == "0" ]] ; then
		     echo 'error: missing parameter to `--base-kver`'
		     exit -1
		 fi
	      	 base_kver=$1
		 shift
	      	 continue
	     elif [[ "$1" == "--only-extract" ]] ; then
	      	 only_extract=1
	     	 shift
	      	 continue
	     elif [[ "$1" == "--without-rpcrdma" ]] ; then
		 export CONFIG_SUNRPC_XPRT_RDMA=n
	     	 shift
	      	 continue
	     elif [[ "$1" == "--dkms" ]] ; then
	         local build_params=(${build_params[@]} "--dkms")
	     	 shift
	      	 continue
	     elif [[ "$1" == "--no-dkms" ]] ; then
	         local build_params=(${build_params[@]} "--no-dkms")
	     	 shift
	      	 continue
             elif [[ "$1" == "--stage-git-version" ]] ; then
                 shift
                 stage_git_version=$1
                 shift
                 continue
	     elif [[ "$1" == "--no-ofed" ]] ; then
	         local build_params=(${build_params[@]} "--no-ofed")
	     	 shift
	      	 continue
	     else
	     	 echo "Invalid parameter: "$1
		 exit -1
	     fi
	     break
	done

	local kver=${base_kver:-${KVER:-`uname -r`}}
	local current_kernel=0
	if [[ "$kver" == `uname -r` ]] ; then
		current_kernel=1
	fi

	if [[ "$ID_OVERRIDE" != "" ]] ; then
		ID=${ID_OVERRIDE}
	fi

	if [[ "$ID" == "ubuntu" ]] ; then
		script=build-deb.sh
	elif [[ "$ID" == "centos" ]] || [[ "$ID" == "ol" ]] || [[ "$ID" == "rocky" ]] || [[ "$ID" == "scientific" ]] || [[ "$ID" == "sl" ]] || [[ "$ID" == "rhel" ]] || [[ "$ID" == "almalinux" ]]; then
		check_missing_packages_centos ${current_kernel}
		ID=centos
		if [[ "$ID_OVERRIDE" == "" ]] ; then
			if [[ $(grep '.1chaos.' ${kver} 2>/dev/null) != "" ]] ; then
				ID=chaos-${ID}
			fi
		fi
		script=build-rpm.sh
	elif [[ "$ID" == "sles" ]] ; then
		script=build-rpm.sh
	elif [[ "$ID" == "debian" ]] ; then
		script=build-deb.sh
		ID=ubuntu
	elif [[ "$ID" == "opensuse" ]] ; then
		script=build-rpm.sh
		ID=sles
	elif [[ "$ID" == "opensuse-leap" ]] ; then
		script=build-rpm.sh
		ID=sles
	else
		echo 'The Linux distribution of type '${ID}' not supported.'
		exit -1
	fi

	local key=${kver}.${ID}
	echo "Input kernel version key: ${key}"
	local key_match=$(_match_closest_version ${kver} ${ID})
	echo "Closest kernel version key match: ${key_match}"
	local tag=${KVER2TAG[${key_match}]}
	echo "Matched source tag: ${tag}"
	echo "Building for kernel: ${kver} (to override, use KVER environment)"

	if [[ "${tag}" == "" ]] ; then
		echo "Kernel ${kver} not supported on distribution ${ID}."
		exit -1
	fi

	echo "Preparing source for kernel version"

	set -e
	set -u
	set -o pipefail

	rm -rf build/
	local build_dir_prefix=build/

	if [[ -e src/${tag}.version ]] ; then
		echo "Building from source and patches"
		local short_tag=$(_mk_short_tag $tag)
		local first=1

		for iter_tag in ${TAGS[@]} ; do
			iter_short_tag=$(_mk_short_tag $iter_tag)
			if [[ "${short_tag}" != ${iter_short_tag} ]] ; then
				continue
			fi

			if [[ ${first} == 1 ]] ; then
				cp -a src/${iter_tag}/ build/
			else
				cat src/${iter_tag}.diff | \
					(cd build && patch -p1 > /dev/null)
				# Apply new file modes of (git-specific format)
				cat src/${iter_tag}.diff | (grep '^new file mode' -B1 || true) \
 				        | (grep -v ^-- || true) | sed -zE 's%\nnew file mode 10% %g' | \
					awk -F" " '{print "chmod " $5 " build/" substr($4, 3)}' | \
					bash
			fi

			if [[ "${iter_tag}" == "${tag}" ]] ; then
				break
			fi
			first=0
		done
		cp src/${tag}.version build/.git-version-save
	elif [[ -e .git-version-save ]] ; then
		build_dir_prefix=
	else
		echo "Taking version from Git"

                if [[ ${stage_git_version} != "" ]] ; then
                        tag=$(git describe ${stage_git_version})
                        echo "Redirecting to ${tag}"
                fi

		git archive --format=tar --prefix=build/ ${tag} | tar -xf -
		if [[ "${tag}" == "HEAD" ]] ; then
			./git-version.sh HEAD save build/.git-version-save
		else
			echo ${tag} > build/.git-version-save
		fi
	fi

	echo ${VASTNFS_VERSION} > ${build_dir_prefix}.base-version

	if [[ ! -x build/docs ]] ; then
		cp -a docs/ build/
	fi

	if [[ "$only_extract" == "0" ]] ; then
		if [[ "${build_dir_prefix}" != "" ]] ; then
			cd ${build_dir_prefix}
		fi

		set +e
		bash ./${script} "${build_params[@]}"
		local e=$?
		set -e

		if [[ "${e}" != "0" ]] ; then
			echo
			echo "Input kernel version key: ${key}"
			echo "Closest kernel version key match: ${key_match}"
			echo "Matched source tag: ${tag}"
			echo "Built for kernel: ${kver} (to override, use KVER environment)"
			echo
			echo Aborting due to build error
			exit ${e}
		fi

		if [[ "${build_dir_prefix}" != "" ]] ; then
			cd ..
		fi
	else
		exit 0
	fi

	if [[ "$script" == "build-deb.sh" ]] ; then
		rm -rf dist/
		mv ${build_dir_prefix}deb-dist dist/
	elif [[ "$script" == "build-rpm.sh" ]] ; then
		rm -rf dist/
		mv ${build_dir_prefix}rpm-dist dist/
	fi

	echo "------------------------------------------------------------------"
	echo
	echo "Output in dist/"
	echo

	ls -l dist/
}

mdbook-wrap() {
	local SAVEPATH=$PATH
	export PATH=$(pwd)/docs/progs:$PATH
	DOCS_VERSION=${DOCS_VERSION} GENDATE=$(date +'%Y.%m.%d %H:%M') VASTNFS_VERSION=${VASTNFS_VERSION} mdbook "$@"
	export PATH=${SAVEPATH}
}

serve-docs() {
	mdbook-wrap serve docs
}

prep-docs() {
	set -eu
	local branch="$1"
	local output_dir="$2"

	if [[ "${branch}" != "master" ]] ; then
		export VASTNFS_DOCS_SIDE_BRANCH=", branch **${branch}**"
	fi

	local tempdir=$(mktemp -d -t vastnfs-buildsh-XXXXXXXXXX)
	local abs_output_dir=$(realpath ${output_dir})
	export PATH=$(pwd)/docs/progs:$PATH
	mkdir -p ${abs_output_dir}
	mdbook-wrap build docs --dest-dir "${tempdir}"
	cp -a ${tempdir}/html/* ${abs_output_dir}/
	rm -rf ${tempdir}
}

help() {
	echo "Syntax:"
	echo ""
	echo "   ./build.sh bin"
	echo "        Build a package for the current kernel"

	if _is_git_repo ; then
		echo ""
		echo "   ./build.sh src"
		echo "        Build a multi-kernel source package current current Git repository state"
	fi

}

if [[ "$@" == "" ]] ; then
	help
else
	"$@"
fi
