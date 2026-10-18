/* chmod for m68k-amigaos: SetProtection().  The owner's protection bits
   mean "not allowed" when set, the group's and others' mean "allowed";
   write permission also carries the delete bit, as the shell's Protect
   does.  */

#include <errno.h>
#include <sys/stat.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <inline/dos.h>

extern void __seterrno(void);

int chmod(const char *path, mode_t mode) {
	LONG prot = (mode & S_IRUSR ? 0 : FIBF_READ)
		| (mode & S_IWUSR ? 0 : FIBF_WRITE | FIBF_DELETE)
		| (mode & S_IXUSR ? 0 : FIBF_EXECUTE)
		| (mode & S_IRGRP ? FIBF_GRP_READ : 0)
		| (mode & S_IWGRP ? FIBF_GRP_WRITE | FIBF_GRP_DELETE : 0)
		| (mode & S_IXGRP ? FIBF_GRP_EXECUTE : 0)
		| (mode & S_IROTH ? FIBF_OTR_READ : 0)
		| (mode & S_IWOTH ? FIBF_OTR_WRITE | FIBF_OTR_DELETE : 0)
		| (mode & S_IXOTH ? FIBF_OTR_EXECUTE : 0);

	if (!SetProtection((CONST_STRPTR)path, prot)) {
		__seterrno();
		return -1;
	}
	return 0;
}
