/* The standard streams must not depend on the per-thread _REENT.
   libstdc++ uses newlib's headers also when linked with libnix or clib2,
   which supply _impure_ptr's leading stream pointers but not _REENT.
   Poison _REENT after including the headers to catch that dependency.  */
#include <stdio.h>

#undef _REENT
#define _REENT standard_streams_must_not_lookup_task_reent

int main(void)
{
  if (stdin != _REENT_STDIN(_impure_ptr)
      || stdout != _REENT_STDOUT(_impure_ptr)
      || stderr != _REENT_STDERR(_impure_ptr))
    return 1;
  return fputs("PASS: shared standard streams\n", stdout) < 0;
}
