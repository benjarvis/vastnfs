#ifndef __COMPAT_LINUX_FREEZER_H__
#define __COMPAT_LINUX_FREEZER_H__

#include_next <linux/freezer.h>

#if defined(COMPAT_DETECT_TASK_FREEZABLE_6_1)

static inline long freezable_schedule_timeout_interruptible_unsafe(long timeout)
{
	__set_current_state(TASK_INTERRUPTIBLE | TASK_FREEZABLE_UNSAFE);
	schedule_timeout(timeout);
	return 0;
}

static inline long freezable_schedule_timeout_interruptible(long timeout)
{
	__set_current_state(TASK_INTERRUPTIBLE | TASK_FREEZABLE);
	schedule_timeout(timeout);
	return 0;
}

static inline long freezable_schedule_timeout_killable_unsafe(long timeout)
{
	__set_current_state(TASK_KILLABLE | TASK_FREEZABLE_UNSAFE);
	schedule_timeout(timeout);
	return 0;
}

#define freezer_do_not_count() do { } while (0)
#define freezer_count() do { } while (0)
#define freezable_schedule_unsafe() schedule()

#else

#ifndef COMPAT_DETECT_FREEZABLE_SCHEDULE_TIMEOUT_INTERRUPTIBLE_UNSAFE

#ifdef CONFIG_FREEZER

/* DO NOT ADD ANY NEW CALLERS OF THIS FUNCTION */
static inline long freezable_schedule_timeout_interruptible_unsafe(long timeout)
{
	long __retval;

	freezer_do_not_count();
	__retval = schedule_timeout_interruptible(timeout);
	freezer_count_unsafe();
	return __retval;
}

#else

#define freezable_schedule_timeout_interruptible_unsafe(timeout)	\
	schedule_timeout_interruptible(timeout)

#endif
#endif

#endif /* ! COMPAT_DETECT_TASK_FREEZABLE_6_1 */

#endif
