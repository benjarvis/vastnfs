#ifndef __COMPAT_LINUX_SCHED_H__
#define __COMPAT_LINUX_SCHED_H__

#include_next <linux/sched.h>
#include <linux/pid.h>

#ifndef COMPAT_DETECT_PF_LOCAL_THROTTLE
#define PF_LOCAL_THROTTLE PF_LESS_THROTTLE
#endif

#if !defined(COMPAT_DETECT_FIND_TASK_BY_PID_NS)
static inline struct task_struct *unexported_find_task_by_pid_ns(
	pid_t nr, struct pid_namespace *ns)
{
	return pid_task(find_pid_ns(nr, ns), PIDTYPE_PID);
}

#define find_task_by_pid_ns unexported_find_task_by_pid_ns
#endif

#if !defined(COMPAT_DETECT_TASK_FREEZABLE_6_1)

#define TASK_FREEZABLE 0
#define TASK_FREEZABLE_UNSAFE 0

#endif /* ! COMPAT_DETECT_TASK_FREEZABLE_6_1 */


#endif
