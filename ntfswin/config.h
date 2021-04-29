
#define HAVE_STDIO_H 1

#define HAVE_STDLIB_H 1

#define HAVE_STRING_H 1

#define HAVE_ERRNO_H 1

#define HAVE_TIME_H 1

#define HAVE_SYS_STAT_H 1

#define HAVE_GETTIMEOFDAY 1

#define HAVE_DAEMON	1

#define HAVE_SYS_IOCTL_H 1

#define HAVE_FCNTL_H 1

#define HAVE_SYS_TYPES_H 1

#define HAVE_SYS_PARAM_H 1

#define HAVE_UNISTD_H 1

#define HAVE_CTYPE_H 1

#define HAVE_LIMITS_H 1

#define HAVE_FFS 1

 //#define DEBUG 1  //ysy 0217

// #define __CYGWIN32__ 1

//#include <stdarg.h>
#include <machine/endian.h>

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_FFS
#include <strings.h>
#endif

#define S_ISVTX 01000

#define __inline__ __inline
#define inline __inline

#define ISPATHSEPARATOR(x) ((x == '/') || (x == '\\'))

