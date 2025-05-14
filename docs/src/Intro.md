# Introduction

The VAST NFS package provides a modified version of the Linux NFS client and
server kernel code stacks designed to be built for a large range of kernel
versions.

It contains backported upstream NFS stack code from **Linux v5.15.x LTS**
kernel branch with.  This allows older kernels to receive the full
functionality of newer NFS stack code. Only kernels from 4.15.x and above
receive the backports and the fully added functionality. The package applies
also to older kernels but with less functionality (legacy VAST NFS).

## Features

- NFS stack improvements and fixes
- Multipath support for NFSv3
- Multipath support for NFSv4.1 (not for legacy kernels)
- Nvidia GDS integration
