LIBMBIM_DIR := $(call select_from_ports,libmbim)/src/lib/libmbim

TARGET := mbimcli

SRC_C := $(notdir $(wildcard $(LIBMBIM_DIR)/src/mbimcli/*.c))

CC_OPT += -Wno-unused-function

CC_DEF += -DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_48 \
          -DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_2_48 \
          -DGLIB_DISABLE_DEPRECATION_WARNINGS

INC_DIR := $(LIBMBIM_DIR)/src/mbimcli
INC_DIR += $(LIBMBIM_DIR)/src/common
INC_DIR += $(REP_DIR)/src/lib/libmbim

LIBS = libc glib libmbim posix

vpath %.c $(LIBMBIM_DIR)/src/mbimcli

CC_CXX_WARN_STRICT =
