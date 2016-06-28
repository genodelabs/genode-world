///
/// \file       linux_syscalls.cc
/// \author     Menno Valkema <menno.valkema@nlcsl.com>
/// \date       2016-05-17
///
/// \brief      Dummy implementation of nonexistend linux system calls
///

/// 
/// Copyright (c) 2016, Cyber Security Labs B.V.
/// All rights reserved.
/// 
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
/// 1. Redistributions of source code must retain the above copyright
///    notice, this list of conditions and the following disclaimer.
/// 2. Redistributions in binary form must reproduce the above copyright
///    notice, this list of conditions and the following disclaimer in the
///    documentation and/or other materials provided with the distribution.
/// 3. All advertising materials mentioning features or use of this software
///    must display the following acknowledgement:
///    This product includes software developed by the Cyber Security Labs B.V..
/// 4. Neither the name of Cyber Security Labs B.V. nor the
///    names of its contributors may be used to endorse or promote products
///    derived from this software without specific prior written permission.
/// 
/// THIS SOFTWARE IS PROVIDED BY CYBER SECURITY LABS B.V. ''AS IS'' AND ANY
/// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
/// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL CYBER SECURITY LABS B.V. BE LIABLE FOR ANY
/// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
/// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
/// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
/// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
/// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
