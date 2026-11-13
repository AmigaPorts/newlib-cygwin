/* gettimeofday for m68k-amigaos: the DateStamp() clock.  The syscalls/
   connectors of newlib are not built for this target, so both the _name
   and the public name are defined here; time() goes through _gettimeofday.

   A DateStamp is local time since 1978-01-01; __gmtoffset (seconds west of
   UTC) turns it into UTC.  */

#include <sys/time.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <inline/dos.h>

/* 1978-01-01 in seconds since 1970-01-01 */
#define AMIGA_EPOCH 252460800L

extern long __gmtoffset;

/* not inlined into gettimeofday(): one copy is enough */
__attribute__((noinline))
int _gettimeofday(struct timeval *tv, void *tz) {
	if (tv) {
		struct DateStamp ds;
		unsigned short ticks, secs;

		DateStamp(&ds);
		/* under 3000: a 16-bit division, which the 68000 has */
		ticks = ds.ds_Tick;
		secs = ticks / TICKS_PER_SECOND;
		tv->tv_sec = (ds.ds_Days * 1440L + ds.ds_Minute) * 60L
			+ secs + AMIGA_EPOCH + __gmtoffset;
		tv->tv_usec = (unsigned short) (ticks - secs * TICKS_PER_SECOND)
			* (1000000L / TICKS_PER_SECOND);
	}
	if (tz) {
		struct timezone *z = tz;
		z->tz_minuteswest = __gmtoffset / 60;
		z->tz_dsttime = 0;
	}
	return 0;
}

int gettimeofday(struct timeval *tv, struct timezone *tz) {
	return _gettimeofday(tv, tz);
}
