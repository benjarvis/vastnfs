#ifndef __COMPAT_SEQLOCK_H__
#define __COMPAT_SEQLOCK_H__

#include_next <linux/seqlock.h>

#ifndef COMPAT_DETECT_SEQCOUNT_SPINLOCK

#include <linux/spinlock.h>
#include <linux/kconfig.h>

// #include <linux/kcsan-checks.h>
#define kcsan_nestable_atomic_begin()
#define kcsan_nestable_atomic_end()
#define kcsan_atomic_next(x)

#define seqcount_LOCKNAME_init(s, _lock, lockname)			\
	do {								\
		seqcount_##lockname##_t *____s = (s);			\
		seqcount_init(&____s->seqcount);			\
		__SEQ_LOCK(____s->lock = (_lock));			\
	} while (0)

#define seqcount_spinlock_init(s, lock)		seqcount_LOCKNAME_init(s, lock, spinlock)

/*
 * SEQCOUNT_LOCKNAME()	- Instantiate seqcount_LOCKNAME_t and helpers
 * seqprop_LOCKNAME_*()	- Property accessors for seqcount_LOCKNAME_t
 *
 * @lockname:		"LOCKNAME" part of seqcount_LOCKNAME_t
 * @locktype:		LOCKNAME canonical C data type
 * @preemptible:	preemptibility of above locktype
 * @lockmember:		argument for lockdep_assert_held()
 * @lockbase:		associated lock release function (prefix only)
 * @lock_acquire:	associated lock acquisition function (full call)
 */
#define SEQCOUNT_LOCKNAME(lockname, locktype, preemptible, lockmember, lockbase, lock_acquire) \
typedef struct seqcount_##lockname {					\
	seqcount_t		seqcount;				\
	__SEQ_LOCK(locktype	*lock);					\
} seqcount_##lockname##_t;						\
									\
static __always_inline seqcount_t *					\
__seqprop_##lockname##_ptr(seqcount_##lockname##_t *s)			\
{									\
	return &s->seqcount;						\
}									\
									\
static __always_inline unsigned						\
__seqprop_##lockname##_sequence(const seqcount_##lockname##_t *s)	\
{									\
	unsigned seq = READ_ONCE(s->seqcount.sequence);			\
									\
	if (!IS_ENABLED(CONFIG_PREEMPT_RT))				\
		return seq;						\
									\
	if (preemptible && unlikely(seq & 1)) {				\
		__SEQ_LOCK(lock_acquire);				\
		__SEQ_LOCK(lockbase##_unlock(s->lock));			\
									\
		/*							\
		 * Re-read the sequence counter since the (possibly	\
		 * preempted) writer made progress.			\
		 */							\
		seq = READ_ONCE(s->seqcount.sequence);			\
	}								\
									\
	return seq;							\
}									\
									\
static __always_inline bool						\
__seqprop_##lockname##_preemptible(const seqcount_##lockname##_t *s)	\
{									\
	if (!IS_ENABLED(CONFIG_PREEMPT_RT))				\
		return preemptible;					\
									\
	/* PREEMPT_RT relies on the above LOCK+UNLOCK */		\
	return false;							\
}									\
									\
static __always_inline void						\
__seqprop_##lockname##_assert(const seqcount_##lockname##_t *s)		\
{									\
	__SEQ_LOCK(lockdep_assert_held(lockmember));			\
}

#define __SEQ_RT	IS_ENABLED(CONFIG_PREEMPT_RT)

#if defined(CONFIG_LOCKDEP) || defined(CONFIG_PREEMPT_RT)
#define __SEQ_LOCK(expr)	expr
#else
#define __SEQ_LOCK(expr)
#endif

static inline seqcount_t *__seqprop_ptr(seqcount_t *s)
{
	return s;
}


static inline bool __seqprop_preemptible(const seqcount_t *s)
{
	return false;
}

static inline unsigned __seqprop_sequence(const seqcount_t *s)
{
	return READ_ONCE(s->sequence);
}

SEQCOUNT_LOCKNAME(spinlock,     spinlock_t,      __SEQ_RT, s->lock,        spin,     spin_lock(s->lock))

/**
 * raw_write_seqcount_begin() - start a seqcount_t write section w/o lockdep
 * @s: Pointer to seqcount_t or any of the seqcount_LOCKNAME_t variants
 */
#define compat_raw_write_seqcount_begin(s)				\
do {									\
	if (__seqcount_lock_preemptible(s))				\
		preempt_disable();					\
									\
	raw_write_seqcount_t_begin(__seqcount_ptr(s));			\
} while (0)

static inline void raw_write_seqcount_t_end(seqcount_t *s)
{
	smp_wmb();
	s->sequence++;
	kcsan_nestable_atomic_end();
}

/**
 * raw_write_seqcount_end() - end a seqcount_t write section w/o lockdep
 * @s: Pointer to seqcount_t or any of the seqcount_LOCKNAME_t variants
 */
#define compat_raw_write_seqcount_end(s)					\
do {									\
	raw_write_seqcount_t_end(__seqcount_ptr(s));			\
									\
	if (__seqcount_lock_preemptible(s))				\
		preempt_enable();					\
} while (0)

#define __seqprop_case(s, lockname, prop)				\
	seqcount_##lockname##_t: __seqprop_##lockname##_##prop((void *)(s))

#define __seqprop(s, prop) _Generic(*(s),				\
	seqcount_t:		__seqprop_##prop((void *)(s)),		\
	__seqprop_case((s),	spinlock,	prop))

