#ifndef _INCLUDE__SYS__STAT_H_
#define _INCLUDE__SYS__STAT_H_

#include <../libc/sys/stat.h>
#include <sys/filio.h>

#define stat64 stat
#define fstat64 fstat
#define ftruncate64 ftruncate
#define open64 open

#endif /* _INCLUDE__SYS__STAT_H_ */
