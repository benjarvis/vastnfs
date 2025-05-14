#define ___wait_var_event(var, condition, state, exclusive, ret, cmd)	\
({									\
	__label__ __out;						\
	wait_queue_head_t *__wq_head = __var_waitqueue(var);		\
	struct wait_bit_queue __wbq_entry;				\
	long __ret = ret; /* explicit shadow */				\
									\
	init_wait_var_entry(&__wbq_entry, var,				\
			    exclusive ? WQ_FLAG_EXCLUSIVE : 0);		\
	for (;;) {							\
		if (exclusive)						\
			prepare_to_wait_exclusive(__wq_head,		\
						  &__wbq_entry.wait,	\
						  state);		\
		else							\
			prepare_to_wait(__wq_head, &__wbq_entry.wait,	\
					state);				\
									\
		if (condition)						\
			break;						\
									\
		if (___wait_signal_pending(state)) {			\
			__ret = -ERESTARTSYS;				\
			if (exclusive) {				\
				abort_exclusive_wait(__wq_head,		\
						     &__wbq_entry.wait,	\
						     state, NULL);	\
				goto __out;				\
			}						\
			break;						\
		}							\
									\
		cmd;							\
	}								\
	finish_wait(__wq_head, &__wbq_entry.wait);			\
__out:	__ret;								\
})

#define __wait_var_event(var, condition)				\
	___wait_var_event(var, condition, TASK_UNINTERRUPTIBLE, 0, 0,	\
			  schedule())

#define wait_var_event(var, condition)					\
do {									\
	might_sleep();							\
	if (condition)							\
		break;							\
	__wait_var_event(var, condition);				\
} while (0)

#define __wait_var_event_killable(var, condition)			\
	___wait_var_event(var, condition, TASK_KILLABLE, 0, 0,		\
			  schedule())

#define wait_var_event_killable(var, condition)				\
({									\
	int __ret = 0;							\
	might_sleep();							\
	if (!(condition))						\
		__ret = __wait_var_event_killable(var, condition);	\
	__ret;								\
})

#define __wait_var_event_timeout(var, condition, timeout)		\
	___wait_var_event(var, ___wait_cond_timeout(condition),		\
			  TASK_UNINTERRUPTIBLE, 0, timeout,		\
			  __ret = schedule_timeout(__ret))

#define wait_var_event_timeout(var, condition, timeout)			\
({									\
	long __ret = timeout;						\
	might_sleep();							\
	if (!___wait_cond_timeout(condition))				\
		__ret = __wait_var_event_timeout(var, condition, timeout); \
	__ret;								\
})

#define WAIT_TABLE_BITS 8
#define WAIT_TABLE_SIZE (1 << WAIT_TABLE_BITS)

extern wait_queue_head_t bit_wait_table[WAIT_TABLE_SIZE] __cacheline_aligned;

static inline wait_queue_head_t *__var_waitqueue(void *p)
{
	return bit_wait_table + hash_ptr(p, WAIT_TABLE_BITS);
}

static inline void wake_up_var(void *var)
{
	__wake_up_bit(__var_waitqueue(var), var, -1);
}

static inline int
var_wake_function(wait_queue_t *wq_entry, unsigned int mode,
		  int sync, void *arg)
{
	struct wait_bit_key *key = arg;
	struct wait_bit_queue *wbq_entry =
		container_of(wq_entry, struct wait_bit_queue, wait);

	if (wbq_entry->key.flags != key->flags ||
	    wbq_entry->key.bit_nr != key->bit_nr)
		return 0;

	return autoremove_wake_function(wq_entry, mode, sync, key);
}

static inline void init_wait_var_entry(struct wait_bit_queue *wbq_entry, void *var, int flags)
{
	*wbq_entry = (struct wait_bit_queue){
		.key = {
			.flags	= (var),
			.bit_nr = -1,
		},
		.wait = {
			.private = current,
			.func	 = var_wake_function,
			.task_list = LIST_HEAD_INIT(wbq_entry->wait.task_list),
		},
	};
}
