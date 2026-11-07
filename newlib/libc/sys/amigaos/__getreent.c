/* __getreent() for m68k-amigaos: newlib's per-task state, errno first.

   sys/config.h defines __DYNAMIC_REENT__, so _REENT, and with it errno,
   strtok(), rand(), the dtoa buffers and the rest of struct _reent, is per
   task.  The program's main task, which crt0 records, keeps the static
   _impure_ptr.  Every other task that calls into newlib gets its own
   struct _reent on its first call.

   The main task gets it without a call.  sys/reent.h makes _REENT
   __reent_fast, and only if that is null __getreent().  __reent_fast is
   _impure_ptr exactly while the main task has the CPU: crt0 sets the
   task's tc_Switch and tc_Launch, which exec calls when the task loses
   and gets the CPU, to clear and set it.  So for the main task _REENT is
   a load and a test, and every other task finds it null and calls
   __getreent().  As a task's reent never changes, errno's __errno() and
   __getreent() are const, and the compiler may share one call between
   the uses in a function.  The hooks go again at the end of the exit, before the
   program's code does.  Bypassing crt0's exit, for example with DOS Exit()
   or a debugger that unloads the program, leaves hooks in the shell's
   process pointing at unloaded code; that exit path must not be used.
   A main task that already has a tc_Switch or tc_Launch keeps it, and
   gets its reent from __getreent() after one
   compare, as every task did before.

   The reent of another task goes on that task's tc_MemEntry list, so exec
   frees it when the task ends: there is no other hook for a task's end.
   It does not take ownership of tc_UserData or pr_CIS, which a thread
   library or another task creator may use for its startup data.
   The lookup walks the list for our MemList, which is put at its head
   and is usually the first node.  A
   task that cannot get the memory for one shares a static spare reent
   with the other such tasks, never the main task's, until a later call
   gets it one: see newreent().

   The parts that reent-small would malloc() on first use, strtok()'s and
   rand()'s state among them, come in the same allocation.  So exec frees
   them too, and a task's first strtok() or rand() needs no malloc().
   The main task's are static, set up by crt0.

   What newlib allocates later for a reent and keeps with it, the dtoa
   Bigints and their freelist above all, but also the ecvt() buffer and
   the signal() table, comes from __amiga_reent_realloc() (see
   sys/reent.h).  For another task's reent it is AllocMem()'d and goes on
   the task's tc_MemEntry as well, so exec frees it with the task: a
   program that keeps starting tasks that print "%f" does not grow.  The
   Bigints stay on the freelist while the task lives, so each task
   allocates a handful of them once.  Every other reent, the main task's
   among them, keeps using the heap.  Each block is an AllocMem() of its
   own, behind its own MemList: FreeEntry() frees exactly what was
   allocated, which an allocator that replaces exec's, such as TLSFMem,
   may require.

   Code that does not start with crt0, built with -nostartfiles or with a
   startup of its own, gets no hooks: __reent_fast stays null, and every
   _REENT is a call.  The first task that calls __getreent() is
   the main task and keeps _impure_ptr, whose parts reent-small then
   malloc()s on first use.  Nor does the exit list run, which is next.

   A task that outlives the program, such as a system task that ran one
   of its hooks, would keep its reent: the exit list takes this run's
   reents off every task that is still there and frees them.  The cookie
   tells this run's reents from those of an earlier run of a program that
   was loaded at the same address.

   Interrupt code must not allocate.  An interrupt server or a software
   interrupt runs while some task is ThisTask, on the system stack between
   SysStkLower and SysStkUpper.  While the main task has the CPU, it finds
   __reent_fast and shares the main task's reent, whose parts are all
   static: an interrupt's errno can then land in the main task's, as it
   did with the one global reent before.  Telling interrupts apart there
   would cost a stack pointer compare on every errno.  While another task
   has the CPU, __getreent() checks the stack pointer first and gives
   interrupt code a static reent of its own.  Either way nothing is
   allocated.  Interrupt code may still not use stdio on a FILE or
   malloc(), whose locks wait, nor "%f", even into a string: its Bigints
   come from malloc().

   Code that a task runs through Supervisor(), and a trap handler, run on
   the system stack too, and are interrupt code here: the main task's
   finds __reent_fast and the main task's reent, and any other task's
   gets the interrupt reent, not its own, with its errno.  The same rules
   hold for it.

   stdin, stdout and stderr are newlib's global __sf[]: every reent points
   at the same three streams, so another task's printf() goes to the same
   buffer and console as the main task's.  The stream's lock, lock.c,
   makes them take turns.  */

