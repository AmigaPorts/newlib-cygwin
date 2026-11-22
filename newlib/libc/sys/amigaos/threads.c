/* libc for a program with threads: the real locks, and a struct _reent
   for each thread.

   This file is not in libc.a: it is installed as newlib-threads.o, which
   gcc's -pthread links before libc with the amigaos thread model.  Its
   functions are then in the program before the linker looks for them in
   libc.a, where lock.c has the ones that do nothing, and libc's stdio is
   locked.  So is the heap: the _malloc_r() and _free_r() here take a lock
   around the allocator's, where those two names otherwise are the
   allocator itself (mallocr.cpp).  A program without -pthread links
   nothing of this.

   The locks are gcc's benaphores (<benaphore.h>): a bset to take a lock
   nobody holds, and a sleep in FIFO order when another task has it.  What
   a lock and an unlock cost a 68000 when nobody else wants the lock, in
   cycles from the instruction timings, without the call that gets here:

	plain lock, malloc()'s too	 70
	recursive lock, a FILE's	235, or 150 nested

   Interrupt code can't take these locks, as it can't take any lock that
   sleeps: stdio on a FILE and malloc() are not for interrupt code.

   errno, strtok() and the rest of struct _reent are each thread's own.
   libc reads them through _impure_ptr as ever: libgcc's tc_Launch, which
   runs whenever a thread gets the CPU, points it at that thread's reent.
   A thread is what libgcc knows as one: the main task, a std::thread, or
   a thread that libpthread starts.  */

#include <newlib.h>

#ifdef _RETARGETABLE_LOCKING

#include <reent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/lock.h>
#include <benaphore.h>
#include "stabs.h"

/* <sys/lock.h> has the storage of libc's static locks.  */
_Static_assert (sizeof (benaphore_recursive_t) <= sizeof (struct __lock),
		"a static lock holds a recursive benaphore");

#define PLAIN(lock) ((benaphore_t *) (lock))
#define RECURSIVE(lock) ((benaphore_recursive_t *) (lock))

/* The heap's lock.  Nothing under it allocates from the heap again, so a
   plain lock.  malloc(), free(), realloc() and calloc() all get to the
   heap through these two.  */
extern void *__malloc_unlocked_r (struct _reent *r, size_t size);
extern void __free_unlocked_r (struct _reent *r, void *p);

static benaphore_t heap_lock;

void *
_malloc_r (struct _reent *r, size_t size)
{
  void *p;

  benaphore_lock (&heap_lock);
  p = __malloc_unlocked_r (r, size);
  benaphore_unlock (&heap_lock);
  return p;
}

void
_free_r (struct _reent *r, void *p)
{
  benaphore_lock (&heap_lock);
  __free_unlocked_r (r, p);
  benaphore_unlock (&heap_lock);
}

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

/* libgcc's, from gthr-amigaos.c: a pointer that follows the running
   thread.  Each new thread gets a value from new_ptr(), which tc_Launch
   stores in *var whenever the thread gets the CPU, and free_ptr() takes
   it back when the thread ends.  The main task keeps the value *var has
   now.  */
extern __stdargs void
__gthread_amigaos_thread_pointer (void **var,
				  void *(__stdargs *new_ptr) (void),
				  void (__stdargs *free_ptr) (void *));

extern int _init_signal_r (struct _reent *r);

static struct _reent *main_reent;

/* In the new thread, before its tc_Launch is on: _impure_ptr is still
   another thread's, so it isn't used.  An allocation that fails stores
   ENOMEM in the scratch reent, where it disturbs no other thread: 76 bytes
   of the new thread's stack, as long as the reent is _REENT_SMALL's.  The
   handlers of signal() are the process's, so every thread shares the main
   task's table.  So are the three standard FILEs, which the main task's
   reent points to: copied from there rather than taken from __sf, which is
   weak, so as not to link stdio, and which a base-relative program can't
   address while it is undefined.  */
static __stdargs void *
new_reent (void)
{
  struct _reent scratch;
  struct _reent *r = _malloc_r (&scratch, sizeof (*r));

  if (r)
    {
      memset (r, 0, sizeof (*r));
      r->_stdin = main_reent->_stdin;
      r->_stdout = main_reent->_stdout;
      r->_stderr = main_reent->_stderr;
      _REENT_SIG_FUNC (r) = _REENT_SIG_FUNC (main_reent);
    }
  return r;
}

static __stdargs void
free_reent (void *r)
{
  _reclaim_reent (r);
  _free_r (main_reent, r);
}

/* Before every constructor, and every other callback of the init list but
   the one that opens the libraries: libgcc refuses to start a thread for a
   newlib program until this has run.  */
void
__init_thread_reent (void)
{
  main_reent = _impure_ptr;
  _init_signal_r (main_reent);
  __gthread_amigaos_thread_pointer ((void **) &_impure_ptr, new_reent,
				    free_reent);
}

ADD2INIT (__init_thread_reent, -60);

#endif /* _RETARGETABLE_LOCKING */
