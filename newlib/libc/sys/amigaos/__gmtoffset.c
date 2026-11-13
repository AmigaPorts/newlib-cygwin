/* The time zone of the system clock for m68k-amigaos.  */

#include <exec/execbase.h>
#include <libraries/locale.h>
#include <proto/exec.h>
#include <proto/locale.h>
#include "stabs.h"

/* Seconds west of UTC of the system clock, which runs on local time: the
   offset of the default locale, 0 when locale.library is not in use.  Unix
   time is the clock plus this, for time(), file dates and localtime()
   alike; TZ changes what localtime() makes of it, not the clock.  */
long __gmtoffset = 0;

void __initgmtoffset(void) {
	struct LocaleBase *LocaleBase;

	/* Only when the library is in memory already, as it is once IPrefs has
	   run: the offset is not worth loading it from disk for, or a requester
	   for a missing LIBS:, at the start of every program.  */
	Forbid();
	LocaleBase = (struct LocaleBase *)FindName(&SysBase->LibList, (STRPTR)"locale.library");
	if (LocaleBase)
		LocaleBase = (struct LocaleBase *)OpenLibrary((STRPTR)"locale.library", 38);
	Permit();
	if (LocaleBase) {
		struct Locale *locale = OpenLocale(0);
		if (locale) {
			__gmtoffset = locale->loc_GMTOffset * 60;
			CloseLocale(locale);
		}
		CloseLibrary((struct Library *)LocaleBase);
	}
}

ADD2INIT(__initgmtoffset, -11);
