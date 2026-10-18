/* chdir, fchdir and getcwd for m68k-amigaos.  The current directory is
   the process's lock; the shell's own is given back at exit, and the locks
   made here are released.  */

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/dos.h>
#include <inline/dos.h>
#include <stabs.h>

extern BPTR __check_fno(unsigned file);
extern void __seterrno(void);

/* The lock the process had before the first chdir(), still the shell's.  */
static BPTR origdir;
static int changed;

/* Make lock the current directory; it is ours from here on.  */
static int setcwd(BPTR lock) {
	struct FileInfoBlock *fib;
	LONG type = 0;
	BPTR old;

	if ((fib = AllocDosObject(DOS_FIB, NULL)) != NULL) {
		if (Examine(lock, fib))
			type = fib->fib_DirEntryType;
		FreeDosObject(DOS_FIB, fib);
	}
	if (type < 0) {
		UnLock(lock);
		errno = ENOTDIR;
		return -1;
	}
	old = CurrentDir(lock);
	if (changed)
		UnLock(old);
	else {
		origdir = old;
		changed = 1;
	}
	return 0;
}

int chdir(const char *path) {
	BPTR lock = Lock((CONST_STRPTR)path, SHARED_LOCK);

	if (!lock) {
		__seterrno();
		return -1;
	}
	return setcwd(lock);
}

int fchdir(int fd) {
	BPTR fh = __check_fno(fd), lock;

	if (!fh) {
		errno = EBADF;
		return -1;
	}
	if ((lock = DupLockFromFH(fh)) == 0) {
		__seterrno();
		return -1;
	}
	return setcwd(lock);
}

/* The name of the current directory's lock, like GetCurrentDirName()
   does, which the CI simulator does not have.  ERANGE for a buffer too
   small, as POSIX says.  */
char *getcwd(char *buf, size_t size) {
	char *b = buf;
	BPTR lock;
	LONG ok;

	if (size == 0) {
		errno = EINVAL;
		return NULL;
	}
	if (b == NULL && (b = malloc(size)) == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	if ((lock = Lock((CONST_STRPTR)"", SHARED_LOCK)) == 0) {
		__seterrno();
		if (b != buf)
			free(b);
		return NULL;
	}
	ok = NameFromLock(lock, (STRPTR)b, size);
	UnLock(lock);
	if (!ok) {
		if (IoErr() == ERROR_LINE_TOO_LONG)
			errno = ERANGE;
		else
			__seterrno();
		if (b != buf)
			free(b);
		return NULL;
	}
	return b;
}

void __exitchdir(void) {
	if (changed)
		UnLock(CurrentDir(origdir));
}

ADD2EXIT(__exitchdir, -35);
