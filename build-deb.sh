#!/bin/bash

set -e
set -u

dkms=0

if [[ -d .git ]] && [[ -n "$(git status --porcelain)" ]]; then
	echo "'git status' needs to be clean for build-deb.sh"
	exit -1
fi

ofed=auto

while [ "$#" != "0" ] ; do
    if [ "$1" = "--no-ofed" ] ; then
	ofed=
	shift
	continue
    fi
    if [ "$1" = "--dkms" ] ; then
	dkms=1
	shift
	continue
    fi
    break
done

if [[ "${ofed}" == "auto" ]] ; then
    OFED_RELEASE=$(ofed_info -s 2>/dev/null | awk -F":" '{print $1}' | tr '_' '.')
    if [[ "$OFED_RELEASE" != "" ]] ; then
	OFED_RELEASE=-${OFED_RELEASE}
    fi
else
    OFED_RELEASE=
fi

GIT_VERSION=$(./git-version.sh HEAD)
if [[ -e .base-version ]] ; then
	BASE_GIT_VERSION="$(cat .base-version)"
else
	BASE_GIT_VERSION=${GIT_VERSION}
fi
PKG_VERSION=$(echo ${GIT_VERSION} | cut -c2- | tr - .)

if [[ "${OFED_RELEASE}" != "" ]] ; then
    MODULES_OFED=$(dpkg -l | grep mlnx-ofed-kernel-modules | awk -F" " '{print $3}' | awk -F".g" '{print $1}')
    DKMS_OFED=$(dpkg -l | grep mlnx-ofed-kernel-dkms | awk -F" " '{print $3}' | awk -F".g" '{print $1}')
    if [[ "$MODULES_OFED" != "" ]] ; then
	dkms=0
    elif [[ "$DKMS_OFED" != "" ]] ; then
	dkms=1
    else
	echo "OFED setup type not detected"
	exit -1
    fi
else
    export WITH_OFED=0
fi

VERSION=${PKG_VERSION}-vastdata${OFED_RELEASE}
BUILD_AREA=.deb-build

if [[ -d .git ]] ; then
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

rm -rf ../../deb-dist
mkdir ../../deb-dist

sed -i s#4.5-OFED.4.5.1.0.1.1.gb4fdfac.multikernel.vastdata#2:${VERSION}# debian/changelog

if [[ "$dkms" == "1" ]] ; then
    dpkg-buildpackage -b -uc -us
else
    mv debian/control.no_dkms debian/control
    WITH_DKMS=0 dpkg-buildpackage -b -uc -us
fi

mv ../*.deb ../../deb-dist
cd ../..
ls -l deb-dist
