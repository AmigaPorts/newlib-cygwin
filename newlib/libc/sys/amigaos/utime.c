/* utime and utimes for m68k-amigaos: SetFileDate().  A file has one date
   on AmigaDOS, so the modification time is stored and the access time
   ignored.  The conversion is stat()'s in reverse: seconds since 1970 as
   if UTC into a DateStamp since 1978, no time zone applied.  */

#include <errno.h>
#include <time.h>
#include <utime.h>
#include <sys/time.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <inline/dos.h>

extern void __seterrno(void);

#define DAYS_1970_TO_1978 2922L
#define SECS_PER_DAY (24L * 60L * 60L)

static void stamp_from_time(struct DateStamp *ds, time_t sec, long usec) {
	sec -= DAYS_1970_TO_1978 * SECS_PER_DAY;
	ds->ds_Days = sec / SECS_PER_DAY;
	sec %= SECS_PER_DAY;
	ds->ds_Minute = sec / 60;
	ds->ds_Tick = (sec % 60) * TICKS_PER_SECOND
		+ usec / (1000000L / TICKS_PER_SECOND);
}

int utimes(const char *path, const struct timeval times[2]) {
	struct DateStamp ds;

	if (times)
		stamp_from_time(&ds, times[1].tv_sec, times[1].tv_usec);
	else
		DateStamp(&ds);
	if (!SetFileDate((CONST_STRPTR)path, &ds)) {
		__seterrno();
		return -1;
	}
	return 0;
}

int utime(const char *path, const struct utimbuf *times) {
	struct timeval tv[2];

	if (!times)
		return utimes(path, NULL);
	tv[0].tv_sec = times->actime;
	tv[1].tv_sec = times->modtime;
	tv[0].tv_usec = tv[1].tv_usec = 0;
	return utimes(path, tv);
}
