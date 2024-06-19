SHARED_LIB := yes

include $(REP_DIR)/lib/import/import-glib.mk

GLIB_SRC_DIR = $(GLIB_PORT_DIR)/src/lib/glib/glib

LIBS += libc libiconv zlib ffi

CC_DEF += \
	-DGLIB_MAJOR_VERSION=2 -DGLIB_MINOR_VERSION=54 -DGLIB_MICRO_VERSION=3 \
	-DGLIB_BINARY_AGE=5302 -DGLIB_INTERFACE_AGE=0 \
	-DGLIB_COMPILATION \
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
	-UEBCDIC

CC_DEF += -DGIO_COMPILATION=1
CC_DEF += -DGOBJECT_COMPILATION=1

CC_DEF += -DPACKAGE_VERSION=\"2.54.3\"
CC_DEF += -DGLIB_LOCALE_DIR=\"\"
CC_DEF += -DGIO_MODULE_DIR=\"/lib/gio/modules\"


CC_WARN += -Wno-unused-function -Wno-deprecated-declarations

INC_DIR += \
	$(REP_DIR)/src/lib/glib \
	$(GLIB_SRC_DIR) \
	$(REP_DIR)/include/glib \
	$(GLIB_PORT_DIR)/src/lib/glib \
	$(GLIB_PORT_DIR)/include/glib \
	$(GLIB_PORT_DIR)/include/glib/gio

DEPRECATED_SRC_C := $(notdir $(wildcard $(GLIB_SRC_DIR)/deprecated/*.c))

PCRE_SRC := $(notdir $(wildcard $(GLIB_SRC_DIR)/pcre/*.c))

LIBCHARSET_SRC := $(notdir $(wildcard $(GLIB_SRC_DIR)/libcharset/*.c))

GLIB_SRC := $(notdir $(wildcard $(GLIB_SRC_DIR)/*.c))

GLIB_GIO_SRC := \
	$(notdir $(wildcard $(GLIB_SRC_DIR)/../gio/*.c)) \
	$(notdir $(wildcard $(GLIB_SRC_DIR)/../gio/xdgmime/*.c))

CC_DEF += -DXDG_PREFIX=_gio_xdg

GLIB_GOBJECT_SRC := $(notdir $(wildcard $(GLIB_SRC_DIR)/../gobject/*.c))

GLIB_GMODULE_SRC := $(notdir $(wildcard $(GLIB_SRC_DIR)/../gmodule/*.c))

GLIB_GMODULE_FILTER := \
	gmodule-ar.c gmodule-dl.c gmodule-dyld.c gmodule-win32.c

INC_DIR += $(REP_DIR)/src/lib/glib/gmodule

GLIB_FILTER := \
	guuid.c win_iconv.c \
	gtester.c \
	$(notdir $(wildcard $(GLIB_SRC_DIR)/*win32*.c)) \
	$(notdir $(wildcard $(GLIB_SRC_DIR)/*win64*.c))

GLIB_GIO_FILTER := \
	gresource.c gsettingsschema.c \
	gcocoanotificationbackend.c gcontenttype-win32.c \
	gnetworkmonitornetlink.c gnextstepsettingsbackend.c \
	gosxappinfo.c gosxcontenttype.c gregistrysettingsbackend.c \
	gwin32outputstream.c gwin32mount.c gwin32inputstream.c gwin32appinfo.c \
	gwin32volumemonitor.c gwin32registrykey.c \
	gapplication-tool.c \
	gio-tool-cat.c gio-tool-info.c gio-tool-mime.c gio-tool-monitor.c \
	gio-tool-move.c gio-tool-remove.c gio-tool-save.c gio-tool-trash.c \
	gio-tool.c gresource-tool.c gdbus-tool.c gio-tool-copy.c gio-tool-list.c \
	gio-tool-mkdir.c gio-tool-mount.c gio-tool-open.c gio-tool-rename.c \
	gio-tool-set.c gio-tool-tree.c gsettings-tool.c \
	gio-querymodules.c glib-compile-resources.c glib-compile-schemas.c

GLIB_GOBJECT_FILTER := \
	gobject-query.c glib-genmarshal.c

SRC_C = \
	$(DEPRECATED_SRC_C) \
	$(PCRE_SRC) \
	$(LIBCHARSET_SRC) \
	$(filter-out $(GLIB_FILTER),$(GLIB_SRC)) \
	$(filter-out $(GLIB_GIO_FILTER),$(GLIB_GIO_SRC)) \
	$(filter-out $(GLIB_GOBJECT_FILTER),$(GLIB_GOBJECT_SRC)) \
	$(filter-out $(GLIB_GMODULE_FILTER),$(GLIB_GMODULE_SRC))

vpath %.c $(GLIB_SRC_DIR)
vpath %.c $(GLIB_SRC_DIR)/../gio
vpath %.c $(GLIB_SRC_DIR)/../gio/xdgmime
vpath %.c $(GLIB_SRC_DIR)/../gobject
vpath %.c $(GLIB_SRC_DIR)/../gmodule
vpath %.c $(GLIB_SRC_DIR)/deprecated
vpath %.c $(GLIB_SRC_DIR)/pcre
vpath %.c $(GLIB_SRC_DIR)/libcharset

vpath %.c $(GLIB_SRC_DIR)/../gio
CC_CXX_WARN_STRICT =
