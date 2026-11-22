/* libc for a program with threads: the real locks.

   This file is not in libc.a: it is installed as newlib-threads.o, which
   gcc's -pthread links before libc with the amigaos thread model.  Its
   functions are then in the program before the linker looks for them in
   libc.a, where lock.c has the ones that do nothing, and libc's stdio and
   malloc() are locked.  A program without -pthread links nothing of
   this.

   The locks are gcc's benaphores (<benaphore.h>): a bset to take a lock
   nobody holds, and a sleep in FIFO order when another task has it.  What
   a lock and an unlock cost a 68000 when nobody else wants the lock, in
   cycles from the instruction timings, without the call that gets here:

	plain lock, malloc()'s too	 70
	recursive lock, a FILE's	235, or 150 nested

   Interrupt code can't take these locks, as it can't take any lock that
   sleeps: stdio on a FILE and malloc() are not for interrupt code.  */

#include <newlib.h>

#ifdef _RETARGETABLE_LOCKING

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/lock.h>
#include <benaphore.h>

/* <sys/lock.h> has the storage of libc's static locks.  */
_Static_assert (sizeof (benaphore_recursive_t) <= sizeof (struct __lock),
		"a static lock holds a recursive benaphore");

#define PLAIN(lock) ((benaphore_t *) (lock))
#define RECURSIVE(lock) ((benaphore_recursive_t *) (lock))

/* What every FILE gets whose lock malloc() had no memory for.  It nests,
   so a task may hold two of them at once, as it could two locks of their
   own.  */
static benaphore_recursive_t nomem;

void
__retarget_lock_init_recursive (_LOCK_T *lock)
{
  benaphore_recursive_t *l = malloc (sizeof (*l));

  if (l)
    memset (l, 0, sizeof (*l));
  else
    l = &nomem;
  *lock = (_LOCK_T) l;
}

void
__retarget_lock_close_recursive (_LOCK_T lock)
{
  if (RECURSIVE (lock) != &nomem)
    free (lock);
}

void
__retarget_lock_acquire (_LOCK_T lock __asm ("a0"))
{
  benaphore_lock (PLAIN (lock));
}

int
__retarget_lock_try_acquire (_LOCK_T lock __asm ("a0"))
{
  return benaphore_trylock (PLAIN (lock));
}

void
__retarget_lock_release (_LOCK_T lock __asm ("a0"))
{
  benaphore_unlock (PLAIN (lock));
}

void
__retarget_lock_acquire_recursive (_LOCK_T lock __asm ("a0"))
{
  benaphore_recursive_lock (RECURSIVE (lock));
}

int
__retarget_lock_try_acquire_recursive (_LOCK_T lock __asm ("a0"))
{
  return benaphore_recursive_trylock (RECURSIVE (lock));
}

void
__retarget_lock_release_recursive (_LOCK_T lock __asm ("a0"))
{
  benaphore_recursive_unlock (RECURSIVE (lock));
}

/* A FILE's lock, <sys/stdio.h>.  The FILE that sprintf() and friends make
   for a string has none, and the stdio functions leave alone a FILE whose
   locking the program took over with __fsetlocking().  */
void
__retarget_flockfile (FILE *fp __asm ("a0"))
{
  if (!(fp->_flags & __SSTR))
    benaphore_recursive_lock (RECURSIVE (fp->_lock));
}

void
__retarget_funlockfile (FILE *fp __asm ("a0"))
{
  if (!(fp->_flags & __SSTR))
    benaphore_recursive_unlock (RECURSIVE (fp->_lock));
}

void
__retarget_stdio_lock (FILE *fp __asm ("a0"))
{
  if (!(fp->_flags2 & __SNLK))
    __retarget_flockfile (fp);
}

void
__retarget_stdio_unlock (FILE *fp __asm ("a0"))
{
  if (!(fp->_flags2 & __SNLK))
    __retarget_funlockfile (fp);
}

#endif /* _RETARGETABLE_LOCKING */
