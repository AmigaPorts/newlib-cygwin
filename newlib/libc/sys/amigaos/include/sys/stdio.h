#ifndef _NEWLIB_STDIO_H
#define _NEWLIB_STDIO_H

#include <sys/lock.h>
#include <sys/reent.h>

/* Internal locking, used to protect stdio functions.  */
#if defined(_RETARGETABLE_LOCKING)

	#ifdef __cplusplus
	extern "C" {
	#endif

	/* A FILE's lock is a call with the FILE in a0, to lock.c's functions
	   that do nothing or, with -pthread, to threads.c's.  Those test
	   whether the FILE has a lock to take, so that libc's stdio functions
	   have only the call: __retarget_flockfile() leaves out the FILE of
	   sprintf() and friends, as flockfile() does, and
	   __retarget_stdio_lock() also a FILE whose locking the program took
	   over with __fsetlocking().  */
	extern void __retarget_flockfile(__FILE *__fp __asm("a0"));
	extern void __retarget_funlockfile(__FILE *__fp __asm("a0"));
	extern void __retarget_stdio_lock(__FILE *__fp __asm("a0"));
	extern void __retarget_stdio_unlock(__FILE *__fp __asm("a0"));

	#ifdef __cplusplus
	}
	#endif

	#define _flockfile(fp)		__retarget_flockfile(fp)
	#define _funlockfile(fp)	__retarget_funlockfile(fp)

	/* For libc/stdio/local.h, in place of its test of __SNLK around
	   _flockfile().  */
	#define _flockfile_unless_nolock(fp)	__retarget_stdio_lock(fp)
	#define _funlockfile_unless_nolock(fp)	__retarget_stdio_unlock(fp)

#elif !defined(__SINGLE_THREAD__)

	/* As <stdio.h> defines it, after it includes this header.  */
	#define	__SSTR	0x0200

	#if !defined(_flockfile)
		static __always_inline void
		_flockfile(__FILE *__fp)
		{
			if (__builtin_expect(!(__fp->_flags & __SSTR), 1))
				__lock_acquire_recursive(__fp->_lock);
		}
	#endif

	#if !defined(_funlockfile)
		static __always_inline void
		_funlockfile(__FILE *__fp)
		{
			if (__builtin_expect(!(__fp->_flags & __SSTR), 1))
				__lock_release_recursive(__fp->_lock);
		}
	#endif

#else

	#if !defined(_flockfile)
		#define _flockfile(fp)	((void) 0)
	#endif
	#if !defined(_funlockfile)
		#define _funlockfile(fp)	((void) 0)
	#endif

#endif

#endif /* _NEWLIB_STDIO_H */
