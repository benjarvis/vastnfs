# Mount parameters

This package adds to the standard mount parameters that the original Linux
NFS stack originally supports.


## VAST NFS parameters

The VAST NFS client supports the following additional mount parameters that
can be used individually or together.

* `remoteports=<IP-SPEC | dns>` enables multipath allowing a single to spread traffic
  across multiple endpoint IPs. The parameter takes a list of addresses.

  The IP specification allows addresses to be specified using an inclusive
  range operator with `-`: `<FIRST-IP>-<LAST-IP>`.

  Multiple ranges and individual IPs can be combined using the `~` operator:
  `<FIRST-IP>-<LAST-IP>~<IP>`.

  Example usage:
  ```
  mount -o vers=3,remoteports=172.25.1.1-172.25.1.32 172.25.1.1:/
  ```

  With a special mode `remoteports=dns`, the client will issue one or more DNS
  queries with the mount target DNS name and will get back an array of addresses
  and use those as the list of remote addresses as if it was passed explicitly.
  Example:
  ```
  mount -t nfs -o vers=3,nconnect=16,remoteports=dns vippool3.company.lab:/ /mnt
  ```
  Notes:
  - Do not use this option in driver versions prior to 4.0.29.
  - This feature expects that no DNS caching done on the DNS server or locally.
    If caching exist on the DNS server, and the number of DNS addresses returned
    in the DNS response is lower than the value of nconnect, it may result in
    unintended reduction of the remoteports array due to repeating (cached) DNS
    responses. In order to test if DNS caching is enabled in your environment
    run:
    ```
    $ for i in $(seq 3); do dig <dns-name>.example.com +short; done
    10.10.12.7
    10.10.12.6
    10.10.12.2
    ```
    If the responses contain the same ip addresess, then caching is enabled.
  - If DNS caching is enabled, It is required to increase the number of addresses
    in the DNS server response in order for the client to correctly distribute
    connections across different remote ports.
    See example (4 addresses in the DNS response):
    ```
    $ dig <dns-name>.example.com +short
    10.10.12.1
    10.10.12.14
    10.10.12.9
    10.10.12.3
    ```


* `localports=<INT-SPEC>` controls which client interfaces are used to send
  traffic. By default the vastnfs driver will send traffic over one network
  interface. If the client has multiple network interfaces you can add
  additional interfaces with this parameter.

  The interface specification uses the same form as `remoteports` above with the
  `-` and `~` operators and the IPs assigned to the client interfaces.

  Interface names can be used instead of the IP. The driver will resolve
  the IP currently on the interface and use that IP as described above.

  Example usage:
  ```
  mount -o vers=3,localports=172.25.1.1-172.25.1.2 172.25.1.1:/

  mount -o vers=3,localports=eth0~eth1 172.25.1.1:/
  ```

  The `localports` parameter __is not necessary for multipath to work__. However
  if the local machine has more than one IP address configured for the local
  subnet, it may be useful to prevent a single local port from trying to surpass
  its maximum line rate while the other local ports are underused.

  Also, for GDS-based setups, it is necessary for making the transport layer to
  perform GPU-based routing for GPU overload to be utilized.

  The `localports` mount option isn't compatible with IP reverse path validation
  (RFC 3704). When using this option, make sure that the rp_filter sysctl option
  is disabled on the corresponding network interfaces using:
  ```
  sysctl -w "net.ipv4.conf.all.rp_filter=0" &&
  sysctl -w "net.ipv4.conf.<iface>.rp_filter=0"
  ```
  Or, disable rp_filter globally by placing following in the /etc/sysctl.conf file:
    net.ipv4.conf.default.rp_filter = 0
    net.ipv4.conf.all.rp_filter = 0
  For some distros it may be additionally required to disable rp_filter in the
  /etc/sysctl.conf file for each corresponding network interface.

