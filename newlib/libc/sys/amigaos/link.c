/* link and symlink for m68k-amigaos: MakeLink().  A hard link takes a
   lock on the existing file, a soft link the target's name.  The syscalls/
   connectors of newlib are not built for this target, so both _link and
   link are defined here.  */

#include <errno.h>
#include <unistd.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <inline/dos.h>

extern void __seterrno(void);

int _link(const char *oldpath, const char *newpath) {
	BPTR lock = Lock((CONST_STRPTR)oldpath, SHARED_LOCK);
	LONG ok;

	if (!lock) {
		__seterrno();
		return -1;
	}
	ok = MakeLink((CONST_STRPTR)newpath, (LONG)lock, LINK_HARD);
	UnLock(lock);
	if (!ok) {
		__seterrno();
		return -1;
	}
	return 0;
}

int link(const char *oldpath, const char *newpath) {
	return _link(oldpath, newpath);
}

int symlink(const char *target, const char *linkpath) {
	if (!MakeLink((CONST_STRPTR)linkpath, (LONG)target, LINK_SOFT)) {
		__seterrno();
		return -1;
	}
	return 0;
}
