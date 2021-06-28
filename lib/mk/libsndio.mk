include $(call select_from_repositories,lib/import/import-libsndio.mk)

SNDIO_SRC_DIR    := $(SNDIO_PORT_DIR)/src/lib/sndio
LIBSNDIO_SRC_DIR := $(SNDIO_SRC_DIR)/libsndio

INC_DIR += $(LIBSNDIO_SRC_DIR)
INC_DIR += $(SNDIO_SRC_DIR)/bsd-compat

SRC_C := $(notdir $(wildcard $(LIBSNDIO_SRC_DIR)/*.c))

CC_OPT += -DDEBUG
CC_OPT += -DHAVE_CLOCK_GETTIME -DHAVE_ISSETUGID -DHAVE_STRLCAT \
          -DHAVE_STRLCPY -DHAVE_STRTONUM
CC_OPT += -DUSE_OSS

# available not before FreeBSD 8.3
CC_OPT += -DO_CLOEXEC=0

LIBS := libc

vpath %.c $(LIBSNDIO_SRC_DIR)

SHARED_LIB := yes

CC_CXX_WARN_STRICT =
