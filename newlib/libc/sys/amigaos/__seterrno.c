/* errno from the last dos.library error, for the system calls that go
   straight to dos.library.  The table is libnix's. */

#include <errno.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <inline/dos.h>

static const struct {
	short ioerr;
	short err;
} __errortable[] = {
	{ ERROR_NO_FREE_STORE, ENOMEM },
	{ ERROR_TASK_TABLE_FULL, EPROCLIM },
	{ ERROR_TOO_MANY_ARGS, E2BIG },
	{ ERROR_LINE_TOO_LONG, E2BIG },
	{ ERROR_OBJECT_IN_USE, ETXTBSY },
	{ ERROR_OBJECT_EXISTS, EEXIST },
	{ ERROR_DIR_NOT_FOUND, ENOENT },
	{ ERROR_OBJECT_NOT_FOUND, ENOENT },
	{ ERROR_BAD_STREAM_NAME, ENAMETOOLONG },
	{ ERROR_OBJECT_TOO_LARGE, EFBIG },
	{ ERROR_ACTION_NOT_KNOWN, ENODEV },
	{ ERROR_OBJECT_WRONG_TYPE, ENOTDIR },
	{ ERROR_DISK_NOT_VALIDATED, EBUSY },
	{ ERROR_DISK_WRITE_PROTECTED, EROFS },
	{ ERROR_RENAME_ACROSS_DEVICES, EXDEV },
	{ ERROR_DIRECTORY_NOT_EMPTY, ENOTEMPTY },
	{ ERROR_TOO_MANY_LEVELS, ENAMETOOLONG },
	{ ERROR_DEVICE_NOT_MOUNTED, ENXIO },
	{ ERROR_SEEK_ERROR, ESPIPE },
	{ ERROR_DISK_FULL, ENOSPC },
	{ ERROR_DELETE_PROTECTED, EACCES },
	{ ERROR_WRITE_PROTECTED, EACCES },
	{ ERROR_READ_PROTECTED, EACCES },
	{ ERROR_NOT_A_DOS_DISK, EFTYPE },
	{ ERROR_NO_DISK, ENXIO },
	{ ERROR_IS_SOFT_LINK, ELOOP },
	{ ERROR_BAD_HUNK, ENOEXEC },
	{ ERROR_NOT_IMPLEMENTED, ENOSYS },
	{ ERROR_NOT_EXECUTABLE, EACCES },
};

void __seterrno(void) {
	LONG ioerr = IoErr();
	unsigned i;

	for (i = 0; i < sizeof(__errortable) / sizeof(__errortable[0]); i++) {
		if (__errortable[i].ioerr == ioerr) {
			errno = __errortable[i].err;
			return;
		}
	}
	errno = ioerr ? EPERM : EIO;
}
