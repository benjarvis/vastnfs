#define TIMER_DATA_TYPE                unsigned long
#define TIMER_FUNC_TYPE                void (*)(TIMER_DATA_TYPE)

static inline void timer_set_function(struct timer_list *timer,
				      void (*callback)(struct timer_list *))
{
	timer->function = (TIMER_FUNC_TYPE)callback;
}
