include $(REP_DIR)/lib/import/import-libmbim.mk

SHARED_LIB=yes

LIBMBIM_SRC_DIR = $(LIBMBIM_PORT_DIR)/src/lib/libmbim/src
LIBMBIM_GENERATED_SRC_DIR = $(LIBMBIM_GENERATED_PORT_DIR)/src/lib/libmbim_generated

LIBMBIM_COMMON_SRC_C := $(notdir $(wildcard $(LIBMBIM_SRC_DIR)/common/*.c))
LIBMBIM_GLIB_SRC_C := $(notdir $(wildcard $(LIBMBIM_SRC_DIR)/libmbim-glib/*.c))
LIBMBIM_GENERATED_SRC_C := $(notdir $(wildcard $(LIBMBIM_GENERATED_SRC_DIR)/generated/*.c))

CC_DEF += -DLIBMBIM_GLIB_COMPILATION

SRC_C += $(LIBMBIM_COMMON_SRC_C)
SRC_C += $(LIBMBIM_GLIB_SRC_C)
SRC_C += $(LIBMBIM_GENERATED_SRC_C)

CC_DEF += -DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_48 \
          -DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_2_48 \
          -DGLIB_DISABLE_DEPRECATION_WARNINGS

CC_DEF += -DLIBEXEC_PATH=

CC_DEF += -Wno-unused-function

INC_DIR += $(REP_DIR)/src/lib/libmbim
INC_DIR += $(LIBMBIM_SRC_DIR)/libmbim-glib
INC_DIR += $(LIBMBIM_SRC_DIR)/common
INC_DIR += $(LIBMBIM_GENERATED_SRC_DIR)
INC_DIR += $(LIBMBIM_GENERATED_SRC_DIR)/generated


vpath %.c $(LIBMBIM_SRC_DIR)/common
vpath %.c $(LIBMBIM_SRC_DIR)/libmbim-glib
vpath %.c $(LIBMBIM_GENERATED_SRC_DIR)/generated

LIBS += libc libiconv glib

CC_CXX_WARN_STRICT =
