# Detecting running version

To verify that the driver is loaded, check if the `/sys/module/sunrpc/parameters/nfs_bundle_version` exists. The content of the file can be obtained via:

```
cat /sys/module/sunrpc/parameters/nfs_bundle_version
```

The content of the file indicates the loaded version. If it does not exist,
then either it was not loaded yet, or that the NFS layer that is provided with
the installed kernel is loaded instead - in that case the upper directory `/sys/module/sunrpc` exists.
