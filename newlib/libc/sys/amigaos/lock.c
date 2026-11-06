/* newlib's retargetable locks for m68k-amigaos, on gcc's benaphore
   (<benaphore.h>): a bset to take a lock nobody holds, and a sleep in
   FIFO order when another task has it.

   This replaces libc/misc/lock.c, whose locks do nothing: libc.a keeps
   only the last object of each name.  --enable-newlib-retargetable-locking
   turns _RETARGETABLE_LOCKING on; without it this file is empty.

   sys/lock.h takes, tries and releases the locks inline; only making and
   freeing a recursive one are calls here.  These are libc's own locks:
   none of this is for programs.  The plain locks are bare benaphores
   where libc declares them.  The static recursive ones are here.  All
   are zeros, which is unlocked, so nothing needs setting up before the
   first one.  The dynamic ones, a FILE's lock above all, come from
   malloc() and go with the heap when the program exits.

   What a lock and an unlock cost a 68000 when nobody else wants the lock,
   in cycles from the instruction timings (see <benaphore.h>):

	plain lock, malloc()'s too	 70
	recursive lock, a FILE's	235, or 150 nested

   So each stdio call on a FILE pays about three times what malloc() pays
   for its lock.

   Interrupt code can't take these locks, as it can't take any lock that
   sleeps: stdio on a FILE and malloc() are not for interrupt code.  */

#include <newlib.h>

#ifdef _RETARGETABLE_LOCKING

#include <stdlib.h>
#include <string.h>
#include <sys/lock.h>

struct __lock __lock___sfp_recursive_mutex;
struct __lock __lock___atexit_recursive_mutex;
struct __lock __lock___malloc_recursive_mutex;
struct __lock __lock___env_recursive_mutex;

/* What every lock gets whose malloc() failed.  It nests, so a task may
   hold two of them at once, as it could two locks of their own.  */
static struct __lock nomem;

void __retarget_lock_init_recursive(_LOCK_RECURSIVE_T *lock) {
	struct __lock *l = (struct __lock *)malloc(sizeof *l);

	if (l)
		memset(l, 0, sizeof *l);
	else
		l = &nomem;
	*lock = l;
}

void __retarget_lock_close_recursive(_LOCK_RECURSIVE_T l) {
	if (l != &nomem)
		free(l);
}

#endif /* _RETARGETABLE_LOCKING */
