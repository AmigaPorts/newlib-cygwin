/* stat and lstat for m68k-amigaos: Lock() and Examine().  The syscalls/
   connectors of newlib are not built for this target, so both the _name
   and the public name are defined here.  */

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <inline/dos.h>
#include <inline/exec.h>

extern void __seterrno(void);

/* Fill st from a FileInfoBlock.  The owner's protection bits mean "not
   allowed" when set, the group's and others' mean "allowed".  A DateStamp
   counts from 1978-01-01 in local time; it is converted as if it were UTC,
   and utimes() stores it the same way, so the two agree.  */
void __fibstat(struct stat *st, const struct FileInfoBlock *fib) {
	LONG prot = fib->fib_Protection;

	memset(st, 0, sizeof(*st));
	st->st_ino = fib->fib_DiskKey;
	st->st_mode = (fib->fib_DirEntryType < 0 ? S_IFREG :
		       fib->fib_DirEntryType == ST_SOFTLINK ? S_IFLNK : S_IFDIR)
		| (prot & FIBF_READ ? 0 : S_IRUSR)
		| (prot & FIBF_WRITE ? 0 : S_IWUSR)
		| (prot & FIBF_EXECUTE ? 0 : S_IXUSR)
		| (prot & FIBF_GRP_READ ? S_IRGRP : 0)
		| (prot & FIBF_GRP_WRITE ? S_IWGRP : 0)
		| (prot & FIBF_GRP_EXECUTE ? S_IXGRP : 0)
		| (prot & FIBF_OTR_READ ? S_IROTH : 0)
		| (prot & FIBF_OTR_WRITE ? S_IWOTH : 0)
		| (prot & FIBF_OTR_EXECUTE ? S_IXOTH : 0);
	st->st_nlink = 1;
	st->st_uid = fib->fib_OwnerUID;
	st->st_gid = fib->fib_OwnerGID;
	st->st_size = fib->fib_Size;
	st->st_atime = st->st_mtime = st->st_ctime =
		((fib->fib_Date.ds_Days + 2922) * 1440 + fib->fib_Date.ds_Minute) * 60
		+ fib->fib_Date.ds_Tick / TICKS_PER_SECOND;
	st->st_blksize = 512;
	st->st_blocks = fib->fib_NumBlocks;
}

int _stat(const char *path, struct stat *st) {
	struct Process *proc = (struct Process *)FindTask(NULL);
	APTR oldwin = proc->pr_WindowPtr;
	struct FileInfoBlock *fib;
	BPTR lock;
	int ret = -1;

	/* No "please insert volume" requester for a path on a missing one.  */
	proc->pr_WindowPtr = (APTR)-1;
	if ((lock = Lock((CONST_STRPTR)path, SHARED_LOCK)) != 0) {
		if ((fib = AllocDosObject(DOS_FIB, NULL)) != NULL) {
			if (Examine(lock, fib)) {
				__fibstat(st, fib);
				ret = 0;
			} else
				__seterrno();
			FreeDosObject(DOS_FIB, fib);
		} else
			__seterrno();
		UnLock(lock);
	} else
		__seterrno();
	proc->pr_WindowPtr = oldwin;
	return ret;
}

int stat(const char *path, struct stat *st) {
	return _stat(path, st);
}

/* Lock() follows soft links, so this reports the target like stat().  */
int lstat(const char *path, struct stat *st) {
	return _stat(path, st);
}
