#ifndef __SYS_LOCK_H__
#define __SYS_LOCK_H__

/* dummy lock routines for single-threaded aps */

#include <newlib.h>
#include <_ansi.h>
#include <sys/cdefs.h>

#if !defined(_RETARGETABLE_LOCKING)

typedef int _LOCK_T;
typedef int _LOCK_RECURSIVE_T;

#define __LOCK_INIT(class,lock) static int lock = 0;
#define __LOCK_INIT_RECURSIVE(class,lock) static int lock = 0;
#define __lock_init(lock) ((void) 0)
#define __lock_init_recursive(lock) ((void) 0)
#define __lock_close(lock) ((void) 0)
#define __lock_close_recursive(lock) ((void) 0)
#define __lock_acquire(lock) ((void) 0)
#define __lock_acquire_recursive(lock) ((void) 0)
#define __lock_try_acquire(lock) ((void) 0)
#define __lock_try_acquire_recursive(lock) ((void) 0)
#define __lock_release(lock) ((void) 0)
#define __lock_release_recursive(lock) ((void) 0)

#else

#include <benaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Benaphores from gcc's <benaphore.h>; all zeros is unlocked.  Only
   libc's own code takes these locks.

   A plain lock is a bare benaphore where it is declared.  They are all
   static: libc makes none at run time, so there is no __lock_init() or
   __lock_close() for them.  The calls take the lock by name, as
   __LOCK_INIT() declares it.  */
typedef benaphore_t _LOCK_T;

#define __LOCK_INIT(class,lock) class _LOCK_T lock = BENAPHORE_INITIALIZER
#define __lock_acquire(lock) benaphore_lock(&(lock))
#define __lock_try_acquire(lock) benaphore_trylock(&(lock))
#define __lock_release(lock) benaphore_unlock(&(lock))

/* A recursive lock counts for the task that holds it.  It is a pointer,
   as a FILE's lock comes from malloc(): making and freeing one are
   calls into lock.c.  */
struct __lock { benaphore_recursive_t __r; };
typedef struct __lock * _LOCK_RECURSIVE_T;

#define __LOCK_INIT_RECURSIVE(class,lock) \
	extern struct __lock __lock_ ## lock; \
	class _LOCK_RECURSIVE_T lock = &__lock_ ## lock

extern void __retarget_lock_init_recursive(_LOCK_RECURSIVE_T *lock);
extern void __retarget_lock_close_recursive(_LOCK_RECURSIVE_T lock);
#define __lock_init_recursive(lock) __retarget_lock_init_recursive(&(lock))
#define __lock_close_recursive(lock) __retarget_lock_close_recursive(lock)

/* Inline: a bset and a test to take a lock nobody holds, and a compare
   and a count of its owner.  */
static __always_inline void __lock_acquire_recursive(_LOCK_RECURSIVE_T __l)
{
	benaphore_recursive_lock(&__l->__r);
}

static __always_inline int __lock_try_acquire_recursive(_LOCK_RECURSIVE_T __l)
{
	return benaphore_recursive_trylock(&__l->__r);
}

static __always_inline void __lock_release_recursive(_LOCK_RECURSIVE_T __l)
{
	benaphore_recursive_unlock(&__l->__r);
}

#ifdef __cplusplus
}
#endif

#endif /* !defined(_RETARGETABLE_LOCKING) */

#endif /* __SYS_LOCK_H__ */
