#ifndef _NEWLIB_STDIO_H
#define _NEWLIB_STDIO_H

#include <sys/lock.h>
#include <sys/reent.h>

/* Internal locking, used to protect stdio functions.  As
   libc/include/sys/stdio.h, but the lock is a few instructions inline
   for m68k-amigaos (sys/lock.h), so the usual case, a FILE that is not
   private to sprintf/sscanf, is the straight path.  */
#ifndef __SINGLE_THREAD__

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
