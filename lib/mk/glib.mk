SHARED_LIB =1

GLIB_PORT_DIR = $(call select_from_ports,glib)

GLIB_SRC_DIR = $(GLIB_PORT_DIR)/src/lib/glib/glib

LIBS += libc libiconv

CC_DEF += \
	-DGLIB_MAJOR_VERSION=2 -DGLIB_MINOR_VERSION=52 -DGLIB_MICRO_VERSION=2 \
	-DGLIB_BINARY_AGE=5302 -DGLIB_INTERFACE_AGE=0 \
	-DGLIB_COMPILATION \
	-DG_OS_UNIX \
	-DSUPPORT_UCP \
	-DSUPPORT_UTF \
	-DSUPPORT_UTF8 \
	-DNEWLINE=-1 \
	-DMATCH_LIMIT=10000000 \
	-DMATCH_LIMIT_RECURSION=8192 \
	-DMAX_NAME_SIZE=32 \
	-DMAX_NAME_COUNT=10000 \
	-DMAX_DUPLENGTH=30000 \
	-DLINK_SIZE=2 \
	-DPOSIX_MALLOC_THRESHOLD=10 \
	-DPCRE_STATIC \
	-UBSR_ANYCRLF \
	-UEBCDIC \

CC_WARN += -Wno-unused-function -Wno-deprecated-declarations

INC_DIR += \
	$(REP_DIR)/src/lib/glib \
	$(GLIB_SRC_DIR) \
	$(REP_DIR)/include/glib \
	$(GLIB_PORT_DIR)/src/lib/glib \
	$(GLIB_PORT_DIR)/include/glib \

DEPRECATED_SRC_C := $(notdir $(wildcard $(GLIB_SRC_DIR)/deprecated/*.c))

PCRE_SRC := $(notdir $(wildcard $(GLIB_SRC_DIR)/pcre/*.c))

LIBCHARSET_SRC := $(notdir $(wildcard $(GLIB_SRC_DIR)/libcharset/*.c))

GLIB_SRC := $(notdir $(wildcard $(GLIB_SRC_DIR)/*.c))

GLIB_FILTER := \
	goption.c guuid.c win_iconv.c \
	$(notdir $(wildcard $(GLIB_SRC_DIR)/*win32*.c)) \
	$(notdir $(wildcard $(GLIB_SRC_DIR)/*win64*.c))

SRC_C = \
	$(DEPRECATED_SRC_C) \
	$(PCRE_SRC) \
	$(LIBCHARSET_SRC) \
	$(filter-out $(GLIB_FILTER),$(GLIB_SRC)) \

vpath %.c $(GLIB_SRC_DIR)
vpath %.c $(GLIB_SRC_DIR)/deprecated
vpath %.c $(GLIB_SRC_DIR)/pcre
vpath %.c $(GLIB_SRC_DIR)/libcharset
