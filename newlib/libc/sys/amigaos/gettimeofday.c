/* gettimeofday for m68k-amigaos: the DateStamp() clock.  The syscalls/
   connectors of newlib are not built for this target, so both the _name
   and the public name are defined here; time() goes through _gettimeofday.

   A DateStamp is local time since 1978-01-01; _timezone (seconds west of
   UTC, set by tzset() from TZ, 0 without one) turns it into UTC.  */

#include <time.h>
#include <sys/time.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <inline/dos.h>

/* 1978-01-01 in seconds since 1970-01-01 */
#define AMIGA_EPOCH 252460800L

int _gettimeofday(struct timeval *tv, void *tz) {
	struct DateStamp ds;

	if (tv) {
		DateStamp(&ds);
		tv->tv_sec = (ds.ds_Days * 1440L + ds.ds_Minute) * 60L
			+ ds.ds_Tick / TICKS_PER_SECOND + AMIGA_EPOCH + _timezone;
		tv->tv_usec = (ds.ds_Tick % TICKS_PER_SECOND) * (1000000L / TICKS_PER_SECOND);
	}
	if (tz) {
		struct timezone *z = tz;
		z->tz_minuteswest = _timezone / 60;
		z->tz_dsttime = 0;
	}
	return 0;
}

int gettimeofday(struct timeval *tv, struct timezone *tz) {
	return _gettimeofday(tv, tz);
}
