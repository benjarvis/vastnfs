
# VAST NFS source

This is a source package for VAST NFS, that can build based on kernel and
distribution detection. Multiple kernels are supported by this package.

See [Change Log](docs/src/ChangeLog.md).

See [INSTALL](INSTALL.md) file for list of supported kernels and instructions
relevant to execution on client machines.


## Binary package

To build a binary from this source pacakge for the current kernel and OFED
versions, run `./build.sh bin`, and take the output from the `dist` directory.
If the kernel, distribution, or OFED are not supported, an error will be
printed.

   $ ./build.sh bin

   Output in dist/

   total 760
   -rw-r--r-- 1 user user 777512 Jul 20 14:25 vastnfs-modules_3.3.for.5.3.0.53.ubuntu-vastdata-5.0-OFED.5.0.2.1.8.1.kver.5.3.0-53-generic_amd64.deb
	
