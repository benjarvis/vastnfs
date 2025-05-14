#!/bin/bash

set -e
set -u

if ! git rev-parse --show-toplevel >/dev/null 2>/dev/null ; then
        echo "Not a git repository (or Git not installed)"
	exit -1
fi

if [[ -n "$(git status --porcelain)" ]]; then
	echo "'git status' needs to be clean for build-deb.sh"
	exit -1
fi

GIT_VERSION=$(./git-version.sh HEAD)
VERSION=${GIT_VERSION}-vastdata
BUILD_AREA=.src-build

rm -rf ${BUILD_AREA}
mkdir -p ${BUILD_AREA}
git archive HEAD --prefix=mlnx-nfsrdma-${GIT_VERSION}/ \
	-o ${BUILD_AREA}/mlnx-nfsrdma-${GIT_VERSION}.orig.tar.gz
cd ${BUILD_AREA}
tar -zxf mlnx-nfsrdma-${GIT_VERSION}.orig.tar.gz
cd ..

./git-version.sh HEAD save ${BUILD_AREA}/mlnx-nfsrdma-${GIT_VERSION}/.git-version-save

rm -rf src-dist/
mkdir -p src-dist/

cd .src-build/
tar -czf ../src-dist/mlnx-nfsrdma-${GIT_VERSION}.tar.gz mlnx-nfsrdma-${GIT_VERSION}
cd ..

ls -l src-dist
