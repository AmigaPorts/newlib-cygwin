/* A program with threads, linked with newlib-threads.o, gives each thread
   its own reent, which _impure_ptr follows:
   - errno and strtok()'s state that one thread sets don't show in
     another;
   - a task that isn't libgcc's thread, here a process started with a
     plain CreateNewProc(), gets no reent and no tc_Launch of its own: it
     uses the reent of the thread that ran before it.
   The threads take turns by yielding, so this needs no time slices.  A
   newlib built without locks, for a toolchain without threads, has
   nothing to test.

   amigaos.exp links this with newlib-threads.o, as lock-nomem.c.  */
#include <stdio.h>
#include <sys/lock.h>

#ifndef _RETARGETABLE_LOCKING
int main(void)
{
  return 0;
}
#else

#include <errno.h>
#include <reent.h>
#include <string.h>
#include <dos/dostags.h>
#include <exec/execbase.h>
#include <exec/tasks.h>
#include <proto/dos.h>
#include <proto/exec.h>

/* libgcc's threads, from gthr-amigaos.h, which isn't installed for C.  */
extern __stdargs int __gthread_create (void **, void *(*) (void *), void *);
extern __stdargs int __gthread_join (void *, void **);
extern __stdargs int __gthread_yield (void);

extern struct ExecBase *SysBase;

static int failures;
static const char *failed[8];

static void check(int ok, const char *what)
{
  if (!ok && failures < 8)
    failed[failures++] = what;
  else if (!ok)
    failures++;
}

/* errno and strtok() in a thread, while the main task has its own.  */

static struct _reent *main_reent;
static volatile int step;
static int thread_own, thread_errno, thread_token;

static void *own_reent(void *arg)
{
  char u[] = "x y";
  char *b;

  (void) arg;
  thread_own = _impure_ptr != main_reent;
  errno = EDOM;
  strtok(u, " ");
  step = 1;
  while (step != 2)
    __gthread_yield();
  thread_errno = errno;
  b = strtok(0, " ");
  thread_token = b && !strcmp(b, "y");
  return 0;
}

static void test_own_reent(void)
{
  char s[] = "a b c";
  void *t;
  char *b;

  main_reent = _impure_ptr;
  errno = 0;
  strtok(s, " ");
  if (__gthread_create(&t, own_reent, 0))
    {
      check(0, "thread starts");
      return;
    }
  while (step != 1)
    __gthread_yield();
  check(_impure_ptr == main_reent, "main task's reent comes back");
  check(errno == 0, "thread's errno doesn't show in the main task");
  errno = ERANGE;
  step = 2;
  b = strtok(0, " ");
  check(errno == ERANGE, "main task's errno stays");
  __gthread_join(t, 0);
  check(thread_own, "thread has a reent of its own");
  check(thread_errno == EDOM, "main task's errno doesn't show in the thread");
  check(thread_token, "thread's strtok()");
  check(b && !strcmp(b, "b"), "main task's strtok()");
}

/* A plain process, started by a thread.  It runs at a higher priority, so
   it has ended when the thread's Wait() returns, and nothing ran between
   the thread and it.  */

static struct Task *foreign_parent;
static ULONG foreign_signal;
static struct _reent *foreign_reent, *foreign_expected;
static int foreign_launch, foreign_started;

/* A plain process starts without the program's a4, which -fbaserel code
   needs to reach its data.  */
#ifdef __baserel__
#define FOREIGN_ENTRY __saveds
#else
#define FOREIGN_ENTRY
#endif

static FOREIGN_ENTRY void foreign_entry(void)
{
  struct Task *me = SysBase->ThisTask;

  foreign_reent = _impure_ptr;
  foreign_launch = (me->tc_Flags & TF_LAUNCH) != 0;
  Signal(foreign_parent, foreign_signal);
}

static void *start_foreign(void *arg)
{
  BYTE sig;

  (void) arg;
  foreign_expected = _impure_ptr;
  foreign_parent = SysBase->ThisTask;
  if ((sig = AllocSignal(-1)) < 0)
    return 0;
  foreign_signal = 1UL << sig;
  if (CreateNewProcTags(NP_Entry, (ULONG) foreign_entry,
			NP_StackSize, 8192,
			NP_Name, (ULONG) "reent-thread foreign",
			NP_Priority, foreign_parent->tc_Node.ln_Pri + 1,
			TAG_END))
    {
      foreign_started = 1;
      Wait(foreign_signal);
    }
  FreeSignal(sig);
  return 0;
}

static void test_foreign_task(void)
{
  void *t;

  if (__gthread_create(&t, start_foreign, 0))
    {
      check(0, "thread starts");
      return;
    }
  __gthread_join(t, 0);
  check(foreign_started, "foreign process starts");
  check(foreign_reent == foreign_expected,
	"foreign process gets no reent of its own");
  check(!foreign_launch, "foreign process gets no tc_Launch");
}

int main(void)
{
  int i;

  test_own_reent();
  test_foreign_task();
  for (i = 0; i < failures && i < 8; i++)
    printf("FAIL: %s\n", failed[i]);
  if (!failures)
    printf("PASS: a reent per thread\n");
  return failures != 0;
}

#endif /* _RETARGETABLE_LOCKING */
