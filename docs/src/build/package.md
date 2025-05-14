# Building an installable package

The following command will build the source package. It detects the currently
running kernel, distribution, and optional Mellanox OFED installation.
```
./build.sh bin
```

Following a successful build, the resultant package should reside in a `dist/`
sub-directory, for example:

```
$ ls -1 dist/vastnfs* | grep -v debug

vastnfs-4.0-kernel_4.18.0_348.20.1.el8_5.x86_64.rpm
```

For older kernel, the build relies on the legacy VAST NFS 3.x sources
that are included in the source package.

See [Installation](../INSTALL.md) for installation instructions.


## Arguments

The `build.sh bin` command can receive the following optional arguments:

* `--without-rpcrdma`: Don't build RDMA support
* `--dkms`: For Debian-based system, generate a DKMS-based package even if
no DKMS-based Mellanox OFED is installed.
* `--no-ofed`: Don't detect Mellanox OFED installation, and instead build a driver
targeting the inbox kernel instead.


# Regarding DKMS

DKMS is a source-based package for which the binary driver is built on-demand.
This is useful for minor kernel updates.

If Mellanox OFED installation is detected, and if that installation is
DKMS-based, then the resultant package will also be DKMS-based by default. This
is currently supported only on Debian-based system. Otherwise, the built package
will install binaries that depend on the current kernel version.
