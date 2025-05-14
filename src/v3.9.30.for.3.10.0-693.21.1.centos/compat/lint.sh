#!/bin/bash

usage() {
    git grep COMPAT_DETECT | sed -E 's/.*(COMPAT_DETECT_[0-9A-Z_]+).*/\1/g' | grep -v compat/collect | sort | uniq
}

invalid() {
    git grep DETECT_COMPAT
}

errors=0
for instance in $(usage) ; do
    if [[ ! -e compat/checks/$(echo ${instance} | tr 'A-Z' 'a-z' | cut -c15-).c ]] ; then
	echo "Compat check for ${instance} not found"
	errors=$((${errors} + 1))
	git grep ${instance}
    fi
done

if git grep DETECT_COMPAT | grep -v compat/lint.sh; then
    errors=$((${errors} + 1))
fi

if [[ ${errors} != 0 ]] ; then
    exit -1
fi
