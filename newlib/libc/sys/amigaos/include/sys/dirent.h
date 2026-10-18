/* <dirent.h> for m68k-amigaos: directories are read with ExAll() in
   libc/sys/amigaos/dirent.c.  */

#ifndef _SYS_DIRENT_H
#define _SYS_DIRENT_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* An AmigaDOS name is at most 107 characters (FileInfoBlock).  */
#define MAXNAMLEN 107

struct dirent {
	ino_t d_ino;
	unsigned short d_reclen;
	unsigned char d_type;
	unsigned char d_namlen;
	char d_name[MAXNAMLEN + 1];
};

#define _DIRENT_HAVE_D_TYPE
#define _DIRENT_HAVE_D_NAMLEN
#define _DIRENT_HAVE_D_RECLEN

/* d_type: directory, regular file or soft link; the rest for programs
   that switch over all of them.  */
#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12
#define DT_WHT 14

#define IFTODT(mode) (((mode) & 0170000) >> 12)
#define DTTOIF(dirtype) ((dirtype) << 12)

typedef struct __dirstream DIR;

#ifdef __cplusplus
}
#endif

#endif /* _SYS_DIRENT_H */
