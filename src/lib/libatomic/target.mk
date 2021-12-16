PKG_DIR = $(call select_from_ports,libatomic)/src/lib/gcc/libatomic
LD_OPT_NOSTDLIB := -nostdlib -Wl,-nostdlib
MAKE_TARGET := all

# add libc include to INC_DIR
include $(call select_from_repositories,lib/mk/libc-common.inc)

#
# do not use configure default args
NO_DEFAULT_CONFIGURE_ARGS=

CONFIGURE_ARGS +=	--srcdir=$(PKG_DIR)/ \
			--cache-file=./config.cache \
			--disable-multilib \
			--disable-shared \
			--with-multilib-list=m64 \
			--disable-libada \
			--with-gnu-as \
			--with-gnu-ld \
			--disable-tls \
			--disable-threads \
			--disable-hosted-libstdcxx \
			--enable-multiarch \
			--disable-sjlj-exceptions \
			--enable-languages=c,ada,c++,go,lto \
			--disable-option-checking \
			--build=x86_64-pc-elf\
			--host=genode-x86

include $(call select_from_repositories,mk/noux.mk)

installed_tar.tag: installed.tag
	echo ".... remove previous data ....."; \
	rm -f $(BUILD_BASE_DIR)/bin/libatomic.a; \
	rm -f $(BUILD_BASE_DIR)/bin/libatomic.la; \
	rm -f $(BUILD_BASE_DIR)/debug/libatomic.a; \
	rm -f $(BUILD_BASE_DIR)/debug/libatomic.la; \
	rm -rf $(BUILD_BASE_DIR)/var/libcache/libatomic; \
	if test -e $(BUILD_BASE_DIR)/noux-pkg/libatomic/.libs/libatomic.a; then \
	echo ".... remove built.tag and installed.tag ....."; \
	rm $(BUILD_BASE_DIR)/noux-pkg/libatomic/built.tag; \
	rm $(BUILD_BASE_DIR)/noux-pkg/libatomic/installed.tag; \
	echo ".... remove install dir ....."; \
	rm -rf $(BUILD_BASE_DIR)/noux-pkg/libatomic/install; \
	echo ".... make symlink to ./var/libcache ....."; \
	mkdir -p $(BUILD_BASE_DIR)/var/libcache/libatomic/include; \
	ln -sf $(BUILD_BASE_DIR)/noux-pkg/libatomic/.libs/libatomic.a $(BUILD_BASE_DIR)/var/libcache/libatomic/libatomic.a; \
	ln -sf $(BUILD_BASE_DIR)/noux-pkg/libatomic/libatomic.la $(BUILD_BASE_DIR)/var/libcache/libatomic/libatomic.la; \
	echo ".... copy h-files for to var/libcache/libatomic/include ....."; \
	find $(PKG_DIR)/ -name '*.h' -exec cp -fLp {} $(BUILD_BASE_DIR)/var/libcache/libatomic/include/ \; ; \
	fi

#
 # Make the configure linking test succeed
 #
 Makefile: dummy_libs

 LDFLAGS += -L$(PWD)

 dummy_libs: libpthread.a

libpthread.a:
	$(VERBOSE)$(AR) -rc $@
