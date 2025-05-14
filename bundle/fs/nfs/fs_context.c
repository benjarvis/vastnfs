// SPDX-License-Identifier: GPL-2.0-only
/*
 * linux/fs/nfs/fs_context.c
 *
 * Copyright (C) 1992 Rick Sladkey
 * Conversion to new mount api Copyright (C) David Howells
 *
 * NFS mount handling.
 *
 * Split from fs/nfs/super.c by David Howells <dhowells@redhat.com>
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/fs_context.h>
#include <linux/fs_parser.h>
#include <linux/nfs_fs.h>
#include <linux/nfs_mount.h>
#include <linux/nfs4_mount.h>
#include <linux/inetdevice.h>
#include <linux/compat.h>
#include <linux/dns_resolver.h>
#include "nfs.h"
#include "nfstrace.h"
#include "internal.h"

#define NFSDBG_FACILITY		NFSDBG_MOUNT

#if IS_ENABLED(CONFIG_NFS_V3)
#define NFS_DEFAULT_VERSION 3
#else
#define NFS_DEFAULT_VERSION 2
#endif

#define NFS_MAX_CONNECTIONS 64

static bool always_nosharetransport = 0;
module_param(always_nosharetransport, bool, 0644);

enum nfs_param {
	Opt_ac,
	Opt_acdirmax,
	Opt_acdirmin,
	Opt_acl,
	Opt_acregmax,
	Opt_acregmin,
	Opt_actimeo,
	Opt_addr,
	Opt_bg,
	Opt_bsize,
	Opt_clientaddr,
	Opt_cto,
	Opt_extend,
	Opt_fg,
	Opt_forcerdirplus,
	Opt_relmtime,
	Opt_fscache,
	Opt_fscache_flag,
	Opt_hard,
	Opt_intr,
	Opt_local_lock,
	Opt_lock,
	Opt_idlexprt,
	Opt_lookupcache,
	Opt_mdconnect,
	Opt_migration,
	Opt_minorversion,
	Opt_mountaddr,
	Opt_mounthost,
	Opt_mountport,
	Opt_mountproto,
	Opt_mountvers,
	Opt_namelen,
	Opt_nconnect,
	Opt_max_connect,
	Opt_localports,
	Opt_localports_failover,
	Opt_remoteports,
	Opt_remoteports_offset,
	Opt_spread_reads,
	Opt_spread_writes,
	Opt_port,
	Opt_posix,
	Opt_proto,
	Opt_rdirplus,
	Opt_rdma,
	Opt_resvport,
	Opt_retrans,
	Opt_retry,
	Opt_rsize,
	Opt_optlockflush,
	Opt_sec,
	Opt_sharecache,
	Opt_sharetransportid,
	Opt_nosharetransport,
	Opt_sloppy,
	Opt_soft,
	Opt_softerr,
	Opt_softreval,
	Opt_source,
	Opt_tcp,
	Opt_timeo,
	Opt_udp,
	Opt_v,
	Opt_vers,
	Opt_wsize,
	Opt_write,
};

enum {
	Opt_local_lock_all,
	Opt_local_lock_flock,
	Opt_local_lock_none,
	Opt_local_lock_posix,
};

#if !defined(COMPAT_DETECT_FS_PARAM_DESCRIPTION_ENUMS)
static const struct constant_table nfs_param_enums_local_lock[] = {
	{ "all",		Opt_local_lock_all },
	{ "flock",	Opt_local_lock_flock },
	{ "posix",	Opt_local_lock_posix },
	{ "none",		Opt_local_lock_none },
	{}
};
#endif

enum {
	Opt_lookupcache_all,
	Opt_lookupcache_none,
	Opt_lookupcache_positive,
};

#if !defined(COMPAT_DETECT_FS_PARAM_DESCRIPTION_ENUMS)
static const struct constant_table nfs_param_enums_lookupcache[] = {
	{ "all",		Opt_lookupcache_all },
	{ "none",		Opt_lookupcache_none },
	{ "pos",		Opt_lookupcache_positive },
	{ "positive",		Opt_lookupcache_positive },
	{}
};
#endif

enum {
	Opt_write_lazy,
	Opt_write_eager,
	Opt_write_wait,
};

#if defined(COMPAT_DETECT_FS_PARAM_DESCRIPTION_ENUMS)
static const struct fs_parameter_enum nfs_param_enums[] = {
       { Opt_local_lock,       "all",          Opt_local_lock_all },
       { Opt_local_lock,       "flock",        Opt_local_lock_flock },
       { Opt_local_lock,       "none",         Opt_local_lock_none },
       { Opt_local_lock,       "posix",        Opt_local_lock_posix },
       { Opt_lookupcache,      "all",          Opt_lookupcache_all },
       { Opt_lookupcache,      "none",         Opt_lookupcache_none },
       { Opt_lookupcache,      "pos",          Opt_lookupcache_positive },
       { Opt_lookupcache,      "positive",     Opt_lookupcache_positive },
       { Opt_write,	       "lazy",	       Opt_write_lazy },
       { Opt_write,	       "eager",        Opt_write_eager },
       { Opt_write,	       "wait",         Opt_write_wait },
       {}
};
#endif

static const struct constant_table nfs_param_enums_write[] = {
	{ "lazy",		Opt_write_lazy },
	{ "eager",		Opt_write_eager },
	{ "wait",		Opt_write_wait },
	{}
};

#if defined(COMPAT_DETECT_FS_PARAM_DESCRIPTION)
static const struct fs_parameter_spec nfs_param_specs[]
#else
static const struct fs_parameter_spec nfs_fs_parameters[]
#endif
 = {
	fsparam_flag_no("ac",		Opt_ac),
	fsparam_u32   ("acdirmax",	Opt_acdirmax),
	fsparam_u32   ("acdirmin",	Opt_acdirmin),
	fsparam_flag_no("acl",		Opt_acl),
	fsparam_u32   ("acregmax",	Opt_acregmax),
	fsparam_u32   ("acregmin",	Opt_acregmin),
	fsparam_u32   ("actimeo",	Opt_actimeo),
	fsparam_string("addr",		Opt_addr),
	fsparam_flag  ("bg",		Opt_bg),
	fsparam_u32   ("bsize",		Opt_bsize),
	fsparam_string("clientaddr",	Opt_clientaddr),
	fsparam_flag_no("cto",		Opt_cto),
	fsparam_flag_no("extend",	Opt_extend),
	fsparam_flag  ("fg",		Opt_fg),
#ifdef COMPAT_DETECT_FS_PARAM_V_OPTIONAL
	__fsparam_compat(fs_param_is_string, "fsc",            Opt_fscache,
			  fs_param_neg_with_no|fs_param_v_optional, NULL),
#else
	fsparam_flag_no("fsc",		Opt_fscache_flag),
	fsparam_string("fsc",		Opt_fscache),
#endif
	fsparam_flag  ("hard",		Opt_hard),
	__fsparam_compat(fsparam_type_is_flag, "intr",		Opt_intr,
		  fs_param_neg_with_no|fs_param_deprecated, NULL),
	fsparam_enum_compat  ("local_lock",	Opt_local_lock, nfs_param_enums_local_lock),
	fsparam_flag_no("lock",		Opt_lock),
	fsparam_enum_compat  ("lookupcache",	Opt_lookupcache, nfs_param_enums_lookupcache),
	fsparam_flag_no("migration",	Opt_migration),
	fsparam_flag_no("idlexprt",	Opt_idlexprt),
	fsparam_u32   ("mdconnect",	Opt_mdconnect),
	fsparam_u32   ("minorversion",	Opt_minorversion),
	fsparam_string("mountaddr",	Opt_mountaddr),
	fsparam_string("mounthost",	Opt_mounthost),
	fsparam_u32   ("mountport",	Opt_mountport),
	fsparam_string("mountproto",	Opt_mountproto),
	fsparam_u32   ("mountvers",	Opt_mountvers),
	fsparam_u32   ("namlen",	Opt_namelen),
	fsparam_u32   ("nconnect",	Opt_nconnect),
	fsparam_u32   ("max_connect",	Opt_max_connect),
	fsparam_string("localports",	Opt_localports),
	fsparam_flag  ("localports_failover",	Opt_localports_failover),
	fsparam_flag  ("optlockflush",	Opt_optlockflush),
	fsparam_string("remoteports",	Opt_remoteports),
	fsparam_u32   ("remoteports_offset",	Opt_remoteports_offset),
	fsparam_flag_no("spread_reads",	Opt_spread_reads),
	fsparam_flag_no("spread_writes", Opt_spread_writes),
	fsparam_string("nfsvers",	Opt_vers),
	fsparam_u32   ("port",		Opt_port),
	fsparam_flag_no("posix",	Opt_posix),
	fsparam_string("proto",		Opt_proto),
	fsparam_flag_no("forcerdirplus",	Opt_forcerdirplus),
	fsparam_flag  ("relmtime",  Opt_relmtime),
	fsparam_flag_no("rdirplus",	Opt_rdirplus),
	fsparam_flag  ("rdma",		Opt_rdma),
	fsparam_flag_no("resvport",	Opt_resvport),
	fsparam_u32   ("retrans",	Opt_retrans),
	fsparam_string("retry",		Opt_retry),
	fsparam_u32   ("rsize",		Opt_rsize),
	fsparam_string("sec",		Opt_sec),
	fsparam_flag_no("sharecache",	Opt_sharecache),
	fsparam_u32   ("sharetransport", Opt_sharetransportid),
	fsparam_flag  ("nosharetransport", Opt_nosharetransport),
	fsparam_flag  ("sloppy",	Opt_sloppy),
	fsparam_flag  ("soft",		Opt_soft),
	fsparam_flag  ("softerr",	Opt_softerr),
	fsparam_flag  ("softreval",	Opt_softreval),
	fsparam_string("source",	Opt_source),
	fsparam_flag  ("tcp",		Opt_tcp),
	fsparam_u32   ("timeo",		Opt_timeo),
	fsparam_flag  ("udp",		Opt_udp),
	fsparam_flag  ("v2",		Opt_v),
	fsparam_flag  ("v3",		Opt_v),
	fsparam_flag  ("v4",		Opt_v),
	fsparam_flag  ("v4.0",		Opt_v),
	fsparam_flag  ("v4.1",		Opt_v),
	fsparam_flag  ("v4.2",		Opt_v),
	fsparam_string("vers",		Opt_vers),
	fsparam_enum_compat  ("write",		Opt_write, nfs_param_enums_write),
	fsparam_u32   ("wsize",		Opt_wsize),
	{}
};

#if defined(COMPAT_DETECT_FS_PARAM_DESCRIPTION)
static const struct fs_parameter_description nfs_fs_parameters = {
       .name           = "nfs",
       .specs          = nfs_param_specs,
#if defined(COMPAT_DETECT_FS_PARAM_DESCRIPTION_ENUMS)
       .enums          = nfs_param_enums,
#endif
};
#define NFS_FS_PARAMETERS_REF &nfs_fs_parameters
#else
#define NFS_FS_PARAMETERS_REF nfs_fs_parameters
#endif

enum {
	Opt_vers_2,
	Opt_vers_3,
	Opt_vers_4,
	Opt_vers_4_0,
	Opt_vers_4_1,
	Opt_vers_4_2,
};

static const struct constant_table nfs_vers_tokens[] = {
	{ "2",		Opt_vers_2 },
	{ "3",		Opt_vers_3 },
	{ "4",		Opt_vers_4 },
	{ "4.0",	Opt_vers_4_0 },
	{ "4.1",	Opt_vers_4_1 },
	{ "4.2",	Opt_vers_4_2 },
	{}
};

enum {
	Opt_xprt_rdma,
	Opt_xprt_rdma6,
	Opt_xprt_tcp,
	Opt_xprt_tcp6,
	Opt_xprt_udp,
	Opt_xprt_udp6,
	nr__Opt_xprt
};

static const struct constant_table nfs_xprt_protocol_tokens[] = {
	{ "rdma",	Opt_xprt_rdma },
	{ "rdma6",	Opt_xprt_rdma6 },
	{ "tcp",	Opt_xprt_tcp },
	{ "tcp6",	Opt_xprt_tcp6 },
	{ "udp",	Opt_xprt_udp },
	{ "udp6",	Opt_xprt_udp6 },
	{}
};

enum {
	Opt_sec_krb5,
	Opt_sec_krb5i,
	Opt_sec_krb5p,
	Opt_sec_lkey,
	Opt_sec_lkeyi,
	Opt_sec_lkeyp,
	Opt_sec_none,
	Opt_sec_spkm,
	Opt_sec_spkmi,
	Opt_sec_spkmp,
	Opt_sec_sys,
	nr__Opt_sec
};

static const struct constant_table nfs_secflavor_tokens[] = {
	{ "krb5",	Opt_sec_krb5 },
	{ "krb5i",	Opt_sec_krb5i },
	{ "krb5p",	Opt_sec_krb5p },
	{ "lkey",	Opt_sec_lkey },
	{ "lkeyi",	Opt_sec_lkeyi },
	{ "lkeyp",	Opt_sec_lkeyp },
	{ "none",	Opt_sec_none },
	{ "null",	Opt_sec_none },
	{ "spkm3",	Opt_sec_spkm },
	{ "spkm3i",	Opt_sec_spkmi },
	{ "spkm3p",	Opt_sec_spkmp },
	{ "sys",	Opt_sec_sys },
	{}
};

static DEFINE_IDA(nfs_fs_context_trids);

void nfs_fs_context_cleanup_trids(void)
{
	ida_destroy(&nfs_fs_context_trids);
}

static int nfs_fs_context_alloc_trid(struct nfs_fs_context *ctx)
{
	int id = ida_simple_get(&nfs_fs_context_trids, 5000, 0, GFP_KERNEL);
	if (id < 0)
		return id;

	ctx->trid = id;
	return 0;
}

static void nfs_fs_context_free_trid(struct nfs_fs_context *ctx)
{
	ida_simple_remove(&nfs_fs_context_trids, ctx->trid);
}

void
nfs_fs_context_trace(const char *kind, struct fs_context *fc,
		     const char *fmt, ...)
{
	struct nfs_fs_context *ctx = nfs_fc2context(fc);
	va_list args;
	char buf[0x80] = {};

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	trace_nfs_fs_context_log(ctx, kind, buf);
}
EXPORT_SYMBOL(nfs_fs_context_trace);

/*
 * Sanity-check a server address provided by the mount command.
 *
 * Address family must be initialized, and address must not be
 * the ANY address for that family.
 */
