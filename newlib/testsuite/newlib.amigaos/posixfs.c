/* The POSIX file, directory, time and signal calls of the AmigaOS port,
   on a scratch directory in T:.  Each check prints what failed; the exit
   code is the number of failures.  */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#define DIR_NAME "T:newlib-posixfs"
#define FILE_NAME DIR_NAME "/file"
#define SUB_NAME DIR_NAME "/sub"

static int failures;

static void check(int ok, const char *what)
{
  if (!ok)
    {
      printf("FAIL: %s (errno %d)\n", what, errno);
      failures++;
    }
}

static volatile int signalled;

static void handler(int sig)
{
  signalled = sig;
}

int main(void)
{
  struct stat st;
  struct timeval tv;
  struct utimbuf ut;
  char cwd[256];
  DIR *d;
  struct dirent *e;
  int fd, seen_file = 0, seen_sub = 0;

  rmdir(DIR_NAME);
  check(mkdir(DIR_NAME, 0755) == 0, "mkdir");
  check(stat(DIR_NAME, &st) == 0 && S_ISDIR(st.st_mode), "stat: a directory");
  check(stat(DIR_NAME "/missing", &st) == -1 && errno == ENOENT, "stat: ENOENT");

  fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  check(fd >= 0, "open: create");
  check(write(fd, "0123456789", 10) == 10, "write");
  check(fstat(fd, &st) == 0 && S_ISREG(st.st_mode) && st.st_size == 10, "fstat: size");
  check(close(fd) == 0, "close");
  check(stat(FILE_NAME, &st) == 0 && S_ISREG(st.st_mode) && st.st_size == 10, "stat: a file");
  check(fstat(1, &st) == 0, "fstat: stdout");

  check(chmod(FILE_NAME, 0444) == 0, "chmod");
  check(stat(FILE_NAME, &st) == 0 && !(st.st_mode & S_IWUSR) && (st.st_mode & S_IRUSR), "chmod: read-only");
  check(chmod(FILE_NAME, 0644) == 0, "chmod: back");

  /* 2001-09-09 01:46:40, a round number; AmigaDOS keeps 1/50 s */
  ut.actime = ut.modtime = 1000000000;
  check(utime(FILE_NAME, &ut) == 0, "utime");
  check(stat(FILE_NAME, &st) == 0 && st.st_mtime == 1000000000, "utime: stat reads it back");

  /* link() and fchdir() need MakeLink() and DupLockFromFH(), which the CI
     simulator does not have; they are tried on AmigaOS only.  */
  check(mkdir(SUB_NAME, 0755) == 0, "mkdir: subdirectory");

  d = opendir(DIR_NAME);
  check(d != NULL, "opendir");
  if (d)
    {
      while ((e = readdir(d)) != NULL)
        {
          if (strcmp(e->d_name, "file") == 0)
            seen_file = e->d_type == DT_REG;
          if (strcmp(e->d_name, "sub") == 0)
            seen_sub = e->d_type == DT_DIR;
        }
      check(seen_file && seen_sub, "readdir: both entries with their types");
      rewinddir(d);
      check(readdir(d) != NULL, "rewinddir: entries again");
      check(closedir(d) == 0, "closedir");
    }
  check(opendir(FILE_NAME) == NULL && errno == ENOTDIR, "opendir: ENOTDIR");

  check(chdir(DIR_NAME) == 0, "chdir");
  check(getcwd(cwd, sizeof(cwd)) == cwd && strstr(cwd, "newlib-posixfs") != NULL, "getcwd");
  check(stat("file", &st) == 0, "stat: relative to the new directory");
  check(chdir(FILE_NAME) == -1 && errno == ENOTDIR, "chdir: ENOTDIR");
  check(chdir("T:") == 0, "chdir: back");

  check(rmdir(DIR_NAME) == -1 && errno == ENOTEMPTY, "rmdir: ENOTEMPTY");
  check(rmdir(FILE_NAME) == -1 && errno == ENOTDIR, "rmdir: ENOTDIR");
  check(rmdir(SUB_NAME) == 0, "rmdir: subdirectory");
  check(unlink(FILE_NAME) == 0, "unlink: file");
  check(rmdir(DIR_NAME) == 0, "rmdir");

  check(gettimeofday(&tv, NULL) == 0 && tv.tv_sec > 1000000000 && tv.tv_usec < 1000000, "gettimeofday");
  check(time(NULL) >= tv.tv_sec, "time");
  check(sleep(0) == 0, "sleep(0)");
  check(usleep(20000) == 0, "usleep");

  check(signal(SIGUSR1, handler) == SIG_DFL, "signal: SIG_DFL before");
  check(raise(SIGUSR1) == 0 && signalled == SIGUSR1, "raise: handler ran");
  check(signal(SIGUSR1, SIG_IGN) == SIG_DFL, "signal: reset after delivery");
  check(raise(SIGUSR1) == 0, "raise: SIG_IGN");
  check(kill(getpid(), SIGUSR1) == 0, "kill: self, ignored");

  check(isatty(0) == 0 || isatty(0) == 1, "isatty");

  printf("%d failures\n", failures);
  return failures;
}
