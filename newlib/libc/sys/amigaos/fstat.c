/* fstat for m68k-amigaos.  The syscalls/ connectors of newlib are not
   built for this target, so both the _name and the public name are
   defined here.  stdio sizes a stream's buffer from st_blksize and
   decides on line buffering from S_ISCHR().

   ExamineFH() (V36) tells everything stat() would.  A handler without it,
   or a handle without a handler like NIL:, is a character device unless
   it seeks like a file.  */

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/dos.h>
#include <inline/dos.h>

extern BPTR __check_fno(unsigned file);
extern void __fibstat(struct stat *st, const struct FileInfoBlock *fib);
extern void __seterrno(void);

int _fstat(int fd, struct stat *st) {
	BPTR fh = __check_fno(fd);
	struct FileInfoBlock *fib;
	LONG pos, end;

	if (!fh) {
		errno = EBADF;
		return -1;
	}

	memset(st, 0, sizeof(*st));
	st->st_mode = S_IFCHR | 0666;
	st->st_nlink = 1;
	st->st_blksize = 512;
	st->st_ino = (ino_t)fh;

	if (((struct FileHandle *)BADDR(fh))->fh_Type == NULL)
		return 0;

	if ((fib = AllocDosObject(DOS_FIB, NULL)) != NULL) {
		LONG ok = ExamineFH(fh, fib);

		if (ok)
			__fibstat(st, fib);
		FreeDosObject(DOS_FIB, fib);
		if (ok)
			return 0;
	}

	/* No ExamineFH(): a file if it seeks, and its size is the end.
	   Consoles, pipes and the other stream handlers know neither, and
	   their Seek() returns 0 rather than -1, with IoErr() set to
	   ERROR_ACTION_NOT_KNOWN: 0 is also the position of a file just
	   opened, so only IoErr() tells them apart.  */
	SetIoErr(0);
	pos = Seek(fh, 0, OFFSET_END);
	if (pos == -1 || IoErr() == ERROR_ACTION_NOT_KNOWN)
		return 0;
	/* Seeking back can only fail on a broken handler, but then the
	   position has moved.  */
	end = Seek(fh, pos, OFFSET_BEGINNING);
	if (end == -1) {
		__seterrno();
		return -1;
	}
	st->st_mode = S_IFREG | 0666;
	st->st_size = end;
	st->st_blocks = (end + 511) / 512;
	return 0;
}

int fstat(int fd, struct stat *st) {
	return _fstat(fd, st);
}
