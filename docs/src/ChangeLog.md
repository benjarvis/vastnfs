# 4.0.33

* Introduce `mdconnect` mount option to isolate metadata RPC connections for NFSv3 workloads
* For RHEL9.5 and newer kernels, update compatibility layers
* Add Debian installation dependency on `nfs-common` and run `weak-modules` on RHEL8/9 to ensure kernel modules are properly integrated
* Support separate transport channels for NFS mounts via new `nosharetransport` (NFSv3) and `sharetransport=N` (NFSv4) mount options
* Enhance Makefile: centralize NFS config definitions; fix GCC version parsing; improve RPM-based distribution error messaging
* Improve scripts: add missing trace events and revert an unintended change
* Fix sunrpc crash with backchannels when `rpc_task_state_transition` is enabled
* Correct tracepoint error in `trace_nfs3proc_access`


# 4.0.32

* Fix a backporting bug for older kernels (5.2 and below) that caused FD leak in `rpc-gssd`, relevant only for setups where Kerberos is used on those kernels.


# 4.0.31

* Fix a build issue where source tarball is unpack in a path that has symlink components
* Fix module signing on Ubuntu
* Backport 'sunrpc: fix NFSACL RPC retry on soft mount' from upstream
* For KRB5 setups using PID-based credentials cache, add fixes for NFSv4 IO recovery modes
* Don't ignore `localports=` mount option with NFSv4.x
* Don't ignore `noidlexprt` mount param with NFSv4.x
* Extend `vastnfs-ctl` with status commands for transports and clients
* For NFS4.x show remoteports in `mountstats`
* Documentation fixes

# 4.0.30

* Fix a rare case of stuck IOs in NFS v4.1 `remoteports=` multipath
* Fix documentation regarding `remoteports=dns`
* Fix mount failure when `remoteports=dns` is passed and `nconnect` is not
* Remove unwanted caching of DNS results when `remoteports=dns` is used
* Fix an issue with `buffered_internal_writeback` and unaligned user buffers
* Show the extra mount options in the output of mount for existing mounts (except: NFSv4.1 `remoteports` mode)

# 4.0.29

* Support up to Linux 6.8
* Fix stability issues regard `remoteports=dns` feature
* Fix stability issues regard passing network device names in `localports=`
* Backport fix for idmapper upcall (upstream commit [`51fd2eb52c0ca827`](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?h=51fd2eb52c0ca827))
* Further fixes for recovery states when using RDMA bonding devices

# 4.0.28

* Introduced a new `buffered_internal_writeback` module parameter, which enhances write performance on older kernels. When enabled, it applies to synchronous and large page-aligned writes. These writes are buffered but behave similarly to `O_DIRECT` writes by bypassing the overhead of the Linux writeback.
* Added support for Oracle Linux 8.9.

# 4.0.27

* Bugfix for Linux kernel 4.19.x LTS tree changes
* Fix for `remoteports=` ranges on IPv6 addresess
* Support `remoteports=dns` mode
* Fix for recovery states when using RDMA bonding devices
* Automatic readlink retry via lookup. This allows clients to replace symlinks in directories using `rename` without causing _other_ clients to fail when using them for path traversal. This improvement is pending upstream NFS client contribution.
* When debugging with all trace-events enabled, fix for stability
* Documentation changes

# 4.0.26

* Support for Linux kernels up to Linux 6.6
* Import stability patches from Linux LTS 5.15.x tree up to 5.15.147
* Add an `optlockflush` mount option (see documentation)
* Add a `noextend` mount option (see documentation)
* Debian packaging: add missing depmod call on install
* Debian packaging: fix build error on compressed kernel modules
* Fix backporting issue regarding `srcaddr` access in sysfs
* build.sh: fix Oracle Linux detection
* Various documentation changes

# 4.0.25

* Add `relmtime` mount option (see documentation).

# 4.0.24

* Bugfix for rare cases of lost writeback in backported code from Linux 5.18 to 6.7.

# 4.0.23

* Support RHEL 9.3.
* Support up to upstream kernels Linux 6.2.
* Fix the `localports_failover` implementation which was broken due to a trivial bug.
* Allow interop with builds of Lustre (only ones that have Kerberos disabled).

# 4.0.22

* Support passing network device names in `localports=`.

# 4.0.21

* Fix crash when using xattrs under NFS protocol version 4.2.

# 4.0.20

* Improve kernel header detection on Debian
* Linux 5.15.x LTS sync to v5.15.126.
* nfsd: fix support for xattrs on various kernels
* 3.9.x fallback sync: v3.9.30.

# 4.0.19

* Fix for upcoming RHEL 8.9 kernels.
* NFSv4.1 support for NFS4_RESULT_PRESERVER_UNLINKED

# 4.0.18

* Fix for Ubuntu kernels 5.19.x.

# 4.0.17

* Support for Linux kernels up to 5.19, including.
* Support for RHEL 9.2.
* Tracing improvements.

# 4.0.16

* Fix nfsd (the NFS server service) for various platforms, instead of loading a stub. Platforms include later RHEL 8.x and above ; Linux 5.4.0 kernels and above.
* Add the `spread_reads` and `spread_writes` mount parameters. These allow `remoteports=`-based NFSv3 multipath spread of IOs on single files, in contrast to the default where file handles pin to remote IP address to obtain cache locality in remote servers. NFSv4.1 `remoteports=` multipath is unchanged.

# 4.0.15

* Build fixes for OFED-5.9.
* Fix stuck IOs in NFS v4.1 remoteports= multipath.
* Packaging sync of legacy 3.x branch to 3.9.27.

# 4.0.14

* Packaging sync of legacy 3.x branch to 3.9.26.

# 4.0.13

* Build fix for Scientific Linux detection.
* Support RHEL 9.1.

# 4.0.12

* Fix read_ahead_kb default for kernel targets to be between 60 KB and 128 KB.

# 4.0.11

* Package meta-data fix.

# 4.0.10

* An additional compatibility fix was required to prevent a crash on sysctl
  access. The crash only happened with `rpcrdma` loaded under kernels older than
  Linux v5.7 which don't contain a backport of upstream commit 32927393dc1.

# 4.0.9

* A fix for a compat check under SLES15 that caused a specific known crash
  during sysctl access.

# 4.0.8

* Re-enable multipath roundrobin of meta-data IOs for NFSv3.

# 4.0.7

* A small fix for SID-based GSS auth tracking.

# 4.0.6

* Fixes to `vastnfs-ctl trace` command.

# 4.0.5

* Sync to upstream v5.15.73 TLS kernel.
* Extending trace points for most of the stack.
* Add `vastnfs-ctl trace` command.

# 4.0.4

* Build script fixes

# 4.0.3

* Build script fixes

# 4.0.2

* Import patches from 3.9.21.

# 4.0.1

* Fix `build.sh` execute mode bit.
* Remove legacy root directory docs. All docs are now under `docs/`.

# 4.0

* Initial version, including support for SID-based GSS auth tracking.

# 3.9.x

For older changes, see the [change log for the older 3.9.x branch](https://vastnfs.vastdata.com/docs/3.9.x/ChangeLog.html).