#include <errno.h>
#include <reent.h>
#include <stabs.h>
#include <stdlib.h>
#include <string.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <proto/exec.h>
#include <inline/exec.h>

extern struct ExecBase *SysBase;

/* struct task_reent, the main task's static parts and the interrupt
   reent are made of the reent-small parts; configure.host turns it on.  */
#ifndef _REENT_SMALL
#error "m68k-amigaos needs the small reent: configure with --enable-newlib-reent-small"
#endif

static void newlist(struct List *l) {
	l->lh_Head = (struct Node *)&l->lh_Tail;
	l->lh_Tail = NULL;
	l->lh_TailPred = (struct Node *)&l->lh_Head;
}

/* Set by crt0 at startup; without it, the first task that calls in.  */
struct Task *__reent_main_task;

/* Tells this run's reents from those a task kept from an earlier run of a
   program that was loaded at the same address.  */
static unsigned long cookie;

/* The MemLists' ln_Name: this program's own addresses, MARKER for a
   reent, PART for the memory that hangs off one.  */
#define MARKER ((char *)&__reent_main_task)
#define PART ((char *)&cookie)

struct task_reent {
	/* First, here and in a part, for the exit sweep.  */
	unsigned long cookie;
	/* Whose tc_MemEntry gets its parts; null for intr and spare.  */
	struct Task *owner;
	struct _reent r;
	struct _misc_reent misc;
	struct _mprec mp;
	struct _rand48 r48;
	struct __tm tm;
	char asctime[_REENT_ASCTIME_SIZE];
	char emergency[_REENT_EMERGENCY_SIZE];
	char signal_buf[_REENT_SIGNAL_SIZE];
};

/* What newreent() sets up, done statically.  */
#define STATIC_REENT(name) {						\
	.r = {								\
		._stdin = &__sf[0],					\
		._stdout = &__sf[1],					\
		._stderr = &__sf[2],					\
		._emergency = name.emergency,				\
		._mp = &name.mp,					\
		._r48 = &name.r48,					\
		._localtime_buf = &name.tm,				\
		._asctime_buf = name.asctime,				\
		._misc = &name.misc,					\
		._signal_buf = name.signal_buf,				\
	},								\
	.r48 = {							\
		{ _RAND48_SEED_0, _RAND48_SEED_1, _RAND48_SEED_2 },	\
		{ _RAND48_MULT_0, _RAND48_MULT_1, _RAND48_MULT_2 },	\
		_RAND48_ADD, 1						\
	},								\
}

/* Interrupt code's reent.  */
static struct task_reent intr = STATIC_REENT(intr);

/* The reent of a task whose own reent could not be allocated, shared by
   every such task, but never the main task's.  Its state is not task-safe:
   in particular, concurrent dtoa users can race on its Bigint freelist,
   whose accesses are not protected by the heap lock.  This fallback keeps
   allocation failure from corrupting the main task's state; it does not
   provide independent state to the tasks sharing it.  */
static struct task_reent spare = STATIC_REENT(spare);

/* The parts of the main task's reent, static as well.  */
static struct {
	struct _misc_reent misc;
	struct _mprec mp;
	struct _rand48 r48;
	struct __tm tm;
	char asctime[_REENT_ASCTIME_SIZE];
	char emergency[_REENT_EMERGENCY_SIZE];
	char signal_buf[_REENT_SIGNAL_SIZE];
} mainparts;

struct _reent *__reent_fast;

