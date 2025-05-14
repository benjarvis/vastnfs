# RHEL 9.x/8.x

This package is supported on RHEL, and the RHEL-based distributions such
as CentOS Linux, Rocky Linux, and Alma Linux.

NOTE: On RHEL 7.x-based kernels the package installs in backward-compatibility mode: it only provides multipath on NFSv3 mounts.

Here we demonstrate multipath with two local interfaces on the same subnet.

First we need to install `NetworkManager-config-routing-rules`.

`yum install NetworkManager-config-routing-rules`


## Configuring for multipath with `nmcli`

Suppose we have a host with two network cards, ib0 and ib1. The network cards are directly connected to the server's switch and belong to the same IP subnet. We need to define source-based routing to the server (with IP 192.168.40.11) to take advantage of multipath mode.
```
$ nmcli -f 'GENERAL.DEVICE,IP4.ADDRESS' device show
GENERAL.DEVICE:                         enp1s0
IP4.ADDRESS[1]:                         192.168.122.235/24

GENERAL.DEVICE:                         ib0
IP4.ADDRESS[1]:                         192.168.40.1/24

GENERAL.DEVICE:                         ib1
IP4.ADDRESS[1]:                         192.168.40.2/24

GENERAL.DEVICE:                         lo
IP4.ADDRESS[1]:                         127.0.0.1/8
```

1) Verify that routing tables 101 and 102 are empty:

```
ip route show table 101
ip route show table 102
```

2) Add symbolic names for custom routing tables:
```
echo '101 101' >> /etc/iproute2/rt_tables
echo '102 102' >> /etc/iproute2/rt_tables
```
3) Add custom routing tables for ib0 and ib1:

```
nmcli device modify ib0 ipv4.routes "192.168.40.0/24 src=192.168.40.1 table=101"
nmcli device modify ib1 ipv4.routes "192.168.40.0/24 src=192.168.40.2 table=102"
```

4) Add custom routing rules with priority 100:

```
nmcli device modify ib0 +ipv4.routing-rules "priority 100 from 192.168.40.1 table 101"
nmcli device modify ib1 +ipv4.routing-rules "priority 100 from 192.168.40.2 table 102"
```

5) Reload configuration and verify it:

```
$ nmcli connection reload
$ ip route get 192.168.40.11 from 192.168.40.1
192.168.40.11 from 192.168.40.1 dev ib0 table 101
    cache
$ ip route get 192.168.40.11 from 192.168.40.2
192.168.40.11 from 192.168.40.2 dev ib1 table 102
    cache
```

6) If needed, troubleshoot with commands:

```
ip route show table 101
ip route show table 102
ip rule show
```


## Configuring for multipath using old style `network-scripts` interface

We configure new source routing tables:

```
echo '101 101' >> /etc/iproute2/rt_tables
echo '102 102' >> /etc/iproute2/rt_tables
```

For this example we assume two interfaces are configured via `ifcfg-` scripts:

```
$ grep IPADDR /etc/sysconfig/network-scripts/ifcfg-ib0
IPADDR=192.168.40.1
$ grep IPADDR /etc/sysconfig/network-scripts/ifcfg-ib1
IPADDR=192.168.40.2
```

For each interface we need to add a `route-` and `rule-` files:

```
$ cat /etc/sysconfig/network-scripts/route-ib0
192.168.40.0/24 via 192.168.40.1 table 101
$ cat /etc/sysconfig/network-scripts/rule-ib0
from 192.168.40.1/32 table 101
```

NOTE: This is only an example. The IP addresses you pick depend on your network
configuration.

After reloading with `nmcli connection reload`, the `ip` command with regard to routing
should look like this (notice the two added `lookup` lines):
```
$ ip rule
0:      from all lookup local
32764:  from 192.168.40.1 lookup 101
32765:  from 192.168.40.2 lookup 102
32766:  from all lookup main
32767:  from all lookup default

$ ip route show table 101
192.168.40.0/24 via 192.168.40.1 dev ib0

$ ip route show table 102
192.168.40.0/24 via 192.168.40.2 dev ib1
```

Verify that the IPs and routing tables appear correctly.  Below are examples
which will vary based on environment:

IP addresses:

```
$ ip a s | grep 192.168.40

    inet 192.168.40.1/24 brd 192.168.40.255 scope global ib0
    inet 192.168.40.2/24 brd 192.168.40.255 scope global ib1
```

