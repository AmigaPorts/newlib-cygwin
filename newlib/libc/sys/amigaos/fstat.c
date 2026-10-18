/* fstat for m68k-amigaos.  The syscalls/ connectors of newlib are not
   built for this target, so both the _name and the public name are
   defined here.  stdio sizes a stream's buffer from st_blksize and
   decides on line buffering from S_ISCHR().

   Only what a file handle can tell without ExamineFH(), which the CI
   simulator does not have: the type, from whether the handle seeks, and
   the size.  Protection, dates and the inode need stat() on the name.  */

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/dos.h>
#include <inline/dos.h>

extern BPTR __check_fno(unsigned file);

int _fstat(int fd, struct stat *st) {
	BPTR fh = __check_fno(fd);
	LONG pos, end;

	if (!fh) {
		errno = EBADF;
		return -1;
	}

	/* A character device, like NIL: (no handler) or a console, until a
	   seek shows a file.  */
	memset(st, 0, sizeof(*st));
	st->st_mode = S_IFCHR | 0666;
	st->st_nlink = 1;
	st->st_blksize = 512;
	st->st_ino = (ino_t)fh;

	if (((struct FileHandle *)BADDR(fh))->fh_Type == NULL)
		return 0;
	SetIoErr(0);
	pos = Seek(fh, 0, OFFSET_END);
	if (pos < 0 || IoErr())
		return 0;
	end = Seek(fh, pos, OFFSET_BEGINNING);
	st->st_mode = S_IFREG | 0666;
	st->st_size = end;
	st->st_blocks = (end + 511) / 512;
	return 0;
}

int fstat(int fd, struct stat *st) {
	return _fstat(fd, st);
}
