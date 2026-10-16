/* The errno variable is stored in the reentrancy structure.  This
   function returns its address for use by the macro errno defined in
   errno.h.  */
#define __NO_INLINE__
#include <errno.h>
#include <reent.h>

#ifdef _REENT_THREAD_LOCAL
_Thread_local int _tls_errno;
#else /* !_REENT_THREAD_LOCAL */

#ifndef _REENT_ONLY

#ifndef __stdargs
#define __stdargs
#endif

__stdargs int *
__errno ()
{
  return &_REENT_ERRNO(_REENT);
}

#endif

#endif /* _REENT_THREAD_LOCAL */
