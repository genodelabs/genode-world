MY_BUILD_DIR := $(LIB_CACHE_DIR)/libgo
MY_TARGET := $(MY_BUILD_DIR)/.libs/libgo.a

PKG_DIR = $(call select_from_ports,libgo)/src/lib/gcc/libgo
LD_OPT_NOSTDLIB := -nostdlib -Wl,-nostdlib

LIBS += libgo_support libatomic libbacktrace libffi

INC_DIR += $(LIB_CACHE_DIR)/libbacktrace/include

# to glue gnu_build.mk
CUSTOM_TARGET_DEPS := finished.tag

$(MY_TARGET): built.tag

finished.tag:: $(MY_TARGET)
	@$(MSG_INST)$*
	@ln -sf $(MY_TARGET) $(MY_BUILD_DIR)/libgo.lib.a
	@ln -sf $(MY_BUILD_DIR)/libgobegin.a $(MY_BUILD_DIR)/libgobegin.lib.a
	@ln -sf $(MY_BUILD_DIR)/libgolibbegin.a $(MY_BUILD_DIR)/libgolibbegin.lib.a
	@touch $@

#CC_OLEVEL = -O0

CONFIGURE_ARGS +=	--srcdir=$(PKG_DIR)/ \
			--cache-file=./config.cache \
			--disable-multilib \
			--disable-shared \
			--enable-werror=no \
			--with-gnu-ld

include $(call select_from_repositories,mk/noux.mk)
