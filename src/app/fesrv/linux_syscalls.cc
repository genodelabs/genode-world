///
/// \file       linux_syscalls.cc
/// \author     Menno Valkema <menno.valkema@nlcsl.com>
/// \date       2016-05-17
///
/// \copyright  Copyright (C) 2016 Cyber Security Labs B.V. The Netherlands.
///             All rights reserved.
///
/// \brief      Dummy implementation of nonexistend linux system calls
///

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int openat( int dirfd, const char *pathname, int flags , ... )
{
	return open( pathname, flags );
}


int faccessat( int dirfd, const char *pathname, int mode, int flags )
{
	return access( pathname, mode );
}


int linkat( int olddirfd, const char *oldpath,
            int newdirfd, const char *newpath, int flags )
{
	return link( oldpath, newpath );
}

int unlinkat( int dirfd, const char *pathname, int flags )
{
	return unlink( pathname );
}

int mkdirat( int dirfd, const char *pathname, mode_t mode )
{
	return mkdir( pathname, mode );
}
