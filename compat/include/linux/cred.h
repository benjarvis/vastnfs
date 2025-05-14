#ifndef __COMPAT_LINUX_CRED_H__
#define __COMPAT_LINUX_CRED_H__

#include_next <linux/cred.h>

#if !defined(COMPAT_DETECT_CRED_FSCMP)
extern int compat_cred_fscmp(const struct cred *a, const struct cred *b);
#define cred_fscmp compat_cred_fscmp
#endif

#if !defined(COMPAT_DETECT_GET_CRED_RCU)
static inline const struct cred *get_cred_rcu(const struct cred *cred)
{
	struct cred *nonconst_cred = (struct cred *) cred;
	if (!cred)
		return NULL;
#if defined(COMPAT_DETECT_CRED_ATOMIC_LONG_USAGE_6_8)
	if (!atomic64_inc_not_zero(&nonconst_cred->usage))
		return NULL;
#else
	if (!atomic_inc_not_zero(&nonconst_cred->usage))
		return NULL;
#endif
	validate_creds(cred);
#if defined(COMPAT_DETECT_GET_CRED_RCU_NON_RCU)
	nonconst_cred->non_rcu = 0;
#endif
	return cred;
}
#endif

#if !defined(COMPAT_DETECT_GET_TASK_CRED)

#define get_task_cred compat_get_task_cred
extern const struct cred *get_task_cred(struct task_struct *task);

static inline const struct cred *newer_get_cred(const struct cred *cred)
{
       if (!cred)
               return NULL;
       return get_cred(cred);
}

static inline void newer_put_cred(const struct cred *cred)
{
       if (!cred)
               return;
       put_cred(cred);
}

#define get_cred newer_get_cred
#define put_cred newer_put_cred

#endif

#if !defined(COMPAT_DETECT_GROUPS_SORT)
#define compat_groups_sort groups_sort
extern void groups_sort(struct group_info *group_info);
#endif

#endif
