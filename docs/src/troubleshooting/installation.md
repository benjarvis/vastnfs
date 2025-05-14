# Installation problems

## `dnf` errors about conflicting prerequisites

If you have a custom-built kernel you might encounter an error like this:

```
sudo dnf install dist/vastnfs-4.0.23-kernel_5.15.147.el9.x86_64.rpm
...
Error:
 Problem: conflicting requests
  - nothing provides kernel(PDE_DATA) = 0x0e0f5bd2 needed by vastnfs-4.0.23-kernel_5.15.147.el9.x86_64 from @commandline
  - nothing provides kernel(__alloc_pages) = 0xc7233469 needed by vastnfs-4.0.23-kernel_5.15.147.el9.x86_64 from @commandline
  - nothing provides kernel(__bitmap_weight) = 0x63c4d61f needed by vastnfs-4.0.23-kernel_5.15.147.el9.x86_64 from @commandline
  - nothing provides kernel(__cpu_online_mask) = 0x564405cb needed by vastnfs-4.0.23-kernel_5.15.147.el9.x86_64 from @commandline
...
```

The problem is that the `kernel-core` package for your kernel is not reporting
features that the vastnfs package requires. This could either be due to:

* The target kernel is installed from a custom kernel RPM, but that RPM was
  not properly built to include kabichk 'provides' specifications
  (standard RHEL-based kernels always have kabichk).
* The target kernel wasn't installed with `rpm`/`dnf`/`yum` but rather
  installed using some custom setup.

Confirm that your kernel is installed as a package by checking with `rpm`:

```
$ rpm -qa | grep kernel-core
```

You can see what `kernel()` features that `kernel-core` package says it
provides with:

```
$ rpm -q --provides kernel-core-$(uname -r) | grep kernel
```

If the kernel was installed with a package but the package doesn't say it
provides the symbols vastnfs is looking and you suspect it's a packaging
issue with your custom kernel RPM you can force the install with `--nodeps`
to bypass the dependency check.

```
$ sudo rpm -i --nodeps ./dist/vastnfs-vast*.x86_64.rpm
```

A reboot is required for the new drivers to load.

If the vastnfs driver works in this configuration consider updating your custom
RPM kernel builds to specify the kernel features it provides to avoid this in
the future.


## `sunrpc: Unknown symbol mlx_backport_dependency_symbol (err -2)`

This means VAST NFS tried to load but the MOFED kernel layer is not loaded
and cannot load.

The reason is that by default a VAST NFS build depends directly on the MOFED
kernel modules in case the MOFED installation exists.

The usual scenario for this is following a kernel upgrade. After a kernel
upgrade, if the MOFED kernel modules are not prepared for the new kernel, they
are not loading, and system administrators sometimes don't notice it because
the inbox Infiniband layers are loaded instead and cover for that. However,
VAST NFS directly depends on the Infiniband layer it was compiled against.

The most trivial way is to reinstall MOFED (or at minimum, its kernel module
subpackage), followed by VAST NFS rebuild and install.

Another option, suppose that MOFED installation is no longer desired,
is to run `./build.sh bin --no-ofed` to create a VAST NFS build that does
not depend on it, and reinstall it instead.