/* The main task's tc_Launch and tc_Switch.  exec calls them in supervisor
   mode, with no promise about registers: they touch none.  Absolute
   addresses: a4 is not the program's here.  */
__asm__(
"	.text\n"
"	.even\n"
"_reent_launch:\n"
"	move.l	__impure_ptr,___reent_fast\n"
"	rts\n"
"_reent_switch:\n"
"	move.l	#0,___reent_fast\n"
"	rts\n");
extern void reent_launch(void) __asm__("_reent_launch");
extern void reent_switch(void) __asm__("_reent_switch");

static char hooked;

/* crt0, before anything else that could use the reent.  */
void __reent_start(void) {
	struct _reent *r = _impure_ptr;
	struct Task *me = __reent_main_task;

	if (!r->_misc) {
		r->_misc = &mainparts.misc;
		_REENT_INIT_MISC(r);
	}
	if (!r->_mp) {
		r->_mp = &mainparts.mp;
		_REENT_INIT_MP(r);
	}
	if (!r->_r48) {
		r->_r48 = &mainparts.r48;
		_REENT_INIT_RAND48(r);
	}
	if (!r->_localtime_buf)
		r->_localtime_buf = &mainparts.tm;
	if (!r->_asctime_buf)
		r->_asctime_buf = mainparts.asctime;
	if (!r->_emergency)
		r->_emergency = mainparts.emergency;
	if (!r->_signal_buf)
		r->_signal_buf = mainparts.signal_buf;

	/* In Forbid(): no task switch between the store and the hooks.  */
	Forbid();
	if (!(me->tc_Flags & (TF_SWITCH | TF_LAUNCH))) {
		me->tc_Switch = reent_switch;
		me->tc_Launch = reent_launch;
		__reent_fast = r;
		me->tc_Flags |= TF_SWITCH | TF_LAUNCH;
		hooked = 1;
	}
	Permit();
}

/* crt0, at the very end of the exit: the hooks are in the program's code.  */
void __reent_stop(void) {
	struct Task *me = __reent_main_task;

	if (hooked) {
		Forbid();
		me->tc_Flags &= ~(TF_SWITCH | TF_LAUNCH);
		me->tc_Switch = NULL;
		me->tc_Launch = NULL;
		__reent_fast = NULL;
		hooked = 0;
		Permit();
	}
}

/* Without the memory for a reent of its own, a task gets spare.  The
   next call tries again, and the task keeps the first reent it gets:
   remembering that a task has spare would need memory too, on its
   tc_MemEntry, and a list of tasks that would outlive them.  What the
   task did with spare until then, its errno for one, is shared with
   every other task that has it, so it was never safe to keep; the main
   task's is never touched.  */
static __attribute__((noinline)) struct _reent *newreent(struct Task *me) {
	struct MemList *ml;
	struct task_reent *t;
	struct _reent *r;

	ml = (struct MemList *)AllocMem(sizeof *ml, MEMF_PUBLIC | MEMF_CLEAR);
	if (!ml)
		return &spare.r;
	t = (struct task_reent *)AllocMem(sizeof *t, MEMF_ANY | MEMF_CLEAR);
	if (!t) {
		FreeMem(ml, sizeof *ml);
		return &spare.r;
	}

	r = &t->r;
	_REENT_INIT_PTR_ZEROED(r);
	r->_misc = &t->misc;
	_REENT_INIT_MISC(r);
	r->_mp = &t->mp;
	_REENT_INIT_MP(r);
	r->_r48 = &t->r48;
	_REENT_INIT_RAND48(r);
	r->_localtime_buf = &t->tm;
	r->_asctime_buf = t->asctime;
	r->_emergency = t->emergency;
	r->_signal_buf = t->signal_buf;
	t->cookie = cookie;
	t->owner = me;

	ml->ml_Node.ln_Name = MARKER;
	ml->ml_NumEntries = 1;
	ml->ml_ME[0].me_Addr = t;
	ml->ml_ME[0].me_Length = sizeof *t;
	Forbid();
	/* Tasks the ROM starts with AddTask(), input.device's and
	   console.device's among them, have a tc_MemEntry of zeroes, not an
	   empty list: make it one.  */
	if (!me->tc_MemEntry.lh_Head)
		newlist(&me->tc_MemEntry);
	AddHead(&me->tc_MemEntry, &ml->ml_Node);
	Permit();
	return r;
}

