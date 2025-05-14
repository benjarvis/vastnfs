#ifndef __COMPAT_LINUX_TIMER_H__
#define __COMPAT_LINUX_TIMER_H__

#include_next <linux/timer.h>

#if defined(COMPAT_DETECT_TIMER_LIST_DATA)

#if !defined(from_timer)

#define TIMER_DATA_TYPE                unsigned long
#define TIMER_FUNC_TYPE                void (*)(TIMER_DATA_TYPE)

static inline void timer_setup(struct timer_list *timer,
                              void (*callback)(struct timer_list *),
                              unsigned int flags)
{
       __setup_timer(timer, (TIMER_FUNC_TYPE)callback,
                     (TIMER_DATA_TYPE)timer, flags);
}

#define from_timer(var, callback_timer, timer_fieldname) \
       container_of(callback_timer, typeof(*var), timer_fieldname)

#endif

static inline void timer_set_function(struct timer_list *timer,
                                     void (*callback)(struct timer_list *))
{
       timer->function = (TIMER_FUNC_TYPE)callback;
}

#else

static inline void timer_set_function(struct timer_list *timer,
				      void (*callback)(struct timer_list *))
{
       timer->function = callback;
}

#endif

#endif
