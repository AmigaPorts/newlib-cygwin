/* getpid for m68k-amigaos.  The syscalls/ connectors of newlib are not
   built for this target, so both the _name and the public name are
   defined here.  */

#include <unistd.h>
#include <sys/types.h>
#include <proto/exec.h>
#include <inline/exec.h>

/* There are no process ids; the task address is unique while the task
   lives.  Shift it so the result stays positive even for memory above
   2 GB (tasks are at least longword aligned, so nothing is lost).  */
pid_t _getpid(void) {
	return (pid_t)((unsigned long)FindTask(0) >> 2);
}

pid_t getpid(void) {
	return _getpid();
}
