# Tracing

The supplied `vastnfs-ctl` helper script allows tracing the driver. The
underlying mechanism uses `trace-cmd` in order to accomplish that.


## Prerequisites


### `trace-cmd`

Make sure that the `trace-cmd` command is available.

Red Hat-based: `dnf install -y trace-cmd`
Debian-based: `apt install trace-cmd`

It is invoked by the script using `sudo`.


### Loaded kernel modules

The `vastnfs-ctl status` command should indicate that the modules are already
loaded prior to starting trace.

```
# vastnfs-ctl status | grep modules
kernel modules: sunrpc rpcrdma lockd nfs_acl auth_rpcgss rpcsec_gss_krb5 nfs nfsv3 nfsv4
```


## Performing trace

For trace collections there are 3 run modes:

1. Live - traces are emitted as they happen.
2. Collect to RAM.
3. Collect to RAM and files.


### Live traces

This is useful when there are not a lot of events, such as in the mount stage.

There are present commands for this, for example:


`vastnfs-ctl trace cmd meta`



### Collect to RAM

Manages cyclic buffers in RAM - execution goes to background. Later, the traces
can be saved to a file.

In the following example, we use 1GB of buffers for data-based IO and 100MB for
transports, connection management and error handling.

```
vastnfs-ctl trace cmd collect \
    buffer meta 100 class meta \
    buffer io 1000 class all
```

Output should be as followed:

```
Events:
     class meta 149 events bufsize 100 MB
     class all 493 events bufsize 1000 MB

Instructing trace-cmd to start background collection to RAM...
```

#### Saving the buffers to a file

```
$ vastnfs-ctl trace cmd save
Saving tracing data to nfs-traces-20221020-122203.tar.gz... (via /tmp/vastnfs-tmp-BzoxUUyTPp)

(note: the tracing RAM buffer is pruged and 'collect' will be
       needed to be used again to collect new data)

-rw-r--r-- 1 root root 2202606 Oct 20 12:22 /home/user/workdir/nfs-traces-20221020-122203.tar.gz

Done. To start a new session use 'collect' again.
```

#### Observing a saved trace report

The developers can use the extracted `nfs-traces-<timestamp>`.tar.gz tarball
and `trace-cmd report` to see meaningful output of the saved traces.


### Collect to RAM and files

Manages cyclic buffers both in RAM and files, where the RAM is immediately
flushed to files. Execution stays in foreground. Useful for large volume of
traces.

Append an `outdir <path>` to the collect command. The directory needs to exist
before execution.

For example:

```
$ vastnfs-ctl trace cmd collect buffer meta 100 class meta   buffer io 1000 class all outdir saved-traces-20221001
Checking tracepoints...

Output directory: saved-traces-20221001
Output FS type: ext4
FS storage for traces: 696192 MB

Events:
     class meta 149 events bufsize 100 MB
     class all 493 events bufsize 1000 MB

Saving logfile: /home/dan/vd/kernel/vastnfs/saved-traces-20221001/collect.log
Running trace-cmd. Wait for it to prompt readiness.
Hit Ctrl^C to stop recording
...
...
Collection done, saved-traces-20221001 can be packed and sent.
```
