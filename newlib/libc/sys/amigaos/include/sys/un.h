#ifndef __SYS_UN_H
#define __SYS_UN_H

#include <sys/_types.h>

/* The same type as the NDK's <sys/socket.h> gives it */
typedef __sa_family_t sa_family_t;

struct sockaddr_un {
	sa_family_t sun_family;		/* AF_UNIX */
	char        sun_path[108];	/* Pathname */
};

#endif
