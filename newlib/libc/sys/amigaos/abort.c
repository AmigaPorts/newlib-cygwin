#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void abort(void)
{
  raise(SIGABRT);
  /* write(), not stdio: abort() must not flush or close streams, and
     assert() should not link stdio just for this.  */
  write(2, "Program aborted\n", 16);
  /* _exit(): no atexit() handlers and no stdio cleanup, as POSIX
     requires; the exit list still frees memory and closes libraries.  */
  _exit(127);
}
