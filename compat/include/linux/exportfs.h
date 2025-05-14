#ifndef __COMPAT_LINUX_EXPORTFS_H__
#define __COMPAT_LINUX_EXPORTFS_H__

#include_next <linux/exportfs.h>

#ifndef EXPORT_OP_NOWCC
#define	EXPORT_OP_NOWCC			(0x1) /* don't collect v3 wcc data */
#endif

#ifndef EXPORT_OP_NOSUBTREECHK
#define	EXPORT_OP_NOSUBTREECHK		(0x2) /* no subtree checking */
#endif

#ifndef EXPORT_OP_CLOSE_BEFORE_UNLINK
#define	EXPORT_OP_CLOSE_BEFORE_UNLINK	(0x4) /* close files before unlink */
#endif

#ifndef EXPORT_OP_REMOTE_FS
#define EXPORT_OP_REMOTE_FS		(0x8) /* Filesystem is remote */
#endif

#ifndef EXPORT_OP_NOATOMIC_ATTR
#define EXPORT_OP_NOATOMIC_ATTR		(0x10)
#endif

#ifndef EXPORT_OP_SYNC_LOCKS
#define EXPORT_OP_SYNC_LOCKS		(0x20)
#endif

#ifdef COMPAT_DETECT_EXPORTFS_DECODE_FH_RAW
#define exportfs_decode_fh_raw_compat exportfs_decode_fh_raw
#else
#define exportfs_decode_fh_raw_compat exportfs_decode_fh
#endif

#ifdef COMPAT_DETECT_EXPORTFS_FLAGS
static inline unsigned long export_operations_flags(const struct export_operations *eo)
{
	return eo->flags;
}
#else
static inline unsigned long export_operations_flags(const struct export_operations *eo)
{
	return 0;
}
#endif

#endif
