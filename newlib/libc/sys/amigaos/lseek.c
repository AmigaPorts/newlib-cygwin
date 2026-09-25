/* lseek for m68k-amigaos.  The syscalls/ connectors of newlib are not
   built for this target, so both the _name and the public name are
   defined here.  */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <inline/dos.h>

extern BPTR __check_fno(unsigned file);

_off_t _lseek(int file, _off_t ptr, int dir) {
	BPTR fh = __check_fno(file);

	if (!fh) {
		errno = EBADF;
		return -1;
	}
	if (dir != SEEK_SET && dir != SEEK_CUR && dir != SEEK_END) {
		errno = EINVAL;
		return -1;
	}

	SetIoErr(0);
	Seek(fh, ptr, dir - 1);
	int err = IoErr();
	if (err) {
		errno = EIO;
		return -1;
	}
	if (dir == SEEK_SET)
		return ptr;
	return Seek(fh, 0, OFFSET_CURRENT);
}

off_t lseek(int file, off_t ptr, int dir) {
	return _lseek(file, ptr, dir);
}
