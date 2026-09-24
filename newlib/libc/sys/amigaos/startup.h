#ifndef _AMIGAOS_STARTUP_H
#define _AMIGAOS_STARTUP_H

#include <workbench/startup.h>

extern int __argc;
extern char **__argv;
extern char *__commandline;
extern unsigned long __commandlen;
extern struct WBStartup *_WBenchMsg;
void __nocommandline(void);

#endif
