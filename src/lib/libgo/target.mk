PKG_DIR = $(call select_from_ports,libgo)/src/lib/gcc/libgo
LD_OPT_NOSTDLIB := -nostdlib -Wl,-nostdlib
MAKE_TARGET := all
LIBS += libatomic libbacktrace libffi libgo_support

#CC_OLEVEL = -O0

CONFIGURE_ARGS +=	--srcdir=$(PKG_DIR)/ \
			--cache-file=./config.cache \
			--disable-multilib \
			--disable-shared \
			--enable-werror=no \
			--with-gnu-ld

include $(call select_from_repositories,mk/noux.mk)

installed_tar.tag: installed.tag
	echo ".... remove previous data ....."; \
	rm -f $(BUILD_BASE_DIR)/bin/libgo.a; \
	rm -f $(BUILD_BASE_DIR)/bin/libgo.la; \
	rm -f $(BUILD_BASE_DIR)/debug/libgo.a; \
	rm -f $(BUILD_BASE_DIR)/debug/libgo.la; \
	rm -f $(BUILD_BASE_DIR)/bin/libgobegin.a; \
	rm -f $(BUILD_BASE_DIR)/bin/libgobegin.la; \
	rm -f $(BUILD_BASE_DIR)/debug/libgobegin.a; \
	rm -f $(BUILD_BASE_DIR)/debug/libgobegin.la; \
	rm -f $(BUILD_BASE_DIR)/bin/libgolibbegin.a; \
	rm -f $(BUILD_BASE_DIR)/bin/libgolibbegin.la; \
	rm -f $(BUILD_BASE_DIR)/debug/libgolibbegin.a; \
	rm -f $(BUILD_BASE_DIR)/debug/libgolibbegin.la; \
	rm -rf $(BUILD_BASE_DIR)/var/libcache/libgo; \
	if test -e $(BUILD_BASE_DIR)/lib/libgo/.libs/libgo.a; then \
		echo ".... remove built.tag and installed.tag ....."; \
		rm $(BUILD_BASE_DIR)/lib/libgo/built.tag; \
		rm $(BUILD_BASE_DIR)/lib/libgo/installed.tag; \
		echo ".... remove install dir ....."; \
		rm -rf $(BUILD_BASE_DIR)/lib/libgo/install; \
		echo ".... link libgo.a to ./var/libcache ....."; \
		mkdir -p $(LIB_CACHE_DIR)/libgo/include; \
		ln -sf $(BUILD_BASE_DIR)/lib/libgo/.libs/libgo.a $(LIB_CACHE_DIR)/libgo/libgo.a; \
		ln -sf $(BUILD_BASE_DIR)/lib/libgo/libgo.la $(LIB_CACHE_DIR)/libgo/libgo.la; \
		ln -sf $(BUILD_BASE_DIR)/lib/libgo/libgobegin.a $(LIB_CACHE_DIR)/libgo/libgobegin.a; \
		ln -sf $(BUILD_BASE_DIR)/lib/libgo/libgolibbegin.a $(LIB_CACHE_DIR)/libgo/libgolibbegin.a; \
	fi
