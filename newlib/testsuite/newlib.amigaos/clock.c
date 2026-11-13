/* The clock and the file dates as Unix time, on a system an hour east of
   UTC: time() and gettimeofday() are DateStamp() plus the offset, whatever
   TZ says; localtime() is the system's without TZ and TZ's
   with it, and mktime() undoes it; a new file's date is the clock's, and
   utime() stores local time.  Each check prints what failed; the exit code
   is the number of failures.  */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dos/dos.h>
#include <proto/dos.h>

#define NAME "T:newlib-clock-test"

/* 1978-01-01 in seconds since 1970-01-01 */
#define AMIGA_EPOCH 252460800L
#define EAST (-3600L)

extern long __gmtoffset;

static int failures;
static const char *stage = "";

static void check(int ok, const char *what)
{
  if (!ok)
    {
      printf("FAIL: %s%s\n", what, stage);
      failures++;
    }
}

/* DS in Unix time.  */
static long unixtime(const struct DateStamp *ds)
{
  return (ds->ds_Days * 1440L + ds->ds_Minute) * 60 + ds->ds_Tick / TICKS_PER_SECOND
    + AMIGA_EPOCH + EAST;
}

static long now(void)
{
  struct DateStamp ds;

  DateStamp(&ds);
  return unixtime(&ds);
}

/* The minute of the day of T, WEST seconds west of UTC.  */
static long minute(time_t t, long west)
{
  return (t - west) / 60 % 1440;
}

static void clocks(void)
{
  struct timeval tv;
  long before, after;
  time_t t;

  before = now();
  t = time(NULL);
  gettimeofday(&tv, NULL);
  after = now();
  check(t >= before && t <= after, "time() is the DateStamp plus the offset");
  check(tv.tv_sec >= before && tv.tv_sec <= after, "gettimeofday() too");
  check(tv.tv_usec >= 0 && tv.tv_usec < 1000000, "with microseconds below a second");
}

int main(void)
{
  struct FileInfoBlock *fib;
  struct utimbuf ut;
  struct stat st;
  struct tm tm;
  long before, after;
  time_t t;
  BPTR fh, lock;

  /* what the startup code makes of a locale an hour east of UTC */
  __gmtoffset = EAST;
  unsetenv("TZ");

  clocks();

  t = time(NULL);
  tm = *localtime(&t);
  check(tm.tm_hour * 60 + tm.tm_min == minute(t, EAST), "localtime() is an hour ahead of UTC");
  check(mktime(&tm) == t, "mktime() undoes localtime()");

  before = now();
  if ((fh = Open((STRPTR) NAME, MODE_NEWFILE)))
    Close(fh);
  after = now();
  check(fh != 0, "create a file");
  check(stat(NAME, &st) == 0, "stat() on the new file succeeds");
  check(st.st_mtime >= before - 2 && st.st_mtime <= after + 2, "its date is the clock's");

  ut.actime = ut.modtime = 1000000000;
  check(utime(NAME, &ut) == 0, "utime() succeeds");
  check(stat(NAME, &st) == 0 && st.st_mtime == 1000000000, "stat() reads the date back");
  fib = AllocDosObject(DOS_FIB, NULL);
  lock = Lock((STRPTR) NAME, SHARED_LOCK);
  check(fib && lock && Examine(lock, fib) && unixtime(&fib->fib_Date) == 1000000000,
        "the file's DateStamp is that date in local time");
  if (lock)
    UnLock(lock);
  FreeDosObject(DOS_FIB, fib);
  DeleteFile((STRPTR) NAME);

  setenv("TZ", "EST5", 1);
  tzset();
  stage = ", with TZ=EST5";
  clocks();
  t = time(NULL);
  tm = *localtime(&t);
  check(tm.tm_hour * 60 + tm.tm_min == minute(t, 5 * 3600L), "localtime() is five hours behind UTC");
  check(mktime(&tm) == t, "mktime() undoes localtime()");

  printf("%d failures\n", failures);
  return failures;
}