#define __seqcount_ptr(s)		__seqprop(s, ptr)
#define __seqcount_sequence(s)		__seqprop(s, sequence)
#define __seqcount_lock_preemptible(s)	__seqprop(s, preemptible)
#define __seqcount_assert_lock_held(s)	__seqprop(s, assert)

static inline void raw_write_seqcount_t_begin(seqcount_t *s)
{
	kcsan_nestable_atomic_begin();
	s->sequence++;
	smp_wmb();
}

#define compat_raw_seqcount_begin(s)						\
({									\
	/*								\
	 * If the counter is odd, let read_seqcount_retry() fail	\
	 * by decrementing the counter.					\
	 */								\
	raw_read_seqcount(s) & ~1;					\
})

static inline int __read_seqcount_t_retry(const seqcount_t *s, unsigned start)
{
	kcsan_atomic_next(0);
	return unlikely(READ_ONCE(s->sequence) != start);
}

static inline int read_seqcount_t_retry(const seqcount_t *s, unsigned start)
{
	smp_rmb();
	return __read_seqcount_t_retry(s, start);
}

/**
 * read_seqcount_retry() - end a seqcount_t read critical section
 * @s: Pointer to seqcount_t or any of the seqcount_LOCKNAME_t variants
 * @start: count, from read_seqcount_begin()
 *
 * read_seqcount_retry closes the read critical section of given
 * seqcount_t.  If the critical section was invalid, it must be ignored
 * (and typically retried).
 *
 * Return: true if a read section retry is required, else false
 */
#define read_seqcount_retry(s, start)					\
	read_seqcount_t_retry(__seqcount_ptr(s), start)

/**
 * raw_read_seqcount() - read the raw seqcount_t counter value
 * @s: Pointer to seqcount_t or any of the seqcount_LOCKNAME_t variants
 *
 * raw_read_seqcount opens a read critical section of the given
 * seqcount_t, without any lockdep checking, and without checking or
 * masking the sequence counter LSB. Calling code is responsible for
 * handling that.
 *
 * Return: count to be passed to read_seqcount_retry()
 */
#define raw_read_seqcount(s)						\
({									\
	unsigned seq = __seqcount_sequence(s);				\
									\
	smp_rmb();							\
	kcsan_atomic_next(KCSAN_SEQLOCK_REGION_MAX);			\
	seq;								\
})

/**
 * raw_seqcount_begin() - begin a seqcount_t read critical section w/o
 *                        lockdep and w/o counter stabilization
 * @s: Pointer to seqcount_t or any of the seqcount_LOCKNAME_t variants
 *
 * raw_seqcount_begin opens a read critical section of the given
 * seqcount_t. Unlike read_seqcount_begin(), this function will not wait
 * for the count to stabilize. If a writer is active when it begins, it
 * will fail the read_seqcount_retry() at the end of the read critical
 * section instead of stabilizing at the beginning of it.
 *
 * Use this only in special kernel hot paths where the read section is
 * small and has a high probability of success through other external
 * means. It will save a single branching instruction.
 *
 * Return: count to be passed to read_seqcount_retry()
 */
#define raw_seqcount_begin(s)						\
({									\
	/*								\
	 * If the counter is odd, let read_seqcount_retry() fail	\
	 * by decrementing the counter.					\
	 */								\
	raw_read_seqcount(s) & ~1;					\
})

#else

#define compat_raw_write_seqcount_begin raw_write_seqcount_begin
#define compat_raw_write_seqcount_end raw_write_seqcount_end
#define compat_raw_seqcount_begin raw_seqcount_begin

#endif

#endif