static int nfs_verify_server_address(struct sockaddr *addr)
{
	switch (addr->sa_family) {
	case AF_INET: {
		struct sockaddr_in *sa = (struct sockaddr_in *)addr;
		return sa->sin_addr.s_addr != htonl(INADDR_ANY);
	}
	case AF_INET6: {
		struct in6_addr *sa = &((struct sockaddr_in6 *)addr)->sin6_addr;
		return !ipv6_addr_any(sa);
	}
	}

	dfprintk(MOUNT, "NFS: Invalid IP address specified\n");
	return 0;
}

#ifdef CONFIG_NFS_DISABLE_UDP_SUPPORT
static bool nfs_server_transport_udp_invalid(const struct nfs_fs_context *ctx)
{
	return true;
}
#else
static bool nfs_server_transport_udp_invalid(const struct nfs_fs_context *ctx)
{
	if (ctx->version == 4)
		return true;
	return false;
}
#endif

/*
 * Sanity check the NFS transport protocol.
 */
static int nfs_validate_transport_protocol(struct fs_context *fc,
					   struct nfs_fs_context *ctx)
{
	switch (ctx->nfs_server.protocol) {
	case XPRT_TRANSPORT_UDP:
		if (nfs_server_transport_udp_invalid(ctx))
			goto out_invalid_transport_udp;
		break;
	case XPRT_TRANSPORT_TCP:
	case XPRT_TRANSPORT_RDMA:
		break;
	default:
		ctx->nfs_server.protocol = XPRT_TRANSPORT_TCP;
	}
	return 0;
out_invalid_transport_udp:
	return nfs_invalf(fc, "NFS: Unsupported transport protocol udp");
}

/*
 * For text based NFSv2/v3 mounts, the mount protocol transport default
 * settings should depend upon the specified NFS transport.
 */
static void nfs_set_mount_transport_protocol(struct nfs_fs_context *ctx)
{
	if (ctx->mount_server.protocol == XPRT_TRANSPORT_UDP ||
	    ctx->mount_server.protocol == XPRT_TRANSPORT_TCP)
			return;
	switch (ctx->nfs_server.protocol) {
	case XPRT_TRANSPORT_UDP:
		ctx->mount_server.protocol = XPRT_TRANSPORT_UDP;
		break;
	case XPRT_TRANSPORT_TCP:
	case XPRT_TRANSPORT_RDMA:
		ctx->mount_server.protocol = XPRT_TRANSPORT_TCP;
	}
}

/*
 * Add 'flavor' to 'auth_info' if not already present.
 * Returns true if 'flavor' ends up in the list, false otherwise
 */
static int nfs_auth_info_add(struct fs_context *fc,
			     struct nfs_auth_info *auth_info,
			     rpc_authflavor_t flavor)
{
	unsigned int i;
	unsigned int max_flavor_len = ARRAY_SIZE(auth_info->flavors);

	/* make sure this flavor isn't already in the list */
	for (i = 0; i < auth_info->flavor_len; i++) {
		if (flavor == auth_info->flavors[i])
			return 0;
	}

	if (auth_info->flavor_len + 1 >= max_flavor_len)
		return nfs_invalf(fc, "NFS: too many sec= flavors");

	auth_info->flavors[auth_info->flavor_len++] = flavor;
	return 0;
}

/*
 * Parse the value of the 'sec=' option.
 */
static int nfs_parse_security_flavors(struct fs_context *fc,
				      struct fs_parameter *param)
{
	struct nfs_fs_context *ctx = nfs_fc2context(fc);
	rpc_authflavor_t pseudoflavor;
	char *string = param->string, *p;
	int ret;

	dfprintk(MOUNT, "NFS: parsing %s=%s option\n", param->key, param->string);

