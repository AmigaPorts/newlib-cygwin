/* mkdir and rmdir for m68k-amigaos.  The syscalls/ connectors of newlib
   are not built for this target, so both _mkdir and mkdir are defined
   here.  */

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/dos.h>
#include <inline/dos.h>

extern void __seterrno(void);

int _mkdir(const char *path, mode_t mode) {
	BPTR lock = CreateDir((CONST_STRPTR)path);

	if (!lock) {
		__seterrno();
		return -1;
	}
	UnLock(lock);
	return chmod(path, mode);
}

int mkdir(const char *path, mode_t mode) {
	return _mkdir(path, mode);
}

/* DeleteFile() removes empty directories too; it is only refused a file
   here, which POSIX wants reported as ENOTDIR.  */
int rmdir(const char *path) {
	struct FileInfoBlock *fib;
	BPTR lock;
	LONG type = 0;

	if ((lock = Lock((CONST_STRPTR)path, SHARED_LOCK)) == 0) {
		__seterrno();
		return -1;
	}
	if ((fib = AllocDosObject(DOS_FIB, NULL)) != NULL) {
		if (Examine(lock, fib))
			type = fib->fib_DirEntryType;
		FreeDosObject(DOS_FIB, fib);
	}
	UnLock(lock);
	if (type < 0) {
		errno = ENOTDIR;
		return -1;
	}
	if (!DeleteFile((CONST_STRPTR)path)) {
		__seterrno();
		return -1;
	}
	return 0;
}