/* Out of line, so that the main task's path in __getreent() saves no
   registers.  */
static __attribute__((noinline)) struct _reent *taskreent(struct Task *me) {
	struct Node *n;

	if (!cookie) {
		Forbid();
		if (!__reent_main_task)
			__reent_main_task = me;
		if (!cookie)
			cookie = ((unsigned long)__reent_main_task
				  ^ SysBase->DispCount) | 1;
		Permit();
		if (me == __reent_main_task)
			return _impure_ptr;
	}

	for (n = me->tc_MemEntry.lh_Head; n && n->ln_Succ; n = n->ln_Succ)
		if (n->ln_Name == MARKER) {
			struct task_reent *t = (struct task_reent *)
				((struct MemList *)n)->ml_ME[0].me_Addr;
			if (t->cookie == cookie)
				return &t->r;
		}
	return newreent(me);
}

#ifndef __stdargs
#define __stdargs
#endif

#undef __getreent
__stdargs struct _reent *__getreent(void) {
	struct ExecBase *sys;
	struct Task *me;
	char *sp;

	/* For callers built with older headers.  */
	if (__reent_fast)
		return __reent_fast;
	sys = SysBase;
	me = sys->ThisTask;
	__asm__ ("move.l sp,%0" : "=r" (sp));
	if (sp >= (char *)sys->SysStkLower && sp < (char *)sys->SysStkUpper)
		return &intr.r;
	if (me == __reent_main_task)
		return _impure_ptr;
	return taskreent(me);
}

/* A block that hangs off a task's reent, in front of the memory.  8
   bytes, so the memory is as aligned as AllocMem()'s.  */
struct part {
	unsigned long cookie;
	struct _reent *r;
};

/* The task reent that r is, if it is one of this run's with an owner.
   Its _mp points into the same block, which no other reent's does.  */
static struct task_reent *owned(struct _reent *r) {
	struct task_reent *t = (struct task_reent *)
		((char *)r - __builtin_offsetof(struct task_reent, r));

	if (r->_mp != &t->mp || !cookie || t->cookie != cookie || !t->owner)
		return NULL;
	return t;
}

/* In Forbid(): the MemList of r's part at p, taken off the list.  */
static struct MemList *takepart(struct task_reent *t, void *p) {
	struct Node *n;

	for (n = t->owner->tc_MemEntry.lh_Head; n && n->ln_Succ; n = n->ln_Succ)
		if (n->ln_Name == PART
		    && ((struct MemList *)n)->ml_ME[0].me_Addr == (struct part *)p - 1) {
			Remove(n);
			return (struct MemList *)n;
		}
	return NULL;
}

/* See sys/reent.h: zeroed memory for r to keep, with old's contents if
   old is not null, which it then frees; on failure null, and old is
   kept, as with realloc().  */
__stdargs void *__amiga_reent_realloc(struct _reent *r, void *old, size_t size) {
	struct task_reent *t = owned(r);
	struct MemList *ml, *oml;
	struct part *p;

	if (!t)
		return old ? _realloc_r(r, old, size) : _calloc_r(r, 1, size);
	if (size > 0x7fffffff - sizeof *p)
		goto nomem;
	ml = (struct MemList *)AllocMem(sizeof *ml, MEMF_PUBLIC | MEMF_CLEAR);
	if (!ml)
		goto nomem;
	p = (struct part *)AllocMem(sizeof *p + size, MEMF_ANY | MEMF_CLEAR);
	if (!p) {
		FreeMem(ml, sizeof *ml);
		goto nomem;
	}
	p->cookie = cookie;
	p->r = r;
	ml->ml_Node.ln_Name = PART;
	ml->ml_NumEntries = 1;
	ml->ml_ME[0].me_Addr = p;
	ml->ml_ME[0].me_Length = sizeof *p + size;
	Forbid();
	oml = old ? takepart(t, old) : NULL;
	/* At the tail: the reent's MemList stays first.  */
	AddTail(&t->owner->tc_MemEntry, &ml->ml_Node);
	Permit();
	if (oml) {
		size_t n = oml->ml_ME[0].me_Length - sizeof *p;
		memcpy(p + 1, old, n < size ? n : size);
		FreeEntry(oml);
	}
	return p + 1;

nomem:
	_REENT_ERRNO(r) = ENOMEM;
	return NULL;
}

