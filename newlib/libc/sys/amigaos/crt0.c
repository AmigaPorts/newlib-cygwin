/**
 * Based on nrct0.s from libnix
 */
#include <exec/exec.h>
#include <proto/dos.h>
#include <dos/dosextens.h>
#include <inline/exec.h>
#include <string.h>
#include <reent.h>
#include <stabs.h>
#include "startup.h"

#pragma GCC push_options
#pragma GCC optimize ("no-toplevel-reorder")

extern __stdargs int main(int, char **);
extern __stdargs void perror(const char *string);

__attribute__((section(".list___INIT_LIST__,\"aw\"")))
int __INIT_LIST__[1] = {0};
__attribute__((section(".list___EXIT_LIST__,\"aw\"")))
int __EXIT_LIST__[1] = {0};

__attribute__((section(".end_of_lists,\"aw\"")))
int __ZZZ_LIST__[1] = {0};
__attribute__((section(".end_of_dlists,\"aw\"")))
int __ZZZ_DLIST__[1] = {0};

__stdargs void exit(int);
/* noinline: it leaves through the startup stack pointer, so inlined into
 * a noreturn caller it would lose its final rts.  */
__entrypoint __stdargs int __crt0_exit(int) __attribute__((noinline));
extern void __call_exitprocs(int, void *);
__entrypoint void callfuncs(int * p asm("a2"), unsigned short prioo asm("d2"));

struct ExecBase * SysBase;
static int __savedSp;
static unsigned short cleanupflag;
/* Set by exit(), not by _exit(): only a normal exit flushes stdio.  */
static int normalexit;

/* The command-line initializer owns __argc and __argv.  Their zero-filled
 * defaults are also the documented no-argument state when an application
 * overrides __nocommandline. */
char **__argv;
int __argc;
unsigned long __commandlen;
char *__commandline;

struct WBStartup *_WBenchMsg;

#if defined(__pic__) || defined (__PIC__)
/* The linker script sets this symbol's address, not its contents, to the
   size of .bss in bytes; __far keeps that address absolute.  */
extern char __far __bss_init_size[];
extern int _edata;
void __restore_a4(void);
__saveds
#endif

__entrypoint __regargs void ____start(unsigned long cmdlen, char *cmdline, int sp asm("sp")) {
#if defined(__pic__) || defined (__PIC__)
	asm("lea ___a4_init,a4");
	memset(&_edata, 0, (unsigned long)__bss_init_size);

#endif

	__savedSp = sp;
	__commandlen = cmdlen;
	__commandline = cmdline;

	SysBase = *(struct ExecBase **) 4;

	// check for wb message if not cli
	struct Process * task = (struct Process *) FindTask(0);
	if (!task->pr_CLI) {
		WaitPort(&task->pr_MsgPort);
		_WBenchMsg = (struct WBStartup *) GetMsg(&task->pr_MsgPort);
	}
	callfuncs(&__INIT_LIST__[0] + 1, 0);
	exit(main(__argc, __argv));
}

#if defined(__pic__) || defined (__PIC__)
void __entrypoint __restore_a4(void) {
	asm("lea ___a4_init,a4");
}
#endif

/**
 * exit(): the atexit() and on_exit() handlers, then the rest of the exit.
 */
#if defined(__pic__) || defined (__PIC__)
__saveds
#endif
__stdargs void exit(int rc) {
	__call_exitprocs(rc, NULL);
	/* Only now: a handler that calls _exit() or abort() skips stdio.  */
	normalexit = 1;
	__crt0_exit(rc);
	__builtin_unreachable();
}

/**
 * _exit() and __exit(): the exit list without the stdio cleanup, also
 * when called from an atexit() handler or a destructor during exit().
 */
asm("___exit: .globl ___exit");
#if defined(__pic__) || defined (__PIC__)
__saveds
#endif
__stdargs void _exit(int rc) {
	normalexit = 0;
	__crt0_exit(rc);
	__builtin_unreachable();
}

/**
 * The rest of the exit: run the exit list (library and stdio cleanup,
 * static destructors), then restore the startup stack pointer and return
 * the code from the program's entry point.
 */
#if defined(__pic__) || defined (__PIC__)
__saveds
#endif
__entrypoint __stdargs int __crt0_exit(int rc) {
	register unsigned __d7 __asm("d7");
	asm("move.l %0,d7"::"r"(rc));

	/* Turn the walk around for the exit list only once: an exit() from
	 * an exit handler or a destructor resumes it where it is.  */
	static short exiting;
	if (!exiting) {
		exiting = 1;
		cleanupflag ^= -1;
	}
	callfuncs(&__EXIT_LIST__[0] + 1, -1);

	if (_WBenchMsg) {
		Forbid();
		ReplyMsg((struct Message *) _WBenchMsg);
	}

	asm("move.l %0,sp"::"r"(__savedSp));
	return __d7;
}

/**
 * Loop over functions.
 * - find next prio
 * - call functions matching last prio
 */
