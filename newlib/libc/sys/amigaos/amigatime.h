/* The system clock of m68k-amigaos, for the functions that convert between
   it and Unix time.  */

#ifndef _AMIGATIME_H
#define _AMIGATIME_H

/* 1978-01-01, where a DateStamp starts, in seconds since 1970-01-01:
     unix_time  = amiga_time + AMIGA_EPOCH;
     amiga_time = unix_time  - AMIGA_EPOCH;  */
#define AMIGA_EPOCH 252460800L

/* Seconds west of UTC of the system clock, which Unix time adds as well;
   see __gmtoffset.c */
extern long __gmtoffset;

#endif
