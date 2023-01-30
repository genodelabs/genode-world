include $(REP_DIR)/lib/import/import-libbacktrace.mk

LIBS += libc

PKG_DIR = $(call select_from_ports,libgo)/src/lib/gcc/libbacktrace
EXT_OBJECTS += $(shell $(CC) $(CC_MARCH) -print-file-name=libgcc_eh.a)

# to glue gnu_build.mk
CUSTOM_TARGET_DEPS += built.tag

CONFIGURE_ARGS += --disable-shared

include $(call select_from_repositories,mk/gnu_build.mk)
