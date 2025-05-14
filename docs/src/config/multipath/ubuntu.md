# Ubuntu

Ubuntu installations can use `netplan` to specify the network configuration.
For each port, we need to have one IP address in the subnet. For each IP
address, source routing needs to be enabled.

For example, in the updated `01-netcfg.yml` for local port configuration (`ib0`,
`ib1`, `ib2`, `ib3`), _append_ `routes` and `routing-policy` rules:

```
    ib0:
        dhcp4: no
        addresses: [172.25.1.101/24]
        routes:
         - to: 172.25.1.0/24
           via: 172.25.1.101
           table: 101
        routing-policy:
         - from: 172.25.1.101
           table: 101
    ib1:
        dhcp4: no
        addresses: [172.25.1.102/24]
        routes:
         - to: 172.25.1.0/24
           via: 172.25.1.102
           table: 102
        routing-policy:
         - from: 172.25.1.102
           table: 102
    ib2:
        dhcp4: no
        addresses: [172.25.1.103/24]
        routes:
         - to: 172.25.1.0/24
           via: 172.25.1.103
           table: 103
        routing-policy:
         - from: 172.25.1.103
           table: 103
    ib3:
        dhcp4: no
        addresses: [172.25.1.104/24]
        routes:
         - to: 172.25.1.0/24
           via: 172.25.1.104
           table: 104
        routing-policy:
         - from: 172.25.1.104
           table: 104
```

NOTE: This is only an example. The IP addresses you pick depend on your network
configuration.

The apply the new configuration, run:

```
sudo netplan apply
```

Verify that the IPs and routing tables appear correctly.  Below are examples which will vary based on environment:

IP addresses:

```
$ ip a s | grep 172.25.1

    inet 172.25.1.101/20 brd 172.25.1.255 scope global ib0
    inet 172.25.1.102/20 brd 172.25.1.255 scope global ib1
    inet 172.25.1.103/20 brd 172.25.1.255 scope global ib2
    inet 172.25.1.104/20 brd 172.25.1.255 scope global ib3

```

Routing tables:

```
$ for i in 101 103 102 104; do ip route show table $i; done;

172.25.1.0/20 via 172.25.1.101 dev ib0 proto static
172.25.1.0/20 via 172.25.1.102 dev ib1 proto static
172.25.1.0/20 via 172.25.1.103 dev ib2 proto static
172.25.1.0/20 via 172.25.1.104 dev ib3 proto static
```

