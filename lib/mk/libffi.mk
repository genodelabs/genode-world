MY_BUILD_DIR := $(LIB_CACHE_DIR)/libffi
MY_TARGET := $(MY_BUILD_DIR)/.libs/libffi.a

LIBS += libc

PKG_DIR = $(call select_from_ports,libgo)/src/lib/gcc/libffi
LD_OPT_NOSTDLIB := -nostdlib -Wl,-nostdlib

# to glue gnu_build.mk
CUSTOM_TARGET_DEPS := finished.tag

$(MY_TARGET): built.tag

finished.tag: $(MY_TARGET)
	@$(MSG_INST)$* ; \
	echo ".... strip *gcc.a files from generated static library"; \
	$(AR) d $(MY_TARGET) libgcc.a lt1-libgcc.a ; \
	ln -sf $(MY_TARGET) $(MY_BUILD_DIR)/libffi.lib.a
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
