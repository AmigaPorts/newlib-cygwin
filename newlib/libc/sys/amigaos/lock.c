/* libc's locks for a program without threads: functions that do nothing.

   --enable-newlib-retargetable-locking turns _RETARGETABLE_LOCKING on, and
   libc then takes its locks through the __retarget_lock_*() functions of
   sys/lock.h.  With the ones here, a program pays an empty call at each
   lock.  With -pthread, gcc links newlib-threads.o before libc: its
   functions of the same names are the real locks, and the linker has no
   reason to take this file.

   This replaces libc/misc/lock.c: libc.a keeps only the last object of
   each name.  Without _RETARGETABLE_LOCKING this file is empty, and libc
   has no trace of its locks.  */

#include <newlib.h>

#ifdef _RETARGETABLE_LOCKING

#include <stdio.h>
#include <sys/lock.h>

#define UNUSED __attribute__ ((__unused__))

/* A FILE's lock, <sys/stdio.h>.  */
void
__retarget_flockfile (FILE *fp __asm ("a0") UNUSED)
{
}

void
__retarget_funlockfile (FILE *fp __asm ("a0") UNUSED)
{
}

void
__retarget_stdio_lock (FILE *fp __asm ("a0") UNUSED)
{
}

void
__retarget_stdio_unlock (FILE *fp __asm ("a0") UNUSED)
{
}

/* A FILE's lock: none.  */
void
__retarget_lock_init_recursive (_LOCK_T *lock)
{
  *lock = 0;
}

void
__retarget_lock_close_recursive (_LOCK_T lock UNUSED)
{
}

void
__retarget_lock_acquire (_LOCK_T lock __asm ("a0") UNUSED)
{
}

int
__retarget_lock_try_acquire (_LOCK_T lock __asm ("a0") UNUSED)
{
  return 1;
}

void
__retarget_lock_release (_LOCK_T lock __asm ("a0") UNUSED)
{
}

void
__retarget_lock_acquire_recursive (_LOCK_T lock __asm ("a0") UNUSED)
{
}

int
__retarget_lock_try_acquire_recursive (_LOCK_T lock __asm ("a0") UNUSED)
{
  return 1;
}

void
__retarget_lock_release_recursive (_LOCK_T lock __asm ("a0") UNUSED)
{
}

#endif /* _RETARGETABLE_LOCKING */
