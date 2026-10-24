/* kill for m68k-amigaos.  newlib's raise() calls _kill(getpid(), sig) for
   a signal with SIG_DFL: SIGINT ends the program the way a shell's Break
   does, every other signal is ignored, as libnix does, since programs
   expect to go on after abort()'s raise(SIGABRT) prints its message.

   Another process (a pid is its task address, see getpid.c) gets Ctrl-C
   for SIGINT and SIGTERM, which is all a task can be sent.  */

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <exec/execbase.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <inline/exec.h>

extern struct ExecBase *SysBase;

/* The task with this pid, if it still exists; call under Forbid().  */
static struct Task *findpid(pid_t pid) {
	struct Task *want = (struct Task *)((unsigned long)pid << 2), *t;
	struct List *lists[2] = { &SysBase->TaskReady, &SysBase->TaskWait };
	int i;

	if (want == SysBase->ThisTask)
		return want;
	for (i = 0; i < 2; i++)
		for (t = (struct Task *)lists[i]->lh_Head; t->tc_Node.ln_Succ;
		     t = (struct Task *)t->tc_Node.ln_Succ)
			if (t == want)
				return t;
	return NULL;
}

int _kill(pid_t pid, int sig) {
	struct Task *t;

	if (pid <= 0) {
		errno = EINVAL;
		return -1;
	}
	if (pid == getpid()) {
		if (sig == SIGINT) {
			write(2, "***Break\n", 9);
			exit(20);
		}
		return 0;
	}
	if (sig != SIGINT && sig != SIGTERM) {
		errno = EINVAL;
		return -1;
	}
	Forbid();
	t = findpid(pid);
	if (t)
		Signal(t, SIGBREAKF_CTRL_C);
	Permit();
	if (!t) {
		errno = ESRCH;
		return -1;
	}
	return 0;
}

int kill(pid_t pid, int sig) {
	return _kill(pid, sig);
}
