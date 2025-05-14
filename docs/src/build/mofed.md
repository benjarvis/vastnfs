# Mellanox OFED support

All versions of Mellanox OFED that successfully install on a Linux system
should be supported by this package. Newer OFED versions are preferred.

If the Mellanox OFED is installed on a system, this package can also be built
in such a way that NFS RDMA is functional and working using the Mellanox RDMA
stack instead of the inbox kernel RDMA stack, which the installation of the
Mellanox OFED supersedes.

The state where the Mellanox OFED is installed is detected automatically by the
build scripts, and the resultant package will only work with that particular
Mellanox OFED version.
