/* opendir, readdir, rewinddir and closedir for m68k-amigaos, with
   Examine() and ExNext().  newlib's own libc/posix versions are not
   built for this target: they need getdents().

   Not ExAll(): its autodoc lists bugs in V36 and in the V39 FFS, and it
   keeps state in the handler until the listing is over.  ExNext() asks
   for one entry at a time and needs no cleanup when the reader stops.  */

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/dos.h>
#include <inline/dos.h>

extern void __seterrno(void);

struct __dirstream {
	BPTR lock;
	struct FileInfoBlock *fib;
	int started;		/* Examine() done, ExNext() continues it */
	struct dirent ent;
};

DIR *opendir(const char *path) {
	DIR *d;

	if ((d = calloc(1, sizeof(*d))) == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	if ((d->fib = AllocDosObject(DOS_FIB, NULL)) == NULL) {
		free(d);
		errno = ENOMEM;
		return NULL;
	}
	if ((d->lock = Lock((CONST_STRPTR)path, SHARED_LOCK)) == 0
	    || !Examine(d->lock, d->fib)) {
		__seterrno();
		closedir(d);
		return NULL;
	}
	if (d->fib->fib_DirEntryType < 0) {
		closedir(d);
		errno = ENOTDIR;
		return NULL;
	}
	d->started = 1;
	return d;
}

struct dirent *readdir(DIR *d) {
	size_t len;

	if (!d->started) {
		if (!Examine(d->lock, d->fib)) {
			__seterrno();
			return NULL;
		}
		d->started = 1;
	}
	if (!ExNext(d->lock, d->fib)) {
		if (IoErr() != ERROR_NO_MORE_ENTRIES)
			__seterrno();
		return NULL;
	}

	/* Some programs take a zero inode for a deleted entry.  */
	d->ent.d_ino = d->fib->fib_DiskKey ? d->fib->fib_DiskKey : 1;
	d->ent.d_reclen = sizeof(d->ent);
	d->ent.d_type = d->fib->fib_DirEntryType < 0 ? DT_REG
		: d->fib->fib_DirEntryType == ST_SOFTLINK ? DT_LNK : DT_DIR;
	len = strlen((const char *)d->fib->fib_FileName);
	if (len > MAXNAMLEN)
		len = MAXNAMLEN;
	memcpy(d->ent.d_name, d->fib->fib_FileName, len);
	d->ent.d_name[len] = 0;
	d->ent.d_namlen = len;
	return &d->ent;
}

/* The next readdir() starts over with Examine().  */
void rewinddir(DIR *d) {
	d->started = 0;
}

int closedir(DIR *d) {
	if (d->lock)
		UnLock(d->lock);
	FreeDosObject(DOS_FIB, d->fib);
	free(d);
	return 0;
}
