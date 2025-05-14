#!/bin/bash

set -eux

rm -rf $2/.ofa_kernel
mkdir -p $2/.ofa_kernel
cp -as $1/* $2/.ofa_kernel
rm -rf $2/.ofa_kernel/include/linux/sunrpc

# Mellanox's rbtree.h affects 'struct task_struct' field sizes on some kernels.
rm -rf $2/.ofa_kernel/include/linux/rbtree.h

# Mellanox's file_inode shouldn't change implementation
if [[ -e .ofa_kernel/include/linux/compat-3.12.h ]] ; then
    cat .ofa_kernel/include/linux/compat-3.12.h > .ofa_kernel/include/linux/compat-3.12.h.to.fix
    rm -f .ofa_kernel/include/linux/compat-3.12.h
    patch -p0 < ofa-patches/compat-3.12.h.patch || true
    mv .ofa_kernel/include/linux/compat-3.12.h.to.fix .ofa_kernel/include/linux/compat-3.12.h
fi

touch $2/.ofa_kernel/.done
