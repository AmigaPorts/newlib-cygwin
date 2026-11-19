/* errno and the rest of struct _reent per thread, for gcc's amigaos
   thread model (--enable-threads=amigaos, which gives newlib.h's
   _AMIGAOS_THREADS).

   A thread is what libgcc knows as one: the main task, a std::thread, or
   a thread that libpthread starts, which tells libgcc about it.  Each
   keeps its reent as its value of one libgcc key, __reent_key
   (gthr-amigaos.h), so that _REENT in sys/reent.h is two loads:

   - at startup the main task sets its value to _impure_ptr, the static
     reent it has always used;
   - another thread's value is null until it first needs its reent, when
     __getreent() allocates one from the heap and sets it;
   - when a thread ends, libgcc calls the key's destructor, which reclaims
     the reent and frees it.  The key is the program's first one, and
     libgcc runs the newest key's destructors first, so a thread's
     thread_local and other destructors still find their reent.

   A program that starts no thread allocates one key block in libgcc for
   the main task's value, and costs nothing at a task switch: libgcc puts
   a tc_Launch on the main task only once it starts its first thread.

   A task that libgcc doesn't know, such as one started with a plain
   CreateNewProc() or a system task that runs a hook of the program's,
   reads the key values of whichever thread ran before it, as it does
   for libgcc's other keys.  So its errno and the rest are undefined, as
   for a task created around the thread library on other systems.
   Interrupt code likewise finds the reent of the thread it interrupted.

   Without the amigaos thread model this file is empty, errno is the
   static reent's and _REENT is _impure_ptr.  */

#include <newlib.h>

#ifdef _AMIGAOS_THREADS

#include <errno.h>
#include <reent.h>
#include <stdlib.h>
#include <string.h>
#include <exec/execbase.h>
#include <exec/tasks.h>
#include <stabs.h>

extern struct ExecBase *SysBase;

/* libgcc's keys, from gthr-amigaos.h, which isn't installed for C.  */
extern __stdargs int __gthread_key_create (unsigned int *,
					   void (*) (void *));
extern __stdargs int __gthread_setspecific (unsigned int, const void *);

unsigned int __reent_key;

/* The task that keeps _impure_ptr, or null before the key exists.  */
static struct Task *main_task;

/* The key's destructor, in a thread that ends.  */
static void
free_reent (void *p)
{
  struct _reent *r = p;

  if (r == _impure_ptr)
    return;
  _reclaim_reent (r);
  _free_r (_impure_ptr, r);
}

/* Before the libraries open, so that everything after has its errno.  As
   the program's first key, it can't fail: __gthread_key_create() only
   fails when all keys are taken, and allocates nothing.  The main task's
   value needs a key block, which can: then its value stays null, and
   __getreent() gives it _impure_ptr.  */
void
__init_reent_key (void)
{
  if (__gthread_key_create (&__reent_key, free_reent) == 0)
    {
      main_task = SysBase->ThisTask;
      __gthread_setspecific (__reent_key, _impure_ptr);
    }
}
ADD2INIT (__init_reent_key, -110);

/* A thread's first use of its reent.  The static reent for the main task,
   before the key exists and after libgcc has dropped the key values at
   exit, for interrupt code, which runs on a stack that isn't its task's,
   and when out of memory.  The new reent comes from _malloc_r() with the
   static reent, as malloc() would look it up again: out of memory, that
   sets the main task's errno.  */
__stdargs struct _reent *
__getreent (void)
{
  struct Task *me = SysBase->ThisTask;
  char *sp = (char *) __builtin_frame_address (0);
  struct _reent *r;

  if (!main_task || me == main_task
      || sp < (char *) me->tc_SPLower || sp >= (char *) me->tc_SPUpper)
    return _impure_ptr;
  if (!(r = (struct _reent *) _malloc_r (_impure_ptr, sizeof (*r))))
    return _impure_ptr;
  _REENT_INIT_PTR (r);
  if (__gthread_setspecific (__reent_key, r))
    {
      _free_r (_impure_ptr, r);
      return _impure_ptr;
    }
  return r;
}

/* errno, for sys/errno.h.  libnix and clib2 have one of the same name for
   theirs, for libstdc++.  */
__stdargs int *
__amigaos_errno (void)
{
  return &_REENT_ERRNO (_REENT);
}

#endif /* _AMIGAOS_THREADS */
