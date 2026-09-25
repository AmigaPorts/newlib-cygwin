#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <clib/dos_protos.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <inline/dos.h>
#include <proto/exec.h>
#include <inline/exec.h>

int _kill(int pid, int sig) {

}

/* There are no process ids; the task address is unique while the task
   lives.  Shift it so the result stays positive even for memory above
   2 GB (tasks are at least longword aligned, so nothing is lost).  */
pid_t _getpid(void) {
	return (pid_t)((unsigned long)FindTask(0) >> 2);
}

/* The syscalls/ connectors are not built for this target, so provide the
   public name too; libgcov and other library code call getpid directly.  */
pid_t getpid(void) {
	return _getpid();
}

extern BPTR * __fh;
extern int __maxfh;


int _lseek(int file, int ptr, int dir) {
	if (file >= __maxfh) {
		errno = EIO;
		return -1;
	}

	SetIoErr(0);
	Seek(__fh[file], ptr, dir - 1);
	int err = IoErr();
	if (err) {
		errno = EIO;
		return -1;
	}
	if (dir == SEEK_SET)
		return ptr;
	return Seek(__fh[file], 0, OFFSET_CURRENT);
}

int lseek(int file, int ptr, int dir) {
	return _lseek(file, ptr, dir);
}

int _fstat(int file, struct stat *st) {
	return 0;
}

int _unlink(char *name) {
	return !DeleteFile(name);
}

int __stdargs
ioctl (int fd, unsigned long cmd, ...)
{
  return -1;
}
