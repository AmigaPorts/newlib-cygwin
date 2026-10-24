#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <clib/dos_protos.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <inline/dos.h>
#include <stabs.h>

#pragma GCC optimize ("no-toplevel-reorder")

BPTR * __fh;
int __maxfh;
/* Per descriptor, whether it was opened with O_APPEND: AmigaDOS has no
   such mode, so _write() seeks to the end first.  */
static unsigned char *__fhappend;

extern void __seterrno(void);

/* Whether name exists, for the open() flags AmigaDOS has no mode for.  A
   file another handle holds exclusively cannot be locked, but exists.  */
static int __exists(const char *name) {
	BPTR lock = Lock(name, SHARED_LOCK);

	if (lock) {
		UnLock(lock);
		return 1;
	}
	return IoErr() == ERROR_OBJECT_IN_USE;
}

asm("_open: .globl _open");
int _open(const char *name, int flags, ...) {
	int mode;

	/* O_EXCL, and O_TRUNC without O_CREAT, test for the file first: not
	   atomic, as AmigaDOS offers nothing that is.  */
	if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL) && __exists(name)) {
		errno = EEXIST;
		return -1;
	}
	if ((flags & (O_CREAT | O_TRUNC)) == O_TRUNC && !__exists(name)) {
		__seterrno();
		return -1;
	}

	if (flags & O_TRUNC)
		mode = MODE_NEWFILE;
	else if (flags & O_CREAT)
		mode = MODE_READWRITE;
	else
		mode = MODE_OLDFILE;

	BPTR r = Open(name, mode);
	/* Some handlers do not create a missing file for MODE_READWRITE
	   (libnix has the same fallback).  */
	if (!r && mode == MODE_READWRITE && IoErr() == ERROR_OBJECT_NOT_FOUND)
		r = Open(name, MODE_NEWFILE);
	if (!r) {
		__seterrno();
		return -1;
	}

	int fh = 3;
	while(fh < __maxfh) {
		if (!__fh[fh])
			break;
		++fh;
	}

	if (fh == __maxfh) {
		int n = __maxfh + __maxfh + 8;
		__fh = (BPTR*)realloc(__fh, n * sizeof(BPTR));
		__fhappend = (unsigned char *)realloc(__fhappend, n);
		if (!__fh || !__fhappend) {
			exit(ENOMEM);
		}
		while (__maxfh < n) {
			__fhappend[__maxfh] = 0;
			__fh[__maxfh++] = 0;
		}
	}
	__fh[fh] = r;
	__fhappend[fh] = (flags & O_APPEND) != 0;
	return fh;
}

asm("_close: .globl _close");
int _close(int file) {
	int r = -1;
	/* 0-2 are the shell's Input() and Output(), or the Workbench window
	   __exit_fh() closes: not ours to Close().  Keep them, too: clearing
	   __fh[0] would make __check_fno() open the window again.  */
	if (file < 3)
		return 0;
	if (file < __maxfh) {
		r = Close(__fh[file]) == 0;
		__fh[file] = 0;
		__fhappend[file] = 0;
	}
	return r;
}

static BPTR wbstdout;

/* The DOS handle for descriptor file, or 0 for an invalid or closed one;
   opens the stdio handles on first use.  Shared with lseek.c.  */
BPTR __check_fno(unsigned file) {

	if (file >= __maxfh)
		return 0;

	if (file < 3 && __fh[0] == 0) {
		__fh[0] = Input();

		if (__fh[0] == 0) {
			__fh[0] = __fh[1] = __fh[2] = wbstdout = Open("*", MODE_OLDFILE);
		} else {
			__fh[1] = __fh[2] = Output();
		}
	}

	return __fh[file];
}

asm("_write: .globl _write");
int _write(int file, char *ptr, int len) {
	if (__check_fno(file)) {
		/* A stream cannot seek; it writes at its end anyway.  */
		if (__fhappend[file])
			Seek(__fh[file], 0, OFFSET_END);
		return Write(__fh[file], ptr, len);
	}
	return -1;
}

asm("_read: .globl _read");
int _read(int file, char *ptr, int len) {
	if (__check_fno(file))
		return Read(__fh[file], ptr, len);
	return -1;
}


void __init_fh() {
	__fh = (BPTR *)calloc(4, sizeof(BPTR));
	__fhappend = (unsigned char *)calloc(4, 1);
	if (!__fh || !__fhappend)
		exit(ENOMEM);

	__maxfh = 4;
}

void __exit_fh() {
	if (wbstdout)
		Close(wbstdout);
}

int _isatty(int file) {
	return (unsigned)file <= 2;
}

int isatty(int file) {
	return _isatty(file);
}

ADD2INIT(__init_fh, -50);
ADD2EXIT(__exit_fh, -50);

