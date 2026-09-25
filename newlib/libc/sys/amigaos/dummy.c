/* Stubs for system calls that have no implementation on m68k-amigaos.
   The real ones live in one file each (getpid.c, lseek.c, open.c,
   unlink.c, ...), so a program links only what it calls.  */

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <exec/types.h>

/* Signals are not delivered to other tasks.  */
int _kill(pid_t pid, int sig) {
	errno = ENOSYS;
	return -1;
}

/* Nothing is reported about the file.  */
int _fstat(int file, struct stat *st) {
	return 0;
}

/* No device control on this target.  */
int __stdargs
ioctl (int fd, unsigned long cmd, ...)
{
  return -1;
}
