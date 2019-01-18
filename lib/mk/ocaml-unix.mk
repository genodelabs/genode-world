include $(REP_DIR)/lib/mk/ocaml.inc

LIBS += libc

INC_DIR += $(OCAML_SRC_DIR)/byterun
INC_DIR += $(OCAML_SRC_DIR)/byterun/caml

vpath %.c $(OCAML_SRC_DIR)/otherlibs/unix

SRC_C += accept.c access.c addrofstr.c alarm.c bind.c chdir.c chmod.c \
  chown.c chroot.c close.c closedir.c connect.c cst2constr.c cstringv.c \
  dup.c dup2.c envir.c errmsg.c execv.c execve.c execvp.c exit.c \
  fchmod.c fchown.c fcntl.c fork.c ftruncate.c \
  getaddrinfo.c getcwd.c getegid.c geteuid.c getgid.c \
  getgr.c getgroups.c gethost.c gethostname.c getlogin.c \
  getnameinfo.c getpeername.c getpid.c getppid.c getproto.c getpw.c \
  gettimeofday.c getserv.c getsockname.c getuid.c gmtime.c \
  initgroups.c isatty.c itimer.c kill.c link.c listen.c lockf.c lseek.c \
  mkdir.c mkfifo.c mmap.c mmap_ba.c \
  nice.c open.c opendir.c pipe.c putenv.c read.c \
  readdir.c readlink.c rename.c rewinddir.c rmdir.c select.c sendrecv.c \
  setgid.c setgroups.c setsid.c setuid.c shutdown.c signals.c \
  sleep.c socket.c socketaddr.c \
  socketpair.c sockopt.c stat.c strofaddr.c symlink.c termios.c \
  time.c times.c truncate.c umask.c unixsupport.c unlink.c \
  utimes.c wait.c write.c
