#ifndef __COMPAT_IVERSION_H__
#define __COMPAT_IVERSION_H__

#if defined(COMPAT_DETECT_INCLUDE_IVERSION)
#include_next <linux/iversion.h>
#else

#include <linux/fs.h>

/**
 * inode_set_iversion_raw - set i_version to the specified raw value
 * @inode: inode to set
 * @val: new i_version value to set
 *
 * Set @inode's i_version field to @val. This function is for use by
 * filesystems that self-manage the i_version.
 *
 * For example, the NFS client stores its NFSv4 change attribute in this way,
 * and the AFS client stores the data_version from the server here.
 */
static inline void
inode_set_iversion_raw(struct inode *inode, u64 val)
{
	inode->i_version = val;
}

/**
 * inode_peek_iversion_raw - grab a "raw" iversion value
 * @inode: inode from which i_version should be read
 *
 * Grab a "raw" inode->i_version value and return it. The i_version is not
 * flagged or converted in any way. This is mostly used to access a self-managed
 * i_version.
 *
 * With those filesystems, we want to treat the i_version as an entirely
 * opaque value.
 */
static inline u64
inode_peek_iversion_raw(const struct inode *inode)
{
	return inode->i_version;
}

/**
 * inode_eq_iversion_raw - check whether the raw i_version counter has changed
 * @inode: inode to check
 * @old: old value to check against its i_version
 *
 * Compare the current raw i_version counter with a previous one. Returns true
 * if they are the same or false if they are different.
 */
static inline bool
inode_eq_iversion_raw(const struct inode *inode, u64 old)
{
	return inode_peek_iversion_raw(inode) == old;
}

#define I_VERSION_QUERIED_SHIFT	(1)
#define I_VERSION_QUERIED	(1ULL << (I_VERSION_QUERIED_SHIFT - 1))
#define I_VERSION_INCREMENT	(1ULL << I_VERSION_QUERIED_SHIFT)

/**
 * inode_query_iversion - read i_version for later use
 * @inode: inode from which i_version should be read
 *
 * Read the inode i_version counter. This should be used by callers that wish
 * to store the returned i_version for later comparison. This will guarantee
 * that a later query of the i_version will result in a different value if
 * anything has changed.
 *
 * In this implementation, we fetch the current value, set the QUERIED flag and
 * then try to swap it into place with a cmpxchg, if it wasn't already set. If
 * that fails, we try again with the newly fetched value from the cmpxchg.
 */
static inline u64
inode_query_iversion(struct inode *inode)
{
	u64 cur, old, new;

	cur = inode_peek_iversion_raw(inode);
	for (;;) {
		/* If flag is already set, then no need to swap */
		if (cur & I_VERSION_QUERIED) {
			/*
			 * This barrier (and the implicit barrier in the
			 * cmpxchg below) pairs with the barrier in
			 * inode_maybe_inc_iversion().
			 */
			smp_mb();
			break;
		}

		new = cur | I_VERSION_QUERIED;
		old = cmpxchg(&inode->i_version, cur, new);
		if (likely(old == cur))
			break;
		cur = old;
	}
	return cur >> I_VERSION_QUERIED_SHIFT;
}

#endif

#ifndef COMPAT_DETECT_TIME_TO_CHATTR
/*
 * For filesystems without any sort of change attribute, the best we can
 * do is fake one up from the ctime:
 */
static inline u64 time_to_chattr(struct timespec64 *t)
{
	u64 chattr = t->tv_sec;

	chattr <<= 32;
	chattr += t->tv_nsec;
	return chattr;
}
#endif

#endif
