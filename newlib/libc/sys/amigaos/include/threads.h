/* The generic <threads.h> needs a <machine/_threads.h>, which only some
   systems have.  On AmigaOS, threads are POSIX threads: <pthread.h>, with
   -pthread. */
#error "<threads.h>: C11 threads are not available; use <pthread.h> with -pthread"
