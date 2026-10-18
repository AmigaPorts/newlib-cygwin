/* <utime.h> for m68k-amigaos: utime() is in libc/sys/amigaos/utime.c.
   newlib's own <sys/utime.h> has the structure but no prototype.  */

#ifndef _SYS_UTIME_H
#define _SYS_UTIME_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct utimbuf {
	time_t actime;		/* access time, which AmigaDOS does not keep */
	time_t modtime;		/* modification time */
};

__stdargs int utime(const char *__path, const struct utimbuf *__times);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_UTIME_H */
