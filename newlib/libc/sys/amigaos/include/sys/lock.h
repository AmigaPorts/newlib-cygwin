#ifndef __SYS_LOCK_H__
#define __SYS_LOCK_H__

#include <newlib.h>
#include <_ansi.h>
#include <sys/cdefs.h>

#if !defined(_RETARGETABLE_LOCKING)

	/* No locks at all: gcc has no threads.  */
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

	#ifdef __cplusplus
	extern "C" {
	#endif

	/* libc takes its locks through these functions.  The ones in lock.c
	   do nothing: a program without threads pays an empty call at each
	   lock.  With -pthread, gcc links newlib-threads.o (threads.c), whose
	   functions of the same names are the real locks, on gcc's
	   <benaphore.h>.  Only libc's own code takes these locks.

	   The lock goes in a0, so that a call is a load and a jsr.  A lock is
	   a pointer: a static one points to storage defined with it, and a
	   FILE's is made when the FILE is.  All zeros is unlocked, and three
	   words are what threads.c needs for a recursive lock.  */
	struct __lock { void *__words[3]; };
	typedef struct __lock * _LOCK_T;
	typedef struct __lock * _LOCK_RECURSIVE_T;

	#define __LOCK_INIT(class,lock) \
		class struct __lock __lock_ ## lock; \
		class _LOCK_T lock = &__lock_ ## lock
	#define __LOCK_INIT_RECURSIVE(class,lock) __LOCK_INIT(class,lock)

	extern void __retarget_lock_init_recursive(_LOCK_T *lock);
	extern void __retarget_lock_close_recursive(_LOCK_T lock);
	extern void __retarget_lock_acquire(_LOCK_T lock __asm("a0"));
	extern int __retarget_lock_try_acquire(_LOCK_T lock __asm("a0"));
	extern void __retarget_lock_release(_LOCK_T lock __asm("a0"));
	extern void __retarget_lock_acquire_recursive(_LOCK_T lock __asm("a0"));
	extern int __retarget_lock_try_acquire_recursive(_LOCK_T lock __asm("a0"));
	extern void __retarget_lock_release_recursive(_LOCK_T lock __asm("a0"));

	#define __lock_init_recursive(lock) __retarget_lock_init_recursive(&(lock))
	#define __lock_close_recursive(lock) __retarget_lock_close_recursive(lock)
	#define __lock_acquire(lock) __retarget_lock_acquire(lock)
	#define __lock_try_acquire(lock) __retarget_lock_try_acquire(lock)
	#define __lock_release(lock) __retarget_lock_release(lock)
	#define __lock_acquire_recursive(lock) __retarget_lock_acquire_recursive(lock)
	#define __lock_try_acquire_recursive(lock) __retarget_lock_try_acquire_recursive(lock)
	#define __lock_release_recursive(lock) __retarget_lock_release_recursive(lock)

	#ifdef __cplusplus
	}
	#endif

#endif /* !defined(_RETARGETABLE_LOCKING) */

#endif /* __SYS_LOCK_H__ */
