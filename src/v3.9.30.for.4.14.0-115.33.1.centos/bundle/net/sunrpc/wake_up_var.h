#define ___wait_var_event(var, condition, state, exclusive, ret, cmd)  \
({                                                                     \
       __label__ __out;                                                \
       struct wait_queue_head *__wq_head = __var_waitqueue(var);       \
       struct wait_bit_queue_entry __wbq_entry;                        \
       long __ret = ret; /* explicit shadow */                         \
                                                                       \
       init_wait_var_entry(&__wbq_entry, var,                          \
                           exclusive ? WQ_FLAG_EXCLUSIVE : 0);         \
       for (;;) {                                                      \
               long __int = prepare_to_wait_event(__wq_head,           \
                                                  &__wbq_entry.wq_entry, \
                                                  state);              \
               if (condition)                                          \
                       break;                                          \
                                                                       \
               if (___wait_is_interruptible(state) && __int) {         \
                       __ret = __int;                                  \
                       goto __out;                                     \
               }                                                       \
                                                                       \
               cmd;                                                    \
       }                                                               \
       finish_wait(__wq_head, &__wbq_entry.wq_entry);                  \
__out: __ret;                                                          \
})

#define __wait_var_event(var, condition)                               \
       ___wait_var_event(var, condition, TASK_UNINTERRUPTIBLE, 0, 0,   \
                         schedule())

#define wait_var_event(var, condition)                                 \
do {                                                                   \
       might_sleep();                                                  \
       if (condition)                                                  \
               break;                                                  \
       __wait_var_event(var, condition);                               \
} while (0)

#define __wait_var_event_killable(var, condition)                      \
       ___wait_var_event(var, condition, TASK_KILLABLE, 0, 0,          \
                         schedule())

#define wait_var_event_killable(var, condition)                                \
({                                                                     \
       int __ret = 0;                                                  \
       might_sleep();                                                  \
       if (!(condition))                                               \
               __ret = __wait_var_event_killable(var, condition);      \
       __ret;                                                          \
})

#define __wait_var_event_timeout(var, condition, timeout)              \
       ___wait_var_event(var, ___wait_cond_timeout(condition),         \
                         TASK_UNINTERRUPTIBLE, 0, timeout,             \
                         __ret = schedule_timeout(__ret))

#define wait_var_event_timeout(var, condition, timeout)                        \
({                                                                     \
       long __ret = timeout;                                           \
       might_sleep();                                                  \
       if (!___wait_cond_timeout(condition))                           \
               __ret = __wait_var_event_timeout(var, condition, timeout); \
       __ret;                                                          \
})

wait_queue_head_t *__var_waitqueue(void *p)
{
       if (BITS_PER_LONG == 64) {
               unsigned long q = (unsigned long)p;

               return bit_waitqueue((void *)(q & ~1), q & 1);
       }
       return bit_waitqueue(p, 0);
}

static int
var_wake_function(struct wait_queue_entry *wq_entry, unsigned int mode,
                 int sync, void *arg)
{
       struct wait_bit_key *key = arg;
       struct wait_bit_queue_entry *wbq_entry =
               container_of(wq_entry, struct wait_bit_queue_entry, wq_entry);

       if (wbq_entry->key.flags != key->flags ||
           wbq_entry->key.bit_nr != key->bit_nr)
               return 0;

       return autoremove_wake_function(wq_entry, mode, sync, key);
}

static void init_wait_var_entry(struct wait_bit_queue_entry *wbq_entry, void *var, int flags)
{
       *wbq_entry = (struct wait_bit_queue_entry){
               .key = {
                       .flags  = (var),
                       .bit_nr = -1,
               },
               .wq_entry = {
                       .private = current,
                       .func    = var_wake_function,
                       .entry   = LIST_HEAD_INIT(wbq_entry->wq_entry.entry),
               },
       };
}

static void wake_up_var(void *var)
{
       __wake_up_bit(__var_waitqueue(var), var, -1);
}
