# Mount and transport state

Normally, the `mountstats` command shows NFS mount status. We have extended the
interface it uses, `/proc/self/mountstats`, with our own extra state reporting
via `/sys`. The extended information contains the IP addresses related to the
transport, and a string that indicates its state flags.

The extended form of it is obtainable via `/sys/kernel/debug`. For the
association to an NFS mount, we need to obtain the related `sunrpc-id` of the
mount point.

But first, we need to enable `sunrpc-id` reporting. This can be done using
the following command after boot:

```
echo 1 | sudo tee /sys/module/sunrpc/parameters/expose_id_in_mountstats
```


Then, identify the `sunrpc-id` relevant to the mount point by looking into
`/proc/self/mountstats`:

```
$ cat /proc/self/mountstats | grep -E 'fstype nfs|sunrpc-id'
device 192.168.40.7:/opt/export mounted on /mnt/export with fstype nfs statvers=1.1
        sunrpc-id:      4
```

Now we are ready to fetch the full `mountstats` via the following command:

```
    sudo cat /sys/kernel/debug/sunrpc/rpc_clnt/4/stats
```

The added information contains multipath IP address information per `xprt`
(transport) and `xprt` state in string format. The numbers in the first `xprt:`
line are counters and other numbers related that transports.

For example:

```
        xprt:   rdma 0 0 1 0 24 3 3 0 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 11 0 0 0
                172.25.1.101 -> 172.25.1.1, state: CONNECTED BOUND
        xprt:   rdma 0 0 1 0 24 1 1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 11 0 0 0
                172.25.1.102 -> 172.25.1.2, state: CONNECTED BOUND
        xprt:   rdma 0 0 1 0 23 1 1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 11 0 0 0
                172.25.1.103 -> 172.25.1.3, state: CONNECTED BOUND
        xprt:   rdma 0 0 1 0 22 1 1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 11 0 0 0
                172.25.1.104 -> 172.25.1.4, state: CONNECTED BOUND
        xprt:   rdma 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
                172.25.1.101 -> 172.25.1.5, state: BOUND
        xprt:   rdma 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
                172.25.1.102 -> 172.25.1.6, state: BOUND
        xprt:   rdma 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
                172.25.1.103 -> 172.25.1.7, state: BOUND
        xprt:   rdma 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
                172.25.1.104 -> 172.25.1.8, state: BOUND
```
