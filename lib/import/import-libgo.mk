LIBGO_PORT_DIR := $(call select_from_ports,libgo)

INC_DIR += $(BUILD_BASE_DIR)/lib/libgo
INC_DIR += $(LIBBACKTRACE_PORT_DIR)/include

# place for build go packages to be given for any compilation via -I
LIBGO_PKG_BUILD := $(BUILD_BASE_DIR)/lib/libgo

# add includes from build for any .go compilation
CUSTOM_GO_FLAGS = -I$(LIBGO_PKG_BUILD)

# additional static libraries to link gccgo executables
LD_LIBGCC = \
${BUILD_BASE_DIR}/lib/libgo/libgobegin.a \
${BUILD_BASE_DIR}/lib/libgo/libgolibbegin.a \
${BUILD_BASE_DIR}/lib/libgo/.libs/libgo.a \
$(shell $(CC) $(CC_MARCH) -print-libgcc-file-name) \
$(shell $(CC) $(CC_MARCH) -print-file-name=libgcc_eh.a)
