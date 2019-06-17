REQUIRES := x86
TARGET := umurmur

UMURMUR_DIR := $(call select_from_ports,umurmur)/src/app/umurmur/src

SRC_C := Mumble.pb-c.c \
         ban.c \
         channel.c \
         client.c \
         conf.c \
         crypt.c \
         log.c \
         main.c \
         memory.c \
         messagehandler.c \
         messages.c \
         pds.c \
         server.c \
         ssli_openssl.c \
         timer.c \
         util.c \
         voicetarget.c

SRC_C += dummy.c

INC_DIR += $(UMURMUR_DIR) $(PRG_DIR)

LIBS += libc libcrypto libssl protobuf-c libconfig posix

vpath %.c $(UMURMUR_DIR)
vpath %.c $(PRG_DIR)

CC_CXX_WARN_STRICT =
