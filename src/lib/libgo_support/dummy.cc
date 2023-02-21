/*
 * \brief  Implementation of some auxilary functions for golang runtime
 * \author Alexander Tormasov
 * \date   2020-11-17
 */

/*
 * Copyright (C) 2022 Alexander Tormasov
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * ``Software''), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <base/log.h>
#include <errno.h>

extern "C"
{

#include <sys/types.h>
#include <sys/socket.h>

#define NOT_IMPL ({                  \
	Genode::warning(__PRETTY_FUNCTION__, " called, not implemented"); \
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