	while ((p = strsep(&string, ":")) != NULL) {
		if (!*p)
			continue;
		switch (lookup_constant(nfs_secflavor_tokens, p, -1)) {
		case Opt_sec_none:
			pseudoflavor = RPC_AUTH_NULL;
			break;
		case Opt_sec_sys:
			pseudoflavor = RPC_AUTH_UNIX;
			break;
		case Opt_sec_krb5:
			pseudoflavor = RPC_AUTH_GSS_KRB5;
			break;
		case Opt_sec_krb5i:
			pseudoflavor = RPC_AUTH_GSS_KRB5I;
			break;
		case Opt_sec_krb5p:
			pseudoflavor = RPC_AUTH_GSS_KRB5P;
			break;
		case Opt_sec_lkey:
			pseudoflavor = RPC_AUTH_GSS_LKEY;
			break;
		case Opt_sec_lkeyi:
			pseudoflavor = RPC_AUTH_GSS_LKEYI;
			break;
		case Opt_sec_lkeyp:
			pseudoflavor = RPC_AUTH_GSS_LKEYP;
			break;
		case Opt_sec_spkm:
			pseudoflavor = RPC_AUTH_GSS_SPKM;
			break;
		case Opt_sec_spkmi:
			pseudoflavor = RPC_AUTH_GSS_SPKMI;
			break;
		case Opt_sec_spkmp:
			pseudoflavor = RPC_AUTH_GSS_SPKMP;
			break;
		default:
			return nfs_invalf(fc, "NFS: sec=%s option not recognized", p);
		}

		ret = nfs_auth_info_add(fc, &ctx->auth_info, pseudoflavor);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int nfs_parse_version_string(struct fs_context *fc,
				    const char *string)
{
	struct nfs_fs_context *ctx = nfs_fc2context(fc);

	ctx->flags &= ~NFS_MOUNT_VER3;
	switch (lookup_constant(nfs_vers_tokens, string, -1)) {
	case Opt_vers_2:
		ctx->version = 2;
		break;
	case Opt_vers_3:
		ctx->flags |= NFS_MOUNT_VER3;
		ctx->version = 3;
		break;
	case Opt_vers_4:
		/* Backward compatibility option. In future,
		 * the mount program should always supply
		 * a NFSv4 minor version number.
		 */
		ctx->version = 4;
		break;
	case Opt_vers_4_0:
		ctx->version = 4;
		ctx->minorversion = 0;
		break;
	case Opt_vers_4_1:
		ctx->version = 4;
		ctx->minorversion = 1;
		break;
	case Opt_vers_4_2:
		ctx->version = 4;
		ctx->minorversion = 2;
		break;
	default:
		return nfs_invalf(fc, "NFS: Unsupported NFS version");
	}
	return 0;
}

static int nfs_parse_netdev_or_address(struct net *net, const char *buf,
				       const size_t buflen,
				       struct sockaddr *sap, const size_t salen,
				       const char *type)
{
	struct net_device *dev;
	struct in_device *in_dev;
	char buf0[IFNAMSIZ] = {};
	uint32_t ipv4_addr;
	int res;

	res = rpc_pton(net, buf, buflen, sap, salen);
	if (res)
		return res;

	if (strcmp(type, "local"))
		goto error;

	/* We are going for a local address. Try to resolve IPv4 address from
	 * network device name */
	if (buflen >= IFNAMSIZ)
		goto error;

	memcpy(buf0, buf, buflen);
	dev = dev_get_by_name(net, buf0);
	if (dev == NULL)
		goto error;

	in_dev = in_dev_get(dev);
	if (in_dev) {
		if (!sunrpc_get_in_dev_primary_addr_locked(in_dev, &ipv4_addr)) {
			/* We found an address */
			struct sockaddr_in *sin = (struct sockaddr_in *)sap;
			memset(sin, 0, sizeof(*sin));
			sin->sin_family = AF_INET;
			sin->sin_addr.s_addr = ipv4_addr;
			res = sizeof(struct sockaddr_in);
		}
		in_dev_put(in_dev);
	}

	dev_put(dev);
	return res;

error:
	return 0;
}

static int __nfs_get_dns_remoteports(struct fs_context *fc, int max_rports)
{
	struct nfs_fs_context *ctx = nfs_fc2context(fc);
	struct rpc_portgroup *pg = ctx->remote_ports;
	char *ips, *p, *ip_list;
	int ips_len, err = 0, i = 0;
	size_t len;

	if (!ctx->nfs_server.hostname) {
		pr_err("NFS: no dns name mount server given\n");
		return -EINVAL;
	}

	ips_len = dns_query(fc->net_ns, NULL, ctx->nfs_server.hostname,
			strlen(ctx->nfs_server.hostname), "list", &ip_list,
			NULL, true);
	if (ips_len < 0)
		return ips_len;

	ips = ip_list;
	while ((p = strsep(&ips, ",")) != NULL) {
		if (pg->nr >= RPC_MAX_PORTS) {
			pr_debug("NFS: portgroup for remote reached limit %d\n",
				pg->nr);
			break;
		}

		if (i >= max_rports)
			break;

		len = rpc_pton(fc->net_ns, p, strlen(p),
				(struct sockaddr *)&pg->addrs[pg->nr],
				sizeof(pg->addrs[pg->nr]));
		if (!len) {
			err = -EINVAL;
			break;
		}
		pg->lens[pg->nr] = len;

		pr_debug("NFS: adding remoteport[%d]: %pIS\n",
			pg->nr, &pg->addrs[pg->nr]);
		pg->nr++;
		i++;
	}
	kfree(ip_list);
	return err;
}

static int nfs_get_dns_remoteports(struct fs_context *fc)
{
	struct nfs_fs_context *ctx = nfs_fc2context(fc);
	struct rpc_portgroup *pg = ctx->remote_ports;
	int nconnect = ctx->nfs_server.nconnect ?: 1;
	int ret;

	while (pg->nr < nconnect) {
		ret = __nfs_get_dns_remoteports(fc, nconnect - pg->nr);
		if (ret)
			break;
	}
	return ret;
}

static int nfs_parse_port_group(char *string, struct fs_context *fc,
				struct rpc_portgroup **pg_out, const char *type)
{
	struct rpc_portgroup *pg = NULL;
	char *string_scan;
	bool prev_range = false;

	if (!*pg_out) {
		pg = vmalloc(sizeof(*pg));;
		if (!pg)
			return -ENOMEM;
		memset(pg, 0, sizeof(*pg));
		*pg_out = pg;
	} else {
		pg = *pg_out;
	}

	if (pg->nr >= RPC_MAX_PORTS) {
		printk(KERN_INFO "NFS: portgroup for %s"
		       " is too large, reached %d\n", type, pg->nr);
		return -ENOSPC;
	}

	/*
	 * In case remoteports=dns we delay the processing
	 * to the validate stage because we need the source
	 * name in order to perform the dns query upcall.
	 */
	if (!strcmp(string, "dns")) {
		if (!strcmp(type, "local"))
			return -EINVAL;
		if (pg->nr)
			return -EINVAL;
		pg->dns = true;
		return 0;
	}

	string_scan = string;
	while (string_scan != NULL) {
		const char *delim;
		const char *token = string_scan;
		const char *a = strchr(string_scan, '~');
		const char *b = strchr(string_scan, '-');
		bool single = true;

		delim = NULL;
		if (b && a) {
			if (b < a) {
				single = false;
				delim = strsep(&string_scan, "-");
			} else {
				delim = strsep(&string_scan, "~");
			}
		} else if (a) {
			delim = strsep(&string_scan, "~");
		} else if (b) {
			single = false;
			delim = strsep(&string_scan, "-");
		}

		if (!prev_range) {
			if (pg->nr >= RPC_MAX_PORTS) {
				printk(KERN_INFO "NFS: portgroup for %s"
				       " is too large, reached %d\n", type, pg->nr);
				return -ENOSPC;
			}

			pg->lens[pg->nr] = nfs_parse_netdev_or_address(
						    fc->net_ns, token, strlen(token),
						    (struct sockaddr *)
						    &pg->addrs[pg->nr],
						    sizeof(pg->addrs[pg->nr]),
						    type);
			if (pg->lens[pg->nr] == 0)
				return -EINVAL;

			pg->nr++;
		} else {
			struct sockaddr_storage addr;
			size_t len;

			len = rpc_pton(fc->net_ns, token, strlen(token),
				       (struct sockaddr *)&addr, sizeof(addr));

			if (!len)
				return -EINVAL;

			while (true) {
				if (pg->nr >= RPC_MAX_PORTS) {
					printk(KERN_INFO "NFS: portgroup for %s"
					       " is too large, reached %d\n", type, pg->nr);
					return -ENOSPC;
				}
				if (nfs_get_succeeding_ipaddr((struct sockaddr *)(pg->addrs + pg->nr - 1),
								pg->addrs + pg->nr) <= 0)
					return -ENOTSUPP;

				pg->lens[pg->nr] = pg->lens[pg->nr - 1];
				pg->nr += 1;

				if (rpc_cmp_addr((const struct sockaddr *)&pg->addrs[pg->nr - 1],
						  (const struct sockaddr *)&addr))
					break;
			}
			prev_range = false;
		}

		if (!delim) {
			/* We just processed the last remaining token */
			break;
		}

		if (single) {
			prev_range = false;
		} else {
			if (prev_range)
				return -EINVAL;
			prev_range = true;
		}
	}

	return 0;
}

/*
 * Parse a single mount parameter.
 */
static int nfs_fs_context_parse_param(struct fs_context *fc,
				      struct fs_parameter *param)
{
	struct fs_parse_result result;
	struct nfs_fs_context *ctx = nfs_fc2context(fc);
	unsigned short protofamily, mountfamily;
	unsigned int len;
	int ret, opt;

	dfprintk(MOUNT, "NFS:   parsing nfs mount option '%s'\n", param->key);

	opt = fs_parse(fc, NFS_FS_PARAMETERS_REF, param, &result);
	if (opt < 0)
		return ctx->sloppy ? 1 : opt;

	{
		char param_str[0x20] = {};
		const char *value = &param_str[0];

		if (param->type == fs_value_is_string && param->string)
			value = param->string;
		else
			snprintf(param_str, sizeof(param_str),
				 "%d", result.uint_32);

		trace_nfs_fs_context_parse_param(ctx, param->key, value);
	}


#ifdef COMPAT_DETECT_INCLUDE_FS_CONTEXT
	if (fc->security)
		ctx->has_sec_mnt_opts = 1;
#endif
	switch (opt) {
	case Opt_source:
		if (fc->source)
			return nfs_invalf(fc, "NFS: Multiple sources not supported");
		fc->source = param->string;
		param->string = NULL;
		break;

		/*
		 * boolean options:  foo/nofoo
		 */
	case Opt_soft:
		ctx->flags |= NFS_MOUNT_SOFT;
		ctx->flags &= ~NFS_MOUNT_SOFTERR;
		break;
	case Opt_softerr:
		ctx->flags |= NFS_MOUNT_SOFTERR | NFS_MOUNT_SOFTREVAL;
		ctx->flags &= ~NFS_MOUNT_SOFT;
		break;
	case Opt_hard:
		ctx->flags &= ~(NFS_MOUNT_SOFT |
				NFS_MOUNT_SOFTERR |
				NFS_MOUNT_SOFTREVAL);
		break;
	case Opt_softreval:
		if (result.negated)
			ctx->flags &= ~NFS_MOUNT_SOFTREVAL;
		else
			ctx->flags |= NFS_MOUNT_SOFTREVAL;
		break;
	case Opt_posix:
		if (result.negated)
			ctx->flags &= ~NFS_MOUNT_POSIX;
		else
			ctx->flags |= NFS_MOUNT_POSIX;
		break;
	case Opt_cto:
		if (result.negated)
			ctx->flags |= NFS_MOUNT_NOCTO;
		else
			ctx->flags &= ~NFS_MOUNT_NOCTO;
		break;
	case Opt_extend:
		if (result.negated)
			ctx->flags |= NFS_MOUNT_NO_EXTEND;
		else
			ctx->flags &= ~NFS_MOUNT_NO_EXTEND;
		break;
	case Opt_ac:
		if (result.negated)
			ctx->flags |= NFS_MOUNT_NOAC;
		else
			ctx->flags &= ~NFS_MOUNT_NOAC;
		break;
	case Opt_lock:
		if (result.negated) {
			ctx->flags |= NFS_MOUNT_NONLM;
			ctx->flags |= (NFS_MOUNT_LOCAL_FLOCK | NFS_MOUNT_LOCAL_FCNTL);
		} else {
			ctx->flags &= ~NFS_MOUNT_NONLM;
			ctx->flags &= ~(NFS_MOUNT_LOCAL_FLOCK | NFS_MOUNT_LOCAL_FCNTL);
		}
		break;
	case Opt_optlockflush:
		if (result.negated)
			ctx->flags &= ~NFS_MOUNT_OPT_LOCK_FLUSH;
		else
			ctx->flags |= NFS_MOUNT_OPT_LOCK_FLUSH;
		break;
	case Opt_udp:
		ctx->flags &= ~NFS_MOUNT_TCP;
		ctx->nfs_server.protocol = XPRT_TRANSPORT_UDP;
		break;
	case Opt_tcp:
	case Opt_rdma:
		ctx->flags |= NFS_MOUNT_TCP; /* for side protocols */
		ret = xprt_find_transport_ident(param->key);
		if (ret < 0)
			goto out_bad_transport;
		ctx->nfs_server.protocol = ret;
		break;
	case Opt_acl:
		if (result.negated)
			ctx->flags |= NFS_MOUNT_NOACL;
		else
			ctx->flags &= ~NFS_MOUNT_NOACL;
		break;
	case Opt_rdirplus:
		if (result.negated)
			ctx->flags |= NFS_MOUNT_NORDIRPLUS;
		else
			ctx->flags &= ~NFS_MOUNT_NORDIRPLUS;
		break;
	case Opt_sharecache:
		if (result.negated)
			ctx->flags |= NFS_MOUNT_UNSHARED;
		else
			ctx->flags &= ~NFS_MOUNT_UNSHARED;
		break;
	case Opt_sharetransportid:
		ctx->flags &= ~NFS_MOUNT_NOSHARE_XPRT;
		if (param->string && result.uint_32 > 0)
			ctx->xprt_id = result.uint_32;
		break;
	case Opt_nosharetransport:
		ctx->flags |= NFS_MOUNT_NOSHARE_XPRT;
		break;
	case Opt_resvport:
		if (result.negated)
			ctx->flags |= NFS_MOUNT_NORESVPORT;
		else
			ctx->flags &= ~NFS_MOUNT_NORESVPORT;
		break;
	case Opt_fscache_flag:
		if (result.negated)
			ctx->options &= ~NFS_OPTION_FSCACHE;
		else
			ctx->options |= NFS_OPTION_FSCACHE;
		kfree(ctx->fscache_uniq);
		ctx->fscache_uniq = NULL;
		break;
	case Opt_fscache:
		ctx->options |= NFS_OPTION_FSCACHE;
		kfree(ctx->fscache_uniq);
		ctx->fscache_uniq = param->string;
		param->string = NULL;
		break;
	case Opt_migration:
		if (result.negated)
			ctx->options &= ~NFS_OPTION_MIGRATION;
		else
			ctx->options |= NFS_OPTION_MIGRATION;
		break;
	case Opt_forcerdirplus:
		if (result.negated)
			ctx->flags &= ~NFS_MOUNT_FORCE_READDIRPLUS;
		else
			ctx->flags |= NFS_MOUNT_FORCE_READDIRPLUS;
		break;
	case Opt_relmtime:
		if (result.negated)
			ctx->flags &= ~NFS_MOUNT_RELMTIME;
		else
			ctx->flags |= NFS_MOUNT_RELMTIME;
		break;
	case Opt_idlexprt:
		if (result.negated)
			ctx->no_idle_xprt = true;
		break;
	case Opt_mdconnect:
		if (result.uint_32 < 1 || result.uint_32 > NFS_MAX_MD_CONNECTIONS)
			goto out_of_bounds;
		ctx->nfs_server.mdconnect = result.uint_32;
		break;

		/*
		 * options that take numeric values
		 */
	case Opt_port:
		if (result.uint_32 > USHRT_MAX)
			goto out_of_bounds;
		ctx->nfs_server.port = result.uint_32;
		break;
	case Opt_rsize:
		ctx->rsize = result.uint_32;
		break;
	case Opt_wsize:
		ctx->wsize = result.uint_32;
		break;
	case Opt_bsize:
		ctx->bsize = result.uint_32;
		break;
	case Opt_timeo:
		if (result.uint_32 < 1 || result.uint_32 > INT_MAX)
			goto out_of_bounds;
		ctx->timeo = result.uint_32;
		break;
	case Opt_retrans:
		if (result.uint_32 > INT_MAX)
			goto out_of_bounds;
		ctx->retrans = result.uint_32;
		break;
	case Opt_acregmin:
		ctx->acregmin = result.uint_32;
		break;
	case Opt_acregmax:
		ctx->acregmax = result.uint_32;
		break;
	case Opt_acdirmin:
		ctx->acdirmin = result.uint_32;
		break;
	case Opt_acdirmax:
		ctx->acdirmax = result.uint_32;
		break;
	case Opt_actimeo:
		ctx->acregmin = result.uint_32;
		ctx->acregmax = result.uint_32;
		ctx->acdirmin = result.uint_32;
		ctx->acdirmax = result.uint_32;
		break;
	case Opt_namelen:
		ctx->namlen = result.uint_32;
		break;
	case Opt_mountport:
		if (result.uint_32 > USHRT_MAX)
			goto out_of_bounds;
		ctx->mount_server.port = result.uint_32;
		break;
	case Opt_mountvers:
		if (result.uint_32 < NFS_MNT_VERSION ||
		    result.uint_32 > NFS_MNT3_VERSION)
			goto out_of_bounds;
		ctx->mount_server.version = result.uint_32;
		break;
	case Opt_minorversion:
		if (result.uint_32 > NFS4_MAX_MINOR_VERSION)
			goto out_of_bounds;
		ctx->minorversion = result.uint_32;
		break;

		/*
		 * options that take text values
		 */
	case Opt_v:
		ret = nfs_parse_version_string(fc, param->key + 1);
		if (ret < 0)
			return ret;
		break;
	case Opt_vers:
		ret = nfs_parse_version_string(fc, param->string);
		if (ret < 0)
			return ret;
		break;
	case Opt_sec:
		ret = nfs_parse_security_flavors(fc, param);
		if (ret < 0)
			return ret;
		break;

	case Opt_proto:
		protofamily = AF_INET;
		switch (lookup_constant(nfs_xprt_protocol_tokens, param->string, -1)) {
		case Opt_xprt_udp6:
			protofamily = AF_INET6;
			fallthrough;
		case Opt_xprt_udp:
			ctx->flags &= ~NFS_MOUNT_TCP;
			ctx->nfs_server.protocol = XPRT_TRANSPORT_UDP;
			break;
		case Opt_xprt_tcp6:
			protofamily = AF_INET6;
			fallthrough;
		case Opt_xprt_tcp:
			ctx->flags |= NFS_MOUNT_TCP;
			ctx->nfs_server.protocol = XPRT_TRANSPORT_TCP;
			break;
		case Opt_xprt_rdma6:
			protofamily = AF_INET6;
			fallthrough;
		case Opt_xprt_rdma:
			/* vector side protocols to TCP */
			ctx->flags |= NFS_MOUNT_TCP;
			ret = xprt_find_transport_ident(param->string);
			if (ret < 0)
				goto out_bad_transport;
			ctx->nfs_server.protocol = ret;
			break;
		default:
			goto out_bad_transport;
		}

		ctx->protofamily = protofamily;
		break;

	case Opt_mountproto:
		mountfamily = AF_INET;
		switch (lookup_constant(nfs_xprt_protocol_tokens, param->string, -1)) {
		case Opt_xprt_udp6:
			mountfamily = AF_INET6;
			fallthrough;
		case Opt_xprt_udp:
			ctx->mount_server.protocol = XPRT_TRANSPORT_UDP;
			break;
		case Opt_xprt_tcp6:
			mountfamily = AF_INET6;
			fallthrough;
		case Opt_xprt_tcp:
			ctx->mount_server.protocol = XPRT_TRANSPORT_TCP;
			break;
		case Opt_xprt_rdma: /* not used for side protocols */
		default:
			goto out_bad_transport;
		}
		ctx->mountfamily = mountfamily;
		break;

	case Opt_addr:
		len = rpc_pton(fc->net_ns, param->string, param->size,
			       &ctx->nfs_server.address,
			       sizeof(ctx->nfs_server._address));
		if (len == 0)
			goto out_invalid_address;
		ctx->nfs_server.addrlen = len;
		break;
	case Opt_clientaddr:
		kfree(ctx->client_address);
		ctx->client_address = param->string;
		param->string = NULL;
		break;
	case Opt_mounthost:
		kfree(ctx->mount_server.hostname);
		ctx->mount_server.hostname = param->string;
		param->string = NULL;
		break;
	case Opt_mountaddr:
		len = rpc_pton(fc->net_ns, param->string, param->size,
			       &ctx->mount_server.address,
			       sizeof(ctx->mount_server._address));
		if (len == 0)
			goto out_invalid_address;
		ctx->mount_server.addrlen = len;
		break;
	case Opt_nconnect:
		if (result.uint_32 < 1 || result.uint_32 > NFS_MAX_CONNECTIONS)
			goto out_of_bounds;
		ctx->nfs_server.nconnect = result.uint_32;
		break;
	case Opt_remoteports_offset:
		if (!(result.uint_32 < 1 || result.uint_32 > 0x100)) {
			ctx->nfs_server.remoteports_offset = result.uint_32;
			ctx->nfs_server.remoteports_offset_provided = true;
		} else {
			goto out_of_bounds;
		}
		break;
	case Opt_spread_reads:
		ctx->nfs_server.spread_reads = true;
		break;
	case Opt_spread_writes:
		ctx->nfs_server.spread_writes = true;
		break;
	case Opt_localports_failover:
		ctx->local_ports_failover = true;
		break;
	case Opt_localports:
		if (param->string == NULL)
			goto out_nomem;
		ret = nfs_parse_port_group(param->string, fc, &ctx->local_ports, "local");

		switch (ret) {
		case -ENOMEM: goto out_nomem;
		case -ENOSPC: goto out_portgroup_too_large;
		case -EINVAL: goto out_invalid_address;
		}
		break;
	case Opt_remoteports:
		if (param->string == NULL)
			goto out_nomem;
		ret = nfs_parse_port_group(param->string, fc, &ctx->remote_ports, "remote");

		switch (ret) {
		case -ENOMEM: goto out_nomem;
		case -ENOSPC: goto out_portgroup_too_large;
		case -EINVAL: goto out_invalid_address;
		}
		break;
	case Opt_max_connect:
		if (result.uint_32 < 1 || result.uint_32 > NFS_MAX_TRANSPORTS)
			goto out_of_bounds;
		ctx->nfs_server.max_connect = result.uint_32;
		break;
	case Opt_lookupcache:
		switch (result.uint_32) {
		case Opt_lookupcache_all:
			ctx->flags &= ~(NFS_MOUNT_LOOKUP_CACHE_NONEG|NFS_MOUNT_LOOKUP_CACHE_NONE);
			break;
		case Opt_lookupcache_positive:
			ctx->flags &= ~NFS_MOUNT_LOOKUP_CACHE_NONE;
			ctx->flags |= NFS_MOUNT_LOOKUP_CACHE_NONEG;
			break;
		case Opt_lookupcache_none:
			ctx->flags |= NFS_MOUNT_LOOKUP_CACHE_NONEG|NFS_MOUNT_LOOKUP_CACHE_NONE;
			break;
		default:
			goto out_invalid_value;
		}
		break;
	case Opt_local_lock:
		switch (result.uint_32) {
		case Opt_local_lock_all:
			ctx->flags |= (NFS_MOUNT_LOCAL_FLOCK |
				       NFS_MOUNT_LOCAL_FCNTL);
			break;
		case Opt_local_lock_flock:
			ctx->flags |= NFS_MOUNT_LOCAL_FLOCK;
			break;
		case Opt_local_lock_posix:
			ctx->flags |= NFS_MOUNT_LOCAL_FCNTL;
			break;
		case Opt_local_lock_none:
			ctx->flags &= ~(NFS_MOUNT_LOCAL_FLOCK |
					NFS_MOUNT_LOCAL_FCNTL);
			break;
		default:
			goto out_invalid_value;
		}
		break;
	case Opt_write:
		switch (result.uint_32) {
		case Opt_write_lazy:
			ctx->flags &=
				~(NFS_MOUNT_WRITE_EAGER | NFS_MOUNT_WRITE_WAIT);
			break;
		case Opt_write_eager:
			ctx->flags |= NFS_MOUNT_WRITE_EAGER;
			ctx->flags &= ~NFS_MOUNT_WRITE_WAIT;
			break;
		case Opt_write_wait:
			ctx->flags |=
				NFS_MOUNT_WRITE_EAGER | NFS_MOUNT_WRITE_WAIT;
			break;
		default:
			goto out_invalid_value;
		}
		break;

		/*
		 * Special options
		 */
	case Opt_sloppy:
		ctx->sloppy = true;
		dfprintk(MOUNT, "NFS:   relaxing parsing rules\n");
		break;
	}

	return 0;

out_invalid_value:
	return nfs_invalf(fc, "NFS: Bad mount option value specified");
out_invalid_address:
	return nfs_invalf(fc, "NFS: Bad IP address specified");
out_nomem:
	return nfs_invalf(fc, "NFS: not enough memory to parse device name");
out_portgroup_too_large:
	return nfs_invalf(fc, "NFS: portgroup is too large");
out_of_bounds:
	return nfs_invalf(fc, "NFS: Value for '%s' out of range", param->key);
out_bad_transport:
	return nfs_invalf(fc, "NFS: Unrecognized transport protocol");
}

/*
 * Split fc->source into "hostname:export_path".
 *
 * The leftmost colon demarks the split between the server's hostname
 * and the export path.  If the hostname starts with a left square
 * bracket, then it may contain colons.
 *
 * Note: caller frees hostname and export path, even on error.
 */
static int nfs_parse_source(struct fs_context *fc,
			    size_t maxnamlen, size_t maxpathlen)
{
	struct nfs_fs_context *ctx = nfs_fc2context(fc);
	const char *dev_name = fc->source;
	size_t len;
	const char *end;

	if (unlikely(!dev_name || !*dev_name)) {
		dfprintk(MOUNT, "NFS: device name not specified\n");
		return -EINVAL;
	}

	/* Is the host name protected with square brakcets? */
	if (*dev_name == '[') {
		end = strchr(++dev_name, ']');
		if (end == NULL || end[1] != ':')
			goto out_bad_devname;

		len = end - dev_name;
		end++;
	} else {
		const char *comma;

		end = strchr(dev_name, ':');
		if (end == NULL)
			goto out_bad_devname;
		len = end - dev_name;

		/* kill possible hostname list: not supported */
		comma = memchr(dev_name, ',', len);
		if (comma)
			len = comma - dev_name;
	}

	if (len > maxnamlen)
		goto out_hostname;

	kfree(ctx->nfs_server.hostname);

	/* N.B. caller will free nfs_server.hostname in all cases */
	ctx->nfs_server.hostname = kmemdup_nul(dev_name, len, GFP_KERNEL);
	if (!ctx->nfs_server.hostname)
		goto out_nomem;
	len = strlen(++end);
	if (len > maxpathlen)
		goto out_path;
	ctx->nfs_server.export_path = kmemdup_nul(end, len, GFP_KERNEL);
	if (!ctx->nfs_server.export_path)
		goto out_nomem;

	dfprintk(MOUNT, "NFS: MNTPATH: '%s'\n", ctx->nfs_server.export_path);
	return 0;

out_bad_devname:
	return nfs_invalf(fc, "NFS: device name not in host:path format");
out_nomem:
	nfs_errorf(fc, "NFS: not enough memory to parse device name");
	return -ENOMEM;
out_hostname:
	nfs_errorf(fc, "NFS: server hostname too long");
	return -ENAMETOOLONG;
out_path:
	nfs_errorf(fc, "NFS: export pathname too long");
	return -ENAMETOOLONG;
}

static inline bool is_remount_fc(struct fs_context *fc)
{
	return fc->root != NULL;
}

/*
 * Parse monolithic NFS2/NFS3 mount data
 * - fills in the mount root filehandle
 *
 * For option strings, user space handles the following behaviors:
 *
 * + DNS: mapping server host name to IP address ("addr=" option)
 *
 * + failure mode: how to behave if a mount request can't be handled
 *   immediately ("fg/bg" option)
 *
 * + retry: how often to retry a mount request ("retry=" option)
 *
 * + breaking back: trying proto=udp after proto=tcp, v2 after v3,
 *   mountproto=tcp after mountproto=udp, and so on
 */
static int nfs23_parse_monolithic(struct fs_context *fc,
				  struct nfs_mount_data *data)
{
	struct nfs_fs_context *ctx = nfs_fc2context(fc);
	struct nfs_fh *mntfh = ctx->mntfh;
	struct sockaddr *sap = (struct sockaddr *)&ctx->nfs_server.address;
	int extra_flags = NFS_MOUNT_LEGACY_INTERFACE;
	int ret;

	if (data == NULL)
		goto out_no_data;

	ctx->version = NFS_DEFAULT_VERSION;
	switch (data->version) {
	case 1:
		data->namlen = 0;
		fallthrough;
	case 2:
		data->bsize = 0;
		fallthrough;
	case 3:
		if (data->flags & NFS_MOUNT_VER3)
			goto out_no_v3;
		data->root.size = NFS2_FHSIZE;
		memcpy(data->root.data, data->old_root.data, NFS2_FHSIZE);
		/* Turn off security negotiation */
		extra_flags |= NFS_MOUNT_SECFLAVOUR;
		fallthrough;
	case 4:
		if (data->flags & NFS_MOUNT_SECFLAVOUR)
			goto out_no_sec;
		fallthrough;
	case 5:
		memset(data->context, 0, sizeof(data->context));
		fallthrough;
	case 6:
		if (data->flags & NFS_MOUNT_VER3) {
			if (data->root.size > NFS3_FHSIZE || data->root.size == 0)
				goto out_invalid_fh;
			mntfh->size = data->root.size;
			ctx->version = 3;
		} else {
			mntfh->size = NFS2_FHSIZE;
			ctx->version = 2;
		}


		memcpy(mntfh->data, data->root.data, mntfh->size);
		if (mntfh->size < sizeof(mntfh->data))
			memset(mntfh->data + mntfh->size, 0,
			       sizeof(mntfh->data) - mntfh->size);

		/*
		 * for proto == XPRT_TRANSPORT_UDP, which is what uses
		 * to_exponential, implying shift: limit the shift value
		 * to BITS_PER_LONG (majortimeo is unsigned long)
		 */
		if (!(data->flags & NFS_MOUNT_TCP)) /* this will be UDP */
			if (data->retrans >= 64) /* shift value is too large */
				goto out_invalid_data;

		/*
		 * Translate to nfs_fs_context, which nfs_fill_super
		 * can deal with.
		 */
		ctx->flags	= data->flags & NFS_MOUNT_FLAGMASK;
		ctx->flags	|= extra_flags;
		ctx->rsize	= data->rsize;
		ctx->wsize	= data->wsize;
		ctx->timeo	= data->timeo;
		ctx->retrans	= data->retrans;
		ctx->acregmin	= data->acregmin;
		ctx->acregmax	= data->acregmax;
		ctx->acdirmin	= data->acdirmin;
		ctx->acdirmax	= data->acdirmax;
		ctx->need_mount	= false;

		memcpy(sap, &data->addr, sizeof(data->addr));
		ctx->nfs_server.addrlen = sizeof(data->addr);
		ctx->nfs_server.port = ntohs(data->addr.sin_port);
		if (sap->sa_family != AF_INET ||
		    !nfs_verify_server_address(sap))
			goto out_no_address;

		if (!(data->flags & NFS_MOUNT_TCP))
			ctx->nfs_server.protocol = XPRT_TRANSPORT_UDP;
		/* N.B. caller will free nfs_server.hostname in all cases */
		ctx->nfs_server.hostname = kstrdup(data->hostname, GFP_KERNEL);
		if (!ctx->nfs_server.hostname)
			goto out_nomem;

		ctx->namlen		= data->namlen;
		ctx->bsize		= data->bsize;

		if (data->flags & NFS_MOUNT_SECFLAVOUR)
			ctx->selected_flavor = data->pseudoflavor;
		else
			ctx->selected_flavor = RPC_AUTH_UNIX;

		if (!(data->flags & NFS_MOUNT_NONLM))
			ctx->flags &= ~(NFS_MOUNT_LOCAL_FLOCK|
					 NFS_MOUNT_LOCAL_FCNTL);
		else
			ctx->flags |= (NFS_MOUNT_LOCAL_FLOCK|
					NFS_MOUNT_LOCAL_FCNTL);

		/*
		 * The legacy version 6 binary mount data from userspace has a
		 * field used only to transport selinux information into the
		 * kernel.  To continue to support that functionality we
		 * have a touch of selinux knowledge here in the NFS code. The
		 * userspace code converted context=blah to just blah so we are
		 * converting back to the full string selinux understands.
		 */
		if (data->context[0]){
#ifdef CONFIG_SECURITY_SELINUX
			int ret;

			data->context[NFS_MAX_CONTEXT_LEN] = '\0';
			ret = vfs_parse_fs_string(fc, "context",
						  data->context, strlen(data->context));
			if (ret < 0)
				return ret;
#else
			return -EINVAL;
#endif
		}

		break;
	default:
		goto generic;
	}

	ret = nfs_validate_transport_protocol(fc, ctx);
	if (ret)
		return ret;

	ctx->skip_reconfig_option_check = true;
	return 0;

generic:
	return generic_parse_monolithic(fc, data);

out_no_data:
	if (is_remount_fc(fc)) {
		ctx->skip_reconfig_option_check = true;
		return 0;
	}
	return nfs_invalf(fc, "NFS: mount program didn't pass any mount data");

out_no_v3:
	return nfs_invalf(fc, "NFS: nfs_mount_data version does not support v3");

out_no_sec:
	return nfs_invalf(fc, "NFS: nfs_mount_data version supports only AUTH_SYS");

out_nomem:
	dfprintk(MOUNT, "NFS: not enough memory to handle mount options");
	return -ENOMEM;

out_no_address:
	return nfs_invalf(fc, "NFS: mount program didn't pass remote address");

out_invalid_fh:
	return nfs_invalf(fc, "NFS: invalid root filehandle");

out_invalid_data:
	return nfs_invalf(fc, "NFS: invalid binary mount data");
}

#if IS_ENABLED(CONFIG_NFS_V4)
struct compat_nfs_string {
	compat_uint_t len;
	compat_uptr_t data;
};

static inline void compat_nfs_string(struct nfs_string *dst,
				     struct compat_nfs_string *src)
{
	dst->data = compat_ptr(src->data);
	dst->len = src->len;
}

struct compat_nfs4_mount_data_v1 {
	compat_int_t version;
	compat_int_t flags;
	compat_int_t rsize;
	compat_int_t wsize;
	compat_int_t timeo;
	compat_int_t retrans;
	compat_int_t acregmin;
	compat_int_t acregmax;
	compat_int_t acdirmin;
	compat_int_t acdirmax;
	struct compat_nfs_string client_addr;
	struct compat_nfs_string mnt_path;
	struct compat_nfs_string hostname;
	compat_uint_t host_addrlen;
	compat_uptr_t host_addr;
	compat_int_t proto;
	compat_int_t auth_flavourlen;
	compat_uptr_t auth_flavours;
};

static void nfs4_compat_mount_data_conv(struct nfs4_mount_data *data)
{
	struct compat_nfs4_mount_data_v1 *compat =
			(struct compat_nfs4_mount_data_v1 *)data;

	/* copy the fields backwards */
	data->auth_flavours = compat_ptr(compat->auth_flavours);
	data->auth_flavourlen = compat->auth_flavourlen;
	data->proto = compat->proto;
	data->host_addr = compat_ptr(compat->host_addr);
	data->host_addrlen = compat->host_addrlen;
	compat_nfs_string(&data->hostname, &compat->hostname);
	compat_nfs_string(&data->mnt_path, &compat->mnt_path);
	compat_nfs_string(&data->client_addr, &compat->client_addr);
	data->acdirmax = compat->acdirmax;
	data->acdirmin = compat->acdirmin;
	data->acregmax = compat->acregmax;
	data->acregmin = compat->acregmin;
	data->retrans = compat->retrans;
	data->timeo = compat->timeo;
	data->wsize = compat->wsize;
	data->rsize = compat->rsize;
	data->flags = compat->flags;
	data->version = compat->version;
}

/*
 * Validate NFSv4 mount options
 */
static int nfs4_parse_monolithic(struct fs_context *fc,
				 struct nfs4_mount_data *data)
{
	struct nfs_fs_context *ctx = nfs_fc2context(fc);
	struct sockaddr *sap = (struct sockaddr *)&ctx->nfs_server.address;
	int ret;
	char *c;

	if (!data) {
		if (is_remount_fc(fc))
			goto done;
		return nfs_invalf(fc,
			"NFS4: mount program didn't pass any mount data");
	}

	ctx->version = 4;

	if (data->version != 1)
		return generic_parse_monolithic(fc, data);

	if (in_compat_syscall())
		nfs4_compat_mount_data_conv(data);

	if (data->host_addrlen > sizeof(ctx->nfs_server.address))
		goto out_no_address;
	if (data->host_addrlen == 0)
		goto out_no_address;
	ctx->nfs_server.addrlen = data->host_addrlen;
	if (copy_from_user(sap, data->host_addr, data->host_addrlen))
		return -EFAULT;
	if (!nfs_verify_server_address(sap))
		goto out_no_address;
	ctx->nfs_server.port = ntohs(((struct sockaddr_in *)sap)->sin_port);

	if (data->auth_flavourlen) {
		rpc_authflavor_t pseudoflavor;

		if (data->auth_flavourlen > 1)
			goto out_inval_auth;
		if (copy_from_user(&pseudoflavor, data->auth_flavours,
				   sizeof(pseudoflavor)))
			return -EFAULT;
		ctx->selected_flavor = pseudoflavor;
	} else {
		ctx->selected_flavor = RPC_AUTH_UNIX;
	}

	c = strndup_user(data->hostname.data, NFS4_MAXNAMLEN);
	if (IS_ERR(c))
		return PTR_ERR(c);
	ctx->nfs_server.hostname = c;

	c = strndup_user(data->mnt_path.data, NFS4_MAXPATHLEN);
	if (IS_ERR(c))
		return PTR_ERR(c);
	ctx->nfs_server.export_path = c;
	dfprintk(MOUNT, "NFS: MNTPATH: '%s'\n", c);

	c = strndup_user(data->client_addr.data, 16);
	if (IS_ERR(c))
		return PTR_ERR(c);
	ctx->client_address = c;

	/*
	 * Translate to nfs_fs_context, which nfs_fill_super
	 * can deal with.
	 */

	ctx->flags	= data->flags & NFS4_MOUNT_FLAGMASK;
	ctx->rsize	= data->rsize;
	ctx->wsize	= data->wsize;
	ctx->timeo	= data->timeo;
	ctx->retrans	= data->retrans;
	ctx->acregmin	= data->acregmin;
	ctx->acregmax	= data->acregmax;
	ctx->acdirmin	= data->acdirmin;
	ctx->acdirmax	= data->acdirmax;
	ctx->nfs_server.protocol = data->proto;
	ret = nfs_validate_transport_protocol(fc, ctx);
	if (ret)
		return ret;
done:
	ctx->skip_reconfig_option_check = true;
	return 0;

out_inval_auth:
	return nfs_invalf(fc, "NFS4: Invalid number of RPC auth flavours %d",
		      data->auth_flavourlen);

out_no_address:
	return nfs_invalf(fc, "NFS4: mount program didn't pass remote address");
}
#endif

/*
 * Parse a monolithic block of data from sys_mount().
 */
static int nfs_fs_context_parse_monolithic(struct fs_context *fc,
					   void *data)
{
	if (fc->fs_type == &nfs_fs_type)
		return nfs23_parse_monolithic(fc, data);

#if IS_ENABLED(CONFIG_NFS_V4)
	if (fc->fs_type == &nfs4_fs_type)
		return nfs4_parse_monolithic(fc, data);
#endif

	return nfs_invalf(fc, "NFS: Unsupported monolithic data version");
}

/*
 * Validate the preparsed information in the config.
 */
static int nfs_fs_context_validate(struct fs_context *fc)
{
	struct nfs_fs_context *ctx = nfs_fc2context(fc);
	struct nfs_subversion *nfs_mod;
	struct sockaddr *sap = (struct sockaddr *)&ctx->nfs_server.address;
	int max_namelen = PAGE_SIZE;
	int max_pathlen = NFS_MAXPATHLEN;
	int port = 0;
	int ret;

	if (ctx->version < 4) {
		ctx->xprt_id = 0;
		if (always_nosharetransport)
			ctx->flags |= NFS_MOUNT_NOSHARE_XPRT;
	} else if (ctx->flags & NFS_MOUNT_NOSHARE_XPRT)
		goto out_noshare_misuse;

	if (!fc->source)
		goto out_no_device_name;

	/* Check for sanity first. */
	if (ctx->minorversion && ctx->version != 4)
		goto out_minorversion_mismatch;

	if (ctx->options & NFS_OPTION_MIGRATION &&
	    (ctx->version != 4 || ctx->minorversion != 0))
		goto out_migration_misuse;

	/* Verify that any proto=/mountproto= options match the address
	 * families in the addr=/mountaddr= options.
	 */
	if (ctx->protofamily != AF_UNSPEC &&
	    ctx->protofamily != ctx->nfs_server.address.sa_family)
		goto out_proto_mismatch;

	if (ctx->mountfamily != AF_UNSPEC) {
		if (ctx->mount_server.addrlen) {
			if (ctx->mountfamily != ctx->mount_server.address.sa_family)
				goto out_mountproto_mismatch;
		} else {
			if (ctx->mountfamily != ctx->nfs_server.address.sa_family)
				goto out_mountproto_mismatch;
		}
	}

	if (!nfs_verify_server_address(sap))
		goto out_no_address;

	ret = nfs_validate_transport_protocol(fc, ctx);
	if (ret)
		return ret;

	if (ctx->version == 4) {
		if (IS_ENABLED(CONFIG_NFS_V4)) {
			if (ctx->nfs_server.protocol == XPRT_TRANSPORT_RDMA)
				port = NFS_RDMA_PORT;
			else
				port = NFS_PORT;
			max_namelen = NFS4_MAXNAMLEN;
			max_pathlen = NFS4_MAXPATHLEN;
			ctx->flags &= ~(NFS_MOUNT_NONLM | NFS_MOUNT_NOACL |
					NFS_MOUNT_VER3 | NFS_MOUNT_LOCAL_FLOCK |
					NFS_MOUNT_LOCAL_FCNTL);
		} else {
			goto out_v4_not_compiled;
		}
	} else {
		nfs_set_mount_transport_protocol(ctx);
		if (ctx->nfs_server.protocol == XPRT_TRANSPORT_RDMA)
			port = NFS_RDMA_PORT;
	}

	nfs_set_port(sap, &ctx->nfs_server.port, port);

	ret = nfs_parse_source(fc, max_namelen, max_pathlen);
	if (ret < 0)
		return ret;

	if (ctx->remote_ports && ctx->remote_ports->dns) {
		ret = nfs_get_dns_remoteports(fc);
		if (ret)
			return ret;
	}

	/* Load the NFS protocol module if we haven't done so yet */
	if (!ctx->nfs_mod) {
		nfs_mod = get_nfs_version(ctx->version);
		if (IS_ERR(nfs_mod)) {
			ret = PTR_ERR(nfs_mod);
			goto out_version_unavailable;
		}
		ctx->nfs_mod = nfs_mod;
	}

	/* Ensure the filesystem context has the correct fs_type */
	if (fc->fs_type != ctx->nfs_mod->nfs_fs) {
		module_put(fc->fs_type->owner);
		__module_get(ctx->nfs_mod->nfs_fs->owner);
		fc->fs_type = ctx->nfs_mod->nfs_fs;
	}
	return 0;

out_noshare_misuse:
	return nfs_invalf(fc, "NFS: nosharetransport is not compatible "
				"with vers=4\nNFS: use sharetransport=N "
				"for some unique positive number N");
out_no_device_name:
	return nfs_invalf(fc, "NFS: Device name not specified");
out_v4_not_compiled:
	nfs_errorf(fc, "NFS: NFSv4 is not compiled into kernel");
	return -EPROTONOSUPPORT;
out_no_address:
	return nfs_invalf(fc, "NFS: mount program didn't pass remote address");
out_mountproto_mismatch:
	return nfs_invalf(fc, "NFS: Mount server address does not match mountproto= option");
out_proto_mismatch:
	return nfs_invalf(fc, "NFS: Server address does not match proto= option");
out_minorversion_mismatch:
	return nfs_invalf(fc, "NFS: Mount option vers=%u does not support minorversion=%u",
			  ctx->version, ctx->minorversion);
out_migration_misuse:
	return nfs_invalf(fc, "NFS: 'Migration' not supported for this NFS version");
out_version_unavailable:
	nfs_errorf(fc, "NFS: Version unavailable");
	return ret;
}

/*
 * Create an NFS superblock by the appropriate method.
 */
static int nfs_get_tree(struct fs_context *fc)
{
	struct nfs_fs_context *ctx = nfs_fc2context(fc);
	int err = nfs_fs_context_validate(fc);

	if (err)
		return err;
	if (!ctx->internal)
		return ctx->nfs_mod->rpc_ops->try_get_tree(fc);
	else
		return nfs_get_tree_common(fc);
}

/*
 * Handle duplication of a configuration.  The caller copied *src into *sc, but
 * it can't deal with resource pointers in the filesystem context, so we have
 * to do that.  We need to clear pointers, copy data or get extra refs as
 * appropriate.
 */
static int nfs_fs_context_dup(struct fs_context *fc, struct fs_context *src_fc)
{
	struct nfs_fs_context *src = nfs_fc2context(src_fc), *ctx;
	int err;

	ctx = kmemdup(src, sizeof(struct nfs_fs_context), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	err = nfs_fs_context_alloc_trid(ctx);
	if (err < 0) {
		kfree(ctx);
		return err;
	}

	ctx->mntfh = nfs_alloc_fhandle();
	if (!ctx->mntfh) {
		nfs_fs_context_free_trid(ctx);
		kfree(ctx);
		return -ENOMEM;
	}
	nfs_copy_fh(ctx->mntfh, src->mntfh);

	__module_get(ctx->nfs_mod->owner);
	ctx->local_ports		= NULL;
	ctx->remote_ports		= NULL;
	ctx->client_address		= NULL;
	ctx->mount_server.hostname	= NULL;
	ctx->nfs_server.export_path	= NULL;
	ctx->nfs_server.hostname	= NULL;
	ctx->fscache_uniq		= NULL;
	ctx->clone_data.fattr		= NULL;
	fc->fs_private = ctx;

	trace_nfs_fs_context_dup(ctx, src);
	return 0;
}

static void nfs_fs_context_free(struct fs_context *fc)
{
	struct nfs_fs_context *ctx = nfs_fc2context(fc);

	if (ctx) {
		trace_nfs_fs_context_free_start(ctx);
		if (ctx->server)
			nfs_free_server(ctx->server);
		if (ctx->nfs_mod)
			put_nfs_version(ctx->nfs_mod);
		kfree(ctx->client_address);
		if (ctx->local_ports)
			vfree(ctx->local_ports);
		if (ctx->remote_ports)
			vfree(ctx->remote_ports);
		kfree(ctx->mount_server.hostname);
		kfree(ctx->nfs_server.export_path);
		kfree(ctx->nfs_server.hostname);
		kfree(ctx->fscache_uniq);
		nfs_free_fhandle(ctx->mntfh);
		nfs_free_fattr(ctx->clone_data.fattr);
		trace_nfs_fs_context_free_done(ctx);
		nfs_fs_context_free_trid(ctx);
		kfree(ctx);
	}
}

static const struct fs_context_operations nfs_fs_context_ops = {
	.free			= nfs_fs_context_free,
	.dup			= nfs_fs_context_dup,
	.parse_param		= nfs_fs_context_parse_param,
	.parse_monolithic	= nfs_fs_context_parse_monolithic,
	.get_tree		= nfs_get_tree,
	.reconfigure		= nfs_reconfigure,
};

/*
 * Prepare superblock configuration.  We use the namespaces attached to the
 * context.  This may be the current process's namespaces, or it may be a
 * container's namespaces.
 */
static int nfs_init_fs_context(struct fs_context *fc)
{
	struct nfs_fs_context *ctx;
	int err;

	ctx = kzalloc(sizeof(struct nfs_fs_context), GFP_KERNEL);
	if (unlikely(!ctx))
		return -ENOMEM;

	ctx->mntfh = nfs_alloc_fhandle();
	if (unlikely(!ctx->mntfh)) {
		kfree(ctx);
		return -ENOMEM;
	}

	err = nfs_fs_context_alloc_trid(ctx);
	if (unlikely(err < 0)) {
		nfs_free_fhandle(ctx->mntfh);
		kfree(ctx);
		return err;
	}

	ctx->protofamily	= AF_UNSPEC;
	ctx->mountfamily	= AF_UNSPEC;
	ctx->mount_server.port	= NFS_UNSPEC_PORT;

	if (fc->root) {
		/* reconfigure, start with the current config */
		struct nfs_server *nfss = fc->root->d_sb->s_fs_info;
		struct net *net = nfss->nfs_client->cl_net;

		ctx->flags		= nfss->flags;
		ctx->rsize		= nfss->rsize;
		ctx->wsize		= nfss->wsize;
		ctx->retrans		= nfss->client->cl_timeout->to_retries;
		ctx->selected_flavor	= nfss->client->cl_auth->au_flavor;
		ctx->acregmin		= nfss->acregmin / HZ;
		ctx->acregmax		= nfss->acregmax / HZ;
		ctx->acdirmin		= nfss->acdirmin / HZ;
		ctx->acdirmax		= nfss->acdirmax / HZ;
		ctx->timeo		= 10U * nfss->client->cl_timeout->to_initval / HZ;
		ctx->nfs_server.port	= nfss->port;
		ctx->nfs_server.addrlen	= nfss->nfs_client->cl_addrlen;
		ctx->version		= nfss->nfs_client->rpc_ops->version;
		ctx->minorversion	= nfss->nfs_client->cl_minorversion;

		memcpy(&ctx->nfs_server.address, &nfss->nfs_client->cl_addr,
			ctx->nfs_server.addrlen);

		if (fc->net_ns != net) {
			put_net(fc->net_ns);
			fc->net_ns = get_net(net);
		}

		ctx->nfs_mod = nfss->nfs_client->cl_nfs_mod;
		__module_get(ctx->nfs_mod->owner);
	} else {
		/* defaults */
		ctx->timeo		= NFS_UNSPEC_TIMEO;
		ctx->retrans		= NFS_UNSPEC_RETRANS;
		ctx->acregmin		= NFS_DEF_ACREGMIN;
		ctx->acregmax		= NFS_DEF_ACREGMAX;
		ctx->acdirmin		= NFS_DEF_ACDIRMIN;
		ctx->acdirmax		= NFS_DEF_ACDIRMAX;
		ctx->nfs_server.port	= NFS_UNSPEC_PORT;
		ctx->nfs_server.protocol = XPRT_TRANSPORT_TCP;
		ctx->selected_flavor	= RPC_AUTH_MAXFLAVOR;
		ctx->minorversion	= 0;
		ctx->need_mount		= true;

#if !defined(COMPAT_DETECT_INCLUDE_FS_CONTEXT) || defined(COMPAT_DETECT_FS_CONTEXT_S_IFLAGS)
		fc->s_iflags		|= SB_I_STABLE_WRITES;
#endif
		/*
		 * Modified defaults
		 *
		 * In 2022, the client roundtrip is more expensive than the internal
		 * server roundrip to bring in the extra data.
		 */
		ctx->flags              = NFS_MOUNT_FORCE_READDIRPLUS;
	}
	fc->fs_private = ctx;
	fc->ops = &nfs_fs_context_ops;
	trace_nfs_fs_context_new(ctx);

	return 0;
}

struct file_system_type nfs_fs_type = {
	.owner			= THIS_MODULE,
	.name			= "nfs",
#ifdef COMPAT_DETECT_INCLUDE_FS_CONTEXT
	.init_fs_context	= nfs_init_fs_context,
	.parameters             = NFS_FS_PARAMETERS_REF,
#else
	.mount                  = compat_emulate_fs_context_mount,
#endif
	.kill_sb		= nfs_kill_super,
	.fs_flags		= FS_RENAME_DOES_D_MOVE|FS_BINARY_MOUNTDATA,
};
COMPAT_FS_CONTEXT_DETECTOR_IMPL(nfs_fs_type, nfs_init_fs_context, NFS_FS_PARAMETERS_REF);

MODULE_ALIAS_FS("nfs");
EXPORT_SYMBOL_GPL(nfs_fs_type);

#if IS_ENABLED(CONFIG_NFS_V4)
struct file_system_type nfs4_fs_type = {
	.owner			= THIS_MODULE,
	.name			= "nfs4",
#ifdef COMPAT_DETECT_INCLUDE_FS_CONTEXT
	.init_fs_context	= nfs_init_fs_context,
	.parameters             = NFS_FS_PARAMETERS_REF,
#else
	.mount                  = compat_emulate_fs_context_mount,
#endif
	.kill_sb		= nfs_kill_super,
	.fs_flags		= FS_RENAME_DOES_D_MOVE|FS_BINARY_MOUNTDATA,
};
COMPAT_FS_CONTEXT_DETECTOR_IMPL(nfs4_fs_type, nfs_init_fs_context, NFS_FS_PARAMETERS_REF);

MODULE_ALIAS_FS("nfs4");
MODULE_ALIAS("nfs4");
EXPORT_SYMBOL_GPL(nfs4_fs_type);
#endif /* CONFIG_NFS_V4 */
