/* unlink for m68k-amigaos.  The syscalls/ connectors of newlib are not
   built for this target, so both the _name and the public name are
   defined here.  */

#include <errno.h>
#include <unistd.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <inline/dos.h>

int _unlink(const char *name) {
	if (DeleteFile(name))
		return 0;

	switch (IoErr()) {
	case ERROR_OBJECT_NOT_FOUND:
		errno = ENOENT;
		break;
	case ERROR_DIRECTORY_NOT_EMPTY:
		errno = ENOTEMPTY;
		break;
	case ERROR_OBJECT_IN_USE:
	case ERROR_DELETE_PROTECTED:
	case ERROR_WRITE_PROTECTED:
		errno = EACCES;
		break;
	case ERROR_DISK_WRITE_PROTECTED:
		errno = EROFS;
		break;
	default:
		errno = EIO;
	}
	return -1;
}

int unlink(const char *name) {
	return _unlink(name);
}
