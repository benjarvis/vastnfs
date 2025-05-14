#include <linux/cred.h>

#if !defined(COMPAT_DETECT_CRED_FSCMP)
/**
 * cred_fscmp - Compare two credentials with respect to filesystem access.
 * @a: The first credential
 * @b: The second credential
 *
 * cred_cmp() will return zero if both credentials have the same
 * fsuid, fsgid, and supplementary groups.  That is, if they will both
 * provide the same access to files based on mode/uid/gid.
 * If the credentials are different, then either -1 or 1 will
 * be returned depending on whether @a comes before or after @b
 * respectively in an arbitrary, but stable, ordering of credentials.
 *
 * Return: -1, 0, or 1 depending on comparison
 */
int cred_fscmp(const struct cred *a, const struct cred *b)
{
	struct group_info *ga, *gb;
	int g;

	if (a == b)
		return 0;
	if (uid_lt(a->fsuid, b->fsuid))
		return -1;
	if (uid_gt(a->fsuid, b->fsuid))
		return 1;

	if (gid_lt(a->fsgid, b->fsgid))
		return -1;
	if (gid_gt(a->fsgid, b->fsgid))
		return 1;

	ga = a->group_info;
	gb = b->group_info;
	if (ga == gb)
		return 0;
	if (ga == NULL)
		return -1;
	if (gb == NULL)
		return 1;
	if (ga->ngroups < gb->ngroups)
		return -1;
	if (ga->ngroups > gb->ngroups)
		return 1;

	for (g = 0; g < ga->ngroups; g++) {
		if (gid_lt(ga->gid[g], gb->gid[g]))
			return -1;
		if (gid_gt(ga->gid[g], gb->gid[g]))
			return 1;
	}
	return 0;
}
EXPORT_SYMBOL(cred_fscmp);
#endif

#if !defined(COMPAT_DETECT_GET_TASK_CRED)
/**
 * get_task_cred - Get another task's objective credentials
 * @task: The task to query
 *
 * Get the objective credentials of a task, pinning them so that they can't go
 * away.  Accessing a task's credentials directly is not permitted.
 *
 * The caller must also make sure task doesn't get deleted, either by holding a
 * ref on task or by holding tasklist_lock to prevent it from being unlinked.
 */
const struct cred *get_task_cred(struct task_struct *task)
{
        const struct cred *cred;

        rcu_read_lock();

        do {
                cred = __task_cred((task));
                BUG_ON(!cred);
        } while (!get_cred_rcu(cred));

        rcu_read_unlock();
        return cred;
}
EXPORT_SYMBOL(get_task_cred);
#endif

#if !defined(COMPAT_DETECT_GROUPS_SORT)
#include <linux/sort.h>

static int gid_cmp(const void *_a, const void *_b)
{
	kgid_t a = *(kgid_t *)_a;
	kgid_t b = *(kgid_t *)_b;

	return gid_gt(a, b) - gid_lt(a, b);
}

void groups_sort(struct group_info *group_info)
{
	sort(group_info->gid, group_info->ngroups, sizeof(*group_info->gid),
	     gid_cmp, NULL);
}
EXPORT_SYMBOL(groups_sort);
#endif
