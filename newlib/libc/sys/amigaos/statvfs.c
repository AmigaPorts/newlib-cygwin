#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <sys/statvfs.h>

static unsigned long
positive_or_zero(LONG value)
{
	return value > 0 ? (unsigned long)value : 0;
}

static void
set_errno_from_ioerr(void)
{
	switch (IoErr())
	{
	case ERROR_NO_FREE_STORE:
		errno = ENOMEM;
		break;
	case ERROR_DIR_NOT_FOUND:
	case ERROR_OBJECT_NOT_FOUND:
		errno = ENOENT;
		break;
	case ERROR_DEVICE_NOT_MOUNTED:
	case ERROR_NO_DISK:
		errno = ENXIO;
		break;
	case ERROR_DISK_WRITE_PROTECTED:
		errno = EROFS;
		break;
	default:
		errno = EIO;
		break;
	}
}

static void
fill_statvfs(struct statvfs *buf, const struct InfoData *info)
{
	unsigned long blocks = positive_or_zero(info->id_NumBlocks);
	unsigned long used = positive_or_zero(info->id_NumBlocksUsed);
	unsigned long block_size = positive_or_zero(info->id_BytesPerBlock);
	unsigned long free_blocks;

	if (used > blocks)
		used = blocks;
	if (block_size == 0)
		block_size = 512;

	free_blocks = blocks - used;

	memset(buf, 0, sizeof(*buf));
	buf->f_bsize = block_size;
	buf->f_frsize = block_size;
	buf->f_blocks = blocks;
	buf->f_bfree = free_blocks;
	buf->f_bavail = free_blocks;
	buf->f_fsid = (unsigned long)info->id_DiskType;
	if (info->id_DiskState == ID_WRITE_PROTECTED)
		buf->f_flag |= ST_RDONLY;
}

int
statvfs(const char *path, struct statvfs *buf)
{
	struct InfoData info;
	BPTR lock;
	int ret = -1;

	if (path == NULL || buf == NULL)
	{
		errno = EFAULT;
		return -1;
	}

	lock = Lock((CONST_STRPTR)path, SHARED_LOCK);
	if (lock == 0)
	{
		set_errno_from_ioerr();
		return -1;
	}

	if (Info(lock, &info))
	{
		fill_statvfs(buf, &info);
		ret = 0;
	}
	else
		set_errno_from_ioerr();

	UnLock(lock);
	return ret;
}

/* The same limits hold for every file: -1 with errno untouched means no
   limit, as POSIX has it.  libstdc++'s std::filesystem asks for
   _PC_PATH_MAX.  */
static long
path_limit(int name)
{
	switch (name)
	{
	case _PC_NAME_MAX:
		return MAXNAMLEN;
	case _PC_PATH_MAX:
		return PATH_MAX;
	case _PC_LINK_MAX:
	case _PC_MAX_CANON:
	case _PC_MAX_INPUT:
	case _PC_PIPE_BUF:
	case _PC_CHOWN_RESTRICTED:
	case _PC_NO_TRUNC:
	case _PC_VDISABLE:
	case _PC_ASYNC_IO:
	case _PC_PRIO_IO:
	case _PC_SYNC_IO:
	case _PC_FILESIZEBITS:
	case _PC_2_SYMLINKS:
	case _PC_SYMLINK_MAX:
	case _PC_ALLOC_SIZE_MIN:
	case _PC_REC_INCR_XFER_SIZE:
	case _PC_REC_MAX_XFER_SIZE:
	case _PC_REC_MIN_XFER_SIZE:
	case _PC_REC_XFER_ALIGN:
	case _PC_TIMESTAMP_RESOLUTION:
		return -1;
	default:
		errno = EINVAL;
		return -1;
	}
}

long
pathconf(const char *path, int name)
{
	(void)path;
	return path_limit(name);
}

long
fpathconf(int fd, int name)
{
	(void)fd;
	return path_limit(name);
}
