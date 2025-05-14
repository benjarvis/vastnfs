#!/bin/bash

set -eux

rm -rf $2/.ofa_kernel
mkdir -p $2/.ofa_kernel
cp -as $1/* $2/.ofa_kernel
rm -rf $2/.ofa_kernel/include/linux/sunrpc

# Mellanox's rbtree.h affects 'struct task_struct' field sizes on some kernels.
rm -rf $2/.ofa_kernel/include/linux/rbtree.h
touch $2/.ofa_kernel/.done
