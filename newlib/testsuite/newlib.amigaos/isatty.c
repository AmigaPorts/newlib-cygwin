/* Descriptor type, rather than descriptor number, determines isatty().
   Pass any argument on AmigaOS to also test an interactive CON: handle;
   the simulator used by the default tests does not implement CON:.  */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <proto/dos.h>

static int failures;
static int checks;

static void check(int ok, const char *what)
{
  checks++;
  if (!ok)
    {
      printf("FAIL: %s (errno %d)\n", what, errno);
      failures++;
    }
}

static void not_tty(int fd, int error, const char *what)
{
  errno = 0;
  check(isatty(fd) == 0 && errno == error, what);
}

int main(int argc, char **argv)
{
  int fd;
  int interactive = IsInteractive(Output()) != 0;

  (void)argv;
  errno = 0;
  check(isatty(STDOUT_FILENO) == interactive
        && (interactive || errno == ENOTTY), "stdout redirection");
  not_tty(-1, EBADF, "negative descriptor");
  not_tty(INT_MAX, EBADF, "out of range descriptor");
  fd = open("T:newlib-isatty", O_CREAT | O_TRUNC | O_RDWR, 0600);
  check(fd >= 3, "open regular file");
  if (fd >= 0)
    {
      not_tty(fd, ENOTTY, "regular file");
      check(close(fd) == 0, "close regular file");
      not_tty(fd, EBADF, "closed descriptor");
      unlink("T:newlib-isatty");
    }
  if (argc > 1)
    {
      fd = open("CON:0/0/320/80/newlib isatty test", O_RDWR);
      check(fd >= 3, "open console above standard descriptors");
      if (fd >= 0)
        {
          check(isatty(fd) == 1, "interactive console");
          close(fd);
        }
    }
  printf("isatty: %d checks, %d failures\n", checks, failures);
  return failures;
}
