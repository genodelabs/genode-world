UTIL_LINUX_DIR := $(call select_from_ports,util-linux)/src/lib/util-linux
UTIL_LINUX_LIB_DIR := $(UTIL_LINUX_DIR)/lib
LIBUUID_DIR := $(UTIL_LINUX_DIR)/libuuid/src

LIBS += libc

# util-linux/libuuid files
SRC_C := clear.c
SRC_C += compare.c
SRC_C += copy.c
SRC_C += gen_uuid.c
SRC_C += isnull.c
SRC_C += pack.c
SRC_C += parse.c
SRC_C += predefined.c
SRC_C += test_uuid.c
SRC_C += unpack.c
SRC_C += unparse.c
SRC_C += uuid_time.c

# util-linux/lib files
SRC_C += sha1.c
SRC_C += md5.c
SRC_C += randutils.c

CC_C_OPT += -Wno-implicit-function-declaration

INC_DIR += $(LIBUUID_DIR)
INC_DIR += $(UTIL_LINUX_DIR)/include
INC_DIR += $(UTIL_LINUX_LIB_DIR)

vpath %.c $(LIBUUID_DIR)
vpath %.c $(UTIL_LINUX_LIB_DIR)

SHARED_LIB = 1
