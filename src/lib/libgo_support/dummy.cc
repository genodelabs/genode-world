/*
 * \brief  Implementation of some auxilary functions for golang runtime
 * \author Alexander Tormasov
 * \date   2020-11-17
 */

#include <base/debug.h>
#include <errno.h>

extern "C"
{

#include <sys/types.h>
#include <sys/socket.h>

#define NOT_IMPL ({                  \
	PDBG("called not implmemented"); \
})

#include <sys/uio.h>
int sendfile(int fd, int s, off_t offset, size_t nbytes,
             struct sf_hdtr *hdtr, off_t *sbytes, int flags) {
	NOT_IMPL;
	errno = EOPNOTSUPP;
	return -1;
}

#include <unistd.h>
pid_t getpgid(pid_t pid) {
	NOT_IMPL;
	errno = EOPNOTSUPP;
	return -1;
}

int lchown(const char *path, uid_t owner, gid_t group) {
	NOT_IMPL;
	errno = EOPNOTSUPP;
	return -1;
}

#include <sys/mount.h>
int mount(const char *source, const char *target, int f, void *data) {
	NOT_IMPL;
	errno = EOPNOTSUPP;
	return -1;
}

#include <sys/time.h>
int settimeofday(const struct timeval *tv , const struct timezone *tz) {
	NOT_IMPL;
	errno = EOPNOTSUPP;
	return -1;
}

#include <sys/mman.h>
int mlock(const void *addr, size_t len) {
	NOT_IMPL;
	errno = EOPNOTSUPP;
	return -1;
}
int munlock(const void *addr, size_t len) {
	NOT_IMPL;
	errno = EOPNOTSUPP;
	return -1;
}
int mlockall(int flags) {
	NOT_IMPL;
	errno = EOPNOTSUPP;
	return -1;
}
int munlockall(void) {
	NOT_IMPL;
	errno = EOPNOTSUPP;
	return -1;
}

#include <signal.h>
int sigaltstack(const stack_t *ss, stack_t *oss) {
	NOT_IMPL;
	/* cant fail this - runtime_signalstack() will lead bus error */
	return 0;
}

}
