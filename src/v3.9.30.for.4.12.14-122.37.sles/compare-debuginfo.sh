#!/bin/bash

if ! which pahole 2>/dev/null >/dev/null ; then
    echo "Skipping debuginfo compare - 'pahole' not installed"
    exit 0
fi

structs=(task_struct)
dir=$(mktemp -d -t ci-XXXXXXXXXX)

errors=0
for struct in ${structs[@]} ; do
    pahole ./bundle/fs/nfs/nfsv3.ko -C $struct \
	| sed -E 's/__UNIQUE_ID_rh_kabi_hide[0-9]+//g' > ${dir}/before
    pahole ./inbox-compare/test.ko -C $struct \
	| sed -E 's/__UNIQUE_ID_rh_kabi_hide[0-9]+//g' > ${dir}/after
    diff -urN ${dir}/before ${dir}/after
    if [[ $?  != 0 ]] ; then
	echo "Struct is different!"
	errors=1
    fi
    rm -f ${dir}/before ${dir}/after
done

rmdir ${dir}

exit ${errors}
