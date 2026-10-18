/* Stubs for system calls that have no implementation on m68k-amigaos.
   The real ones live in one file each (getpid.c, lseek.c, open.c,
   unlink.c, ...), so a program links only what it calls.  */

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <exec/types.h>

/* No device control on this target.  */
int __stdargs
ioctl (int fd, unsigned long cmd, ...)
{
  return -1;
}
