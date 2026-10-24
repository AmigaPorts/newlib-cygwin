/* sleep and usleep for m68k-amigaos: dos.library's Delay(), in ticks of
   1/50 s.  A fraction of a tick rounds up, so usleep(1) still waits.  */

#include <unistd.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <inline/dos.h>

unsigned sleep(unsigned seconds) {
	/* Delay() takes a LONG; the largest it can wait is still over a year.  */
	while (seconds > 0x3fffffffUL / TICKS_PER_SECOND) {
		Delay(0x3fffffffL);
		seconds -= 0x3fffffffUL / TICKS_PER_SECOND;
	}
	if (seconds)
		Delay(seconds * TICKS_PER_SECOND);
	return 0;
}

int usleep(useconds_t usec) {
	unsigned long ticks = (usec + 1000000UL / TICKS_PER_SECOND - 1)
		/ (1000000UL / TICKS_PER_SECOND);

	if (ticks)
		Delay(ticks);
	return 0;
}