/* _reclaim_reent() of r, see sys/reent.h.  The parts in a task reent's
   own block, which newlib's _reclaim_reent() would hand to _free_r(),
   stay.  Of what hangs off it, the signal() table stays too, as newlib
   keeps it; the rest is freed and forgotten, and the reent can go on.
   intr and spare are static, and what hangs off them stays in the
   heap.  */
__stdargs int __amiga_reclaim_reent(struct _reent *r) {
	struct task_reent *t = (struct task_reent *)
		((char *)r - __builtin_offsetof(struct task_reent, r));
	struct Node *n, *next;
	struct MemList *ml;
	struct List gone;

	if (r->_mp != &t->mp)
		return 0;	/* not a task reent: newlib's own reclaim */
	if (owned(r)) {
		newlist(&gone);
		Forbid();
		for (n = t->owner->tc_MemEntry.lh_Head; n && (next = n->ln_Succ); n = next)
			if (n->ln_Name == PART) {
				struct part *p = (struct part *)((struct MemList *)n)->ml_ME[0].me_Addr;
				if (p->r == r && p->cookie == cookie
				    && (void *)(p + 1) != (void *)_REENT_SIG_FUNC(r)) {
					Remove(n);
					AddTail(&gone, n);
				}
			}
		Permit();
		while ((ml = (struct MemList *)RemHead(&gone)))
			FreeEntry(ml);
		_REENT_MP_FREELIST(r) = NULL;
		_REENT_MP_RESULT(r) = NULL;
		_REENT_MP_RESULT_K(r) = 0;
		_REENT_MP_P5S(r) = NULL;
		_REENT_CVTBUF(r) = NULL;
		_REENT_CVTLEN(r) = 0;
	}
	if (_REENT_CLEANUP(r))
		_REENT_CLEANUP(r)(r);
	return 1;
}

/* Take this run's reents and their parts off the task in Disable(), into
   gone.  */
static void sweep(struct Task *t, struct List *gone) {
	struct Node *n, *next;

	for (n = t->tc_MemEntry.lh_Head; n && (next = n->ln_Succ); n = next)
		if ((n->ln_Name == MARKER || n->ln_Name == PART)
		    && *(unsigned long *)((struct MemList *)n)->ml_ME[0].me_Addr
		       == cookie) {
			Remove(n);
			AddTail(gone, n);
		}
}

/* At exit, after the atexit() handlers, the static destructors and
   libpthread's own exit: the reents still on a task, which is then one
   that outlives the program.  A task that has ended took its reent with
   it.  The task lists change from interrupts too, so they are walked in
   Disable(); the memory is freed after it.  Not static: the exit list
   names it.  */
void __exitreent(void) {
	struct List gone;
	struct MemList *ml;
	struct Node *t;

	if (!cookie)
		return;		/* no task but the main one ever had one */
	newlist(&gone);
	Disable();
	sweep(SysBase->ThisTask, &gone);
	for (t = SysBase->TaskReady.lh_Head; t->ln_Succ; t = t->ln_Succ)
		sweep((struct Task *)t, &gone);
	for (t = SysBase->TaskWait.lh_Head; t->ln_Succ; t = t->ln_Succ)
		sweep((struct Task *)t, &gone);
	Enable();
	while ((ml = (struct MemList *)RemHead(&gone)))
		FreeEntry(ml);
}

ADD2EXIT(__exitreent, -90);
