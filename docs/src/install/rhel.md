# RHEL 8.x/9.x

Install:
```
sudo yum install ./dist/vastnfs-[1-9]*.x86_64.rpm

sudo dracut -f
```

Following install, rebooting is required for the newly installed drivers to
load.

Verify that the package is indeed installed (versions should match):

```
rpm -qa | grep vastnfs
```

To further validate installation, verify that `rpcrdma` in the right path
exists and get loaded from it.  All the added kernel modules go under
`updates/bundle`:

```
rpm -qif /lib/modules/`uname -r`/extra/vastnfs/bundle/net/sunrpc/xprtrdma/rpcrdma.ko
```

```
$ modinfo rpcrdma | grep filename:
filename:       /lib/modules/..../extra/vastnfs/bundle/net/sunrpc/xprtrdma/rpcrdma.ko
```
