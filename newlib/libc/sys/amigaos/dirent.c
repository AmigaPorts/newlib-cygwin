/* opendir, readdir, rewinddir and closedir for m68k-amigaos, reading a
   directory a buffer at a time with ExAll().  newlib's own libc/posix
   versions are not built for this target: they need getdents().

   ExAll() keeps state in the handler until it returns FALSE, so a listing
   that is left before its end is drained in rewinddir() and closedir().  */

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/exall.h>
#include <proto/dos.h>
#include <inline/dos.h>

extern void __seterrno(void);

struct __dirstream {
	BPTR lock;
	struct ExAllControl *eac;
	struct ExAllData *next;	/* the next entry to hand out, in buf */
	unsigned left;		/* entries left in buf */
	LONG more;		/* ExAll() has more after buf */
	struct dirent ent;
	char buf[2048];
};

/* Go on until ExAll() says the listing is over.  */
static void drain(DIR *d) {
	while (d->more)
		d->more = ExAll(d->lock, d->buf, sizeof(d->buf), ED_TYPE, d->eac);
}

DIR *opendir(const char *path) {
	struct FileInfoBlock *fib;
	LONG type = 0;
	DIR *d;

	if ((d = calloc(1, sizeof(*d))) == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	if ((d->lock = Lock((CONST_STRPTR)path, SHARED_LOCK)) == 0) {
		__seterrno();
		free(d);
		return NULL;
	}
	if ((fib = AllocDosObject(DOS_FIB, NULL)) != NULL) {
		if (Examine(d->lock, fib))
			type = fib->fib_DirEntryType;
		FreeDosObject(DOS_FIB, fib);
	}
	if (type < 0 || (d->eac = AllocDosObject(DOS_EXALLCONTROL, NULL)) == NULL) {
		UnLock(d->lock);
		free(d);
		errno = type < 0 ? ENOTDIR : ENOMEM;
		return NULL;
	}
	d->eac->eac_LastKey = 0;
	d->more = DOSTRUE;
	return d;
}

struct dirent *readdir(DIR *d) {
	struct ExAllData *ead;
	size_t len;

	while (d->left == 0) {
		if (!d->more)
			return NULL;
		d->more = ExAll(d->lock, d->buf, sizeof(d->buf), ED_TYPE, d->eac);
		if (!d->more && IoErr() != ERROR_NO_MORE_ENTRIES) {
			__seterrno();
			return NULL;
		}
		d->left = d->eac->eac_Entries;
		d->next = (struct ExAllData *)d->buf;
	}
	ead = d->next;
	d->next = ead->ed_Next;
	d->left--;

	/* ExAll() gives no disk key with ED_TYPE; 1, never 0, which some
	   programs take for a deleted entry.  */
	d->ent.d_ino = 1;
	d->ent.d_reclen = sizeof(d->ent);
	d->ent.d_type = ead->ed_Type < 0 ? DT_REG
		: ead->ed_Type == ST_SOFTLINK ? DT_LNK : DT_DIR;
	len = strlen((const char *)ead->ed_Name);
	if (len > MAXNAMLEN)
		len = MAXNAMLEN;
	memcpy(d->ent.d_name, ead->ed_Name, len);
	d->ent.d_name[len] = 0;
	d->ent.d_namlen = len;
	return &d->ent;
}

void rewinddir(DIR *d) {
	drain(d);
	d->left = 0;
	d->more = DOSTRUE;
	d->eac->eac_LastKey = 0;
}

int closedir(DIR *d) {
	drain(d);
	FreeDosObject(DOS_EXALLCONTROL, d->eac);
	UnLock(d->lock);
	free(d);
	return 0;
}
