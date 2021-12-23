PKG_DIR = $(call select_from_ports,libbacktrace)/src/lib/gcc/libbacktrace
LD_OPT_NOSTDLIB := -nostdlib -Wl,-nostdlib
MAKE_TARGET := all

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

installed_tar.tag: installed.tag
	echo ".... remove previous data ....."; \
	rm -f $(BUILD_BASE_DIR)/bin/libbacktrace.a; \
	rm -f $(BUILD_BASE_DIR)/bin/libbacktrace.la; \
	rm -f $(BUILD_BASE_DIR)/debug/libbacktrace.a; \
	rm -f $(BUILD_BASE_DIR)/debug/libbacktrace.la; \
	rm -rf $(BUILD_BASE_DIR)/var/libcache/libbacktrace; \
	if test -e $(BUILD_BASE_DIR)/noux-pkg/libbacktrace/.libs/libbacktrace.a; then \
	echo ".... remove built.tag and installed.tag ....."; \
	rm $(BUILD_BASE_DIR)/noux-pkg/libbacktrace/built.tag; \
	rm $(BUILD_BASE_DIR)/noux-pkg/libbacktrace/installed.tag; \
	echo ".... make symlink to ./var/libcache ....."; \
	mkdir -p $(LIB_CACHE_DIR)/libbacktrace/include; \
	ln -sf $(BUILD_BASE_DIR)/noux-pkg/libbacktrace/.libs/libbacktrace.a $(LIB_CACHE_DIR)/libbacktrace/libbacktrace.a; \
	ln -sf $(BUILD_BASE_DIR)/noux-pkg/libbacktrace/libbacktrace.la $(LIB_CACHE_DIR)/libbacktrace/libbacktrace.la; \
	echo ".... copy h-files for to var/libcache/libbacktrace/include ....."; \
	cp $(BUILD_BASE_DIR)/noux-pkg/libbacktrace/gstdint.h $(LIB_CACHE_DIR)/libbacktrace/include/; \
	find $(PKG_DIR)/ -name '*.h' -exec cp -fLp {} $(LIB_CACHE_DIR)/libbacktrace/include/ \; ; \
	fi
