#!/bin/bash

# Caching of Git tagging information for version archives that don't contain
# the Git repo information.

set -e

o() {
	if [[ $(git tag | grep -E '^'$1'$') == "$1" ]] ; then
		echo $1
		return 0
	fi

	git describe $1 2>/dev/null >/dev/null
	git describe $1
	return $?
}

if [[ "$2" == "save" ]] ; then
	o $1 > $3
else
	if [[ -e .git-version-save ]] ; then
		cat .git-version-save
	else
		o $1
	fi
fi