__entrypoint void callfuncs(int * q asm("a2"), unsigned short order asm("d2")) {
	for (;;) {
		int * p = q;
		unsigned short curprio = cleanupflag;
		unsigned short nextprio = -1;

		while (*p) {
			unsigned short prio = *((unsigned short *) p + 3) ^ order;

			// invoke
			if (prio == curprio)
				((void (*)(void)) *p)();

			// update next prio
			if (prio < nextprio && prio > curprio)
				nextprio = prio;

			p += 2;
		}
		if (nextprio == curprio)
			break;

		cleanupflag = nextprio;
	}
}

/* These are the elements pointed to by __LIB_LIST__ */
__attribute__((section(".dlist___LIB_LIST__")))
long __LIB_LIST__ = 0;

/* The minimum version of the libraries opened for the program: it sets it by
   defining, say, long __oslibversion = 39;  Common, so that definition wins. */
__attribute__((common)) long __oslibversion;

#pragma GCC push_options
#pragma GCC optimize ("-O2")

/* Opens dos.library itself: the failed library may be dos.library, at
   __oslibversion, or the list may not have reached it yet.  */
static __attribute__((noreturn)) void __openliberror(char const * name) {
	static char const msg[] = " failed to load\n";
	struct DosLibrary * DOSBase = (struct DosLibrary *)OldOpenLibrary("dos.library");

	if (DOSBase) {
		Write(Output(), (APTR)name, strlen(name));
		Write(Output(), (APTR)msg, sizeof msg - 1);
		CloseLibrary((struct Library *)DOSBase);
	}
	exit(20);
}

/**
 * Open all libraries.
 */
void __initlibraries(void) {
	long * l = &__LIB_LIST__ + 1;
	char const * failed = NULL;
	/* Open them all before reporting a failure: __exitlibraries() closes
	   every non-zero base, and the bases not reached yet still hold -1. */
	while (*l) {
		long * base = l++;
		char const * const name = *(char **) (l++);
		*base = (long)(strstr(name, ".resource") ? OpenResource(name) : OpenLibrary(name, __oslibversion));
		if (!*base && !failed)
			failed = name;
	}
	if (failed)
		__openliberror(failed);

	/* On an FPU, the IEEE maths libraries set the FPCR of the task that
	 * opens them: mathieeesingbas to $40 (round to single precision),
	 * mathieeedoubbas to $10 (round toward zero).  Our soft-float goes
	 * through them, so after mathieeesingbas every double would be rounded
	 * to single precision.  A shell's commands also share its process and
	 * inherit what the previous one left, exception flags and trap enables
	 * included.  Start from the FPU's reset values, which is what C expects
	 * (Annex F.8.3): round to nearest, extended precision, no traps, and no
	 * exception flags set.  The FPU instructions are hand-assembled
	 * because the base multilib is built for the 68000. */
	if (SysBase->AttnFlags & AFF_68881) {
		__asm__ volatile ("moveq #0,d0\n\t"
				  ".short 0xf200,0x9000\n\t"	/* fmove.l d0,fpcr */
				  ".short 0xf200,0x8800"	/* fmove.l d0,fpsr */
				  ::: "d0");
	}
}

/**
 * close all opened libraries.
 */
void __exitlibraries(void) {
	long * l = &__LIB_LIST__ + 1;
	while (*l) {
		long * base = l++;
		char const * const name = *(char **) (l++);
		/* *base == 0: it failed to open */
		/* .resource: never closed, exec has no CloseResource() and resources have no Close vector */
		if (*base && !strstr(name, ".resource"))
			CloseLibrary((struct Library *)*base);
	}
}

typedef void (*func_ptr) (void);

__attribute__((section(".list___CTOR_LIST__")))
func_ptr __CTOR_LIST__[] = {0};
__attribute__((section(".list___DTOR_LIST__")))
func_ptr __DTOR_LIST__[] = {0};

void __initcpp() {
  func_ptr *p0 = __CTOR_LIST__ + 1;
  func_ptr *p;
  for (p = p0; *p; p++);
  while (p > p0)
    (*--p)();
}

/* Static, and advanced before each call: a destructor that calls exit()
 * runs the exit list again, which must go on with the next destructor.  */
void __exitcpp() {
  static func_ptr *p = __DTOR_LIST__ + 1;
  while (*p)
    (*p++)();
}

/* Pull the default initializer out of libc unless the application supplies
 * the documented no-argument override. */
static void (*used_commandline)(void) __attribute__((used)) = __nocommandline;

/* Flush and close the streams on exit(), as newlib's generic exit() does.
 * Here it runs from the exit list: after the static destructors, which may
 * still write, and before open.c closes the file handles. */
void __exitstdio(void) {
	if (normalexit && __stdio_exit_handler != NULL)
		(*__stdio_exit_handler)();
}

ADD2INIT(__initlibraries, -100);
ADD2EXIT(__exitlibraries, -100);
ADD2INIT(__initcpp, 100);
ADD2EXIT(__exitcpp, 100);
ADD2EXIT(__exitstdio, 0);

