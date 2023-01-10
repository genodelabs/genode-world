MY_BUILD_DIR := $(LIB_CACHE_DIR)/libgo
MY_TARGET := $(MY_BUILD_DIR)/.libs/libgo.a

PKG_DIR = $(call select_from_ports,libgo)/src/lib/gcc/libgo
LD_OPT_NOSTDLIB := -nostdlib -Wl,-nostdlib

LIBS += libgo_support 

INC_DIR += $(LIB_CACHE_DIR)/libbacktrace/include

# to glue gnu_build.mk
CUSTOM_TARGET_DEPS := check.tag finished.tag

$(MY_TARGET): built.tag

# check that libatomic libbacktrace libffi already build
check.tag:		$(LIB_CACHE_DIR)/libatomic/finished.tag \
				$(LIB_CACHE_DIR)/libbacktrace/finished.tag \
				$(LIB_CACHE_DIR)/libffi/finished.tag

$(LIB_CACHE_DIR)/libatomic/finished.tag \
				$(LIB_CACHE_DIR)/libbacktrace/finished.tag \
				$(LIB_CACHE_DIR)/libffi/finished.tag :
	@$(warning *** $(lastword $(subst /, ,$(@D))) should be build before libgo, for each like)
	@$(error make -C build/<ARCH> lib/$(lastword $(subst /, ,$(@D))))

finished.tag:: $(MY_TARGET)
	@$(MSG_INST)$*
	echo ".... strip *gcc.a files from generated static library"
	@$(AR) d $(MY_TARGET) base.lib.a libatomic.lib.a libbacktrace.lib.a \
		libffi.lib.a libgo_support.lib.a libgcc.a lt1-base.lib.a \
		lt2-libatomic.lib.a lt3-libbacktrace.lib.a lt4-libffi.lib.a \
		lt5-libgo_support.lib.a lt6-libgcc.a lt2-libgo_support.lib.a lt3-libgcc.a
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
