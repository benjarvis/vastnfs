# Supported kernel versions

This package is supported on the following Linux distribution kernels:

- Red Hat Enterprise Linux 8.1 to 8.9, 9.0 to 9.5 and derivatives (CentOS, Rocky, Alma)
- SUSE Enterprise Linux 15
- Ubuntu 24 LTS (up to kernel 6.8)
- Ubuntu 22 LTS
- Ubuntu 20 LTS
- Ubuntu 18 LTS

Generic upstream kernel support ranges are the following:

- Linux v4.15 to v6.8.

Attempting to build for an unsupported kernel may produce compilation
errors.

NOTE: Some older distribution kernels (such as RH 8.1) may require a recent
Mellanox OFED being installed, as it provides a more recent IB layer along with
newer kernel APIs.


## Legacy VAST NFS 3.x fallback

The source package for VAST NFS 4.x includes the older VAST NFS 3.x branch
code targeting older kernels that belong to:

- Red Hat Enterprise Linux 7.x and derivatives (CentOS 7.x)
- SUSE Enterprise Linux 12

Building the 4.x source package will result in a package marked as 3.x due to
the less supported functionality. It currently has less functionality than 4.x
(e.g. for NFSv4.1 mounts, no `remoteports`-based multipath).