* `nconnect=<NUM>` enables multiple connections for a single mount. This parameter
  is available on recent Linux kernels and the vastnfs client allows it to be used
  on Linux kernels for which it was not backported.

  The recommended value for this parameter is 4 for RDMA mounts and 8 for TCP
  mounts.

  Example usage:
  ```
  mount -o vers=3,nconnect=8 172.25.1.1:/
  ```

### Multipath mount examples

Often the above parameters are combined and used together.

Consider this multipath mount:

```
mount -o vers=3,nconnect=8,remoteports=172.25.1.1-172.25.1.32 172.25.1.1:/
```

This mount command will result in 8 TCP connections, going to a psuedo-random
sub-range of 8 addresses under the provided 32 address range.

While multipath can be used with TCP (`proto=tcp`), using it with RDMA (`proto=rdma`)
will have less CPU utilization.

Here is a more advanced example of an RDMA mount with 4 connections, 8 remote
ports and 4 local ports:

```
mount -o proto=rdma,port=20049,vers=3,nconnect=4,localports=172.25.1.101-172.25.1.104,remoteports=172.25.1.1-172.25.1.8 172.25.1.1:/
```

## Advanced parameters

### NFS optimizations

* `forcerdirplus` - Instructs the NFS client to always send an NFSv3
`READDIRPLUS` requests for servers that support it. On some remote systems this
can improve directory listing performance significantly.

* `relmtime` - Don't block stat() calls while there are pending writes. This
improves the scenario in which 'ls -l' blocks while the same client is writing
to a file under the listed directory.

* `optlockflush` - This is an optimization for applications that use a lock to
protect a read-mostly file. The assumption that GETATTR costs less than zapping
the read-cache, and that we can use GETATTR to detect whether the file was
modified at the server.


### NFS operational parameters

* `noextend` - Turns off the default extend-to-page optimization in writes,
useful for some specific applications having multiple clients on NFSv3.


### Additional transport parameters

These parameters can be used for experimentation and fine-tuning.


* `noidlexprt` - Do not disconnect idle connections.

* `remoteports_offset=<NUM>` - Controls the offset into picking out of `remoteports`
for transports if the number of actual transports via `nconnect` is smaller than
the amount of IPs given here. If not given a pseudo-random number is picked
based on source IP address.

* `localports_failover` - Special mode for multipath where failing transports
can temporarily move from local addresses that cannot serve connections, for
example on a local cable disconnect. For this option to work, `noidlexprt` needs
to be passed too, along with an `nconnect` value that is at least a multiple of
the port count in `remoteports` over `localports`.

  Example:
  ```
  mount -o vers=3,noidlexprt,localports_failover,localports=172.25.1.101-172.25.1.104,remoteports=172.25.1.1-172.25.1.8 172.25.1.1:/
  ```

  **This is only supported for NFSv3.**

* `spread_reads` or `spread_writes` - Whether a single file's IO should use a
single connection or multiple connections. Single connection enjoys a few
optimizations on the server side for latency but multiple increases the
potential bandwidth.

  **These are only supported for NFSv3.**

* `mdconnect` - Specifies the number of additional transport connections
dedicated exclusively to metadata operations. Limited at 8. When enabled, the
transports defined by `nconnect` handle only READ/WRITE requests, while the
extra connections defined by `mdconnect` serve metadata requests exclusively.
This option is particularly useful in scenarios where heavy data I/O causes
metadata operations to be queued and starved on shared transport channels.

  **This is only supported for NFSv3.**  

* `nosharetransport` - This option causes the client to establish its own isolated
transport connections. The client will not share the transport connections with
any other mount done before or after.

  **This is only supported for NFSv3.**

* `sharetransport=N` - N is positive number that identifies mounts sharing the
same transport connections. If two or more mounts to a particular NFS server
have a different value of sharetransport, these mounts will use different
connections. If you don't specify the option value for mounts to a particular
NFS server, all the mounts will shate one transport connections.

  **This is only supported for NFSv4.x**
