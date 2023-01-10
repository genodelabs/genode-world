MY_BUILD_DIR := $(LIB_CACHE_DIR)/libbacktrace
MY_TARGET := $(MY_BUILD_DIR)/.libs/libbacktrace.a

LIBS += libc

PKG_DIR = $(call select_from_ports,libgo)/src/lib/gcc/libbacktrace
LD_OPT_NOSTDLIB := -nostdlib -Wl,-nostdlib
EXT_OBJECTS += $(shell $(CC) $(CC_MARCH) -print-file-name=libgcc_eh.a)
SHARED_LIBS += ld.lib.so

# to glue gnu_build.mk
CUSTOM_TARGET_DEPS := finished.tag

$(MY_TARGET): built.tag

finished.tag: $(MY_TARGET)
	@$(MSG_INST)$* ; \
	ln -sf $(MY_TARGET) $(MY_BUILD_DIR)/libbacktrace.lib.a; \
	echo ".... strip *gcc.a files from generated static library"; \
	$(AR) d $(MY_TARGET) libgcc_eh.a libgcc.a lt1-libgcc_eh.a lt2-libgcc.a ; \
	echo ".... copy h-files for to $(MY_BUILD_DIR)/include ....."; \
	mkdir -p $(MY_BUILD_DIR)/include; \
	cp $(MY_BUILD_DIR)/gstdint.h $(MY_BUILD_DIR)/include/; \
	find $(PKG_DIR)/ -name '*.h' -exec cp -fLp {} $(MY_BUILD_DIR)/include/ \;
	@touch $@

# add libc include to INC_DIR
include $(call select_from_repositories,lib/mk/libc-common.inc)

CONFIGURE_ARGS +=       --srcdir=$(PKG_DIR)/ \
                        --cache-file=./config.cache \
                        --disable-multilib \
                        --disable-shared \
                        --disable-libada \
                        --with-gnu-as \
                        --with-gnu-ld \
                        --disable-tls \
                        --disable-threads \
                        --disable-hosted-libstdcxx \
                        --enable-multiarch \
                        --disable-sjlj-exceptions \
                        --enable-languages=c,ada,c++,go,lto \
                        --disable-option-checking 

include $(call select_from_repositories,mk/noux.mk)
