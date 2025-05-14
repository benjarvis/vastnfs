#!/bin/bash

set -e
set -u

if [[ -d .git ]] && [[ -n "$(git status --porcelain)" ]]; then
	echo "'git status' needs to be clean for build-deb.sh"
	exit -1
fi

ofed=auto
ofaparam=""

while [ "$#" != "0" ] ; do
    if [ "$1" = "--no-ofed" ] ; then
	ofed=
	shift
	continue
    fi
    break
done

if [[ "${ofed}" == "auto" ]] ; then
    OFED_RELEASE=$(ofed_info -s 2>/dev/null | awk -F":" '{print $1}' | tr '-' '.')
    if [[ "$OFED_RELEASE" != "" ]] ; then
	OFED_RELEASE=${OFED_RELEASE}_
    fi
else
    OFED_RELEASE=
fi

if [[ "${OFED_RELEASE}" == "" ]] ; then
    ofaparam="OFA_DIR="
fi

GIT_VERSION=$(./git-version.sh HEAD)
if [[ -e .base-version ]] ; then
	BASE_GIT_VERSION="$(cat .base-version)"
else
	BASE_GIT_VERSION=${GIT_VERSION}
fi
PKG_VERSION=$(echo ${GIT_VERSION} | cut -c2- | tr - .)
VERSION=${PKG_VERSION}
BUILD_AREA=.rpm-build

if [[ -e .git ]] ; then
	rm -rf ${BUILD_AREA}
	mkdir -p ${BUILD_AREA}
	git archive HEAD -o ${BUILD_AREA}/vastnfs_${PKG_VERSION}.orig.tar.gz
else
	tar -czf ../vastnfs_${PKG_VERSION}.orig.tar.gz .git-* *
	rm -rf ${BUILD_AREA}
	mkdir -p ${BUILD_AREA}
	mv ../vastnfs_${PKG_VERSION}.orig.tar.gz ${BUILD_AREA}
fi

cd ${BUILD_AREA}
mkdir source
cd source
tar -zxf ../vastnfs_${PKG_VERSION}.orig.tar.gz
echo '#define NFS_BUNDLE_VERSION "'${VERSION}'"' > build-info.h
VERSION_ID=v_$(echo ${VERSION} | cksum | awk -F' ' '{print $1}')
echo '#define NFS_BUNDLE_VERSION_ID '${VERSION_ID} >> build-info.h
echo '#define NFS_BUNDLE_GIT_VERSION "'${GIT_VERSION}'"' >> build-info.h
echo '#define NFS_BUNDLE_BASE_GIT_VERSION "'${BASE_GIT_VERSION}'"' >> build-info.h
DKMS_VERSION=$(echo ${PKG_VERSION} | sed -E 's/(.*)[.]for[.].*/\1/g' | sed -E 's/v(.*)/\1/g')
sed -i 's/^PACKAGE_VERSION=".*/PACKAGE_VERSION="'${DKMS_VERSION}'"/' dkms.conf
tar -czf ../vastnfs_${PKG_VERSION}.orig.tar.gz *

rm -rf ../../rpm-dist
mkdir ../../rpm-dist

make srcrpm ${ofaparam} RELEASE=${OFED_RELEASE} VERSION=${VERSION}
make binrpm ${ofaparam} RELEASE=${OFED_RELEASE} VERSION=${VERSION}

mv rpm-dist/$(uname -p)/vastnfs-*.rpm ../../rpm-dist
cd ../..
ls -l rpm-dist
