MY_BUILD_DIR := $(LIB_CACHE_DIR)/libatomic
MY_TARGET := $(MY_BUILD_DIR)/.libs/libatomic.a

LIBS += libc

PKG_DIR = $(call select_from_ports,libgo)/src/lib/gcc/libatomic
LD_OPT_NOSTDLIB := -nostdlib -Wl,-nostdlib

# to glue gnu_build.mk
CUSTOM_TARGET_DEPS := finished.tag

$(MY_TARGET): built.tag

finished.tag: $(MY_TARGET)
	@$(MSG_INST)$* ; \
	echo ".... strip *gcc.a files from generated static library"; \
	$(AR) d $(MY_TARGET) libgcc.a lt1-libgcc.a ; \
	ln -sf $(MY_TARGET) $(MY_BUILD_DIR)/libatomic.lib.a; \
	echo ".... copy h-files for to $(MY_BUILD_DIR)/include ....."; \
	mkdir -p $(MY_BUILD_DIR)/include; \
	find $(PKG_DIR)/ -name '*.h' -exec cp -fLp {} $(MY_BUILD_DIR)/include/ \;
	@touch $@

# add libc include to INC_DIR
include $(call select_from_repositories,lib/mk/libc-common.inc)

CONFIGURE_ARGS +=	--srcdir=$(PKG_DIR)/ \
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

#
# Make the configure linking test succeed
#
Makefile: dummy_libs

.SECONDARY: dummy_libs
dummy_libs: libpthread.a

libpthread.a:
	$(VERBOSE)$(AR) -rc $@
