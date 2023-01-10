LIBGO_PORT_DIR := $(call select_from_ports,libgo)
LIBBACKTRACE_PORT_DIR := $(call select_from_ports,libgo)

# place for build go packages to be given for any compilation via -I
LIBGO_PKG_BUILD := $(LIB_CACHE_DIR)/libgo

INC_DIR += $(LIBGO_PKG_BUILD)
INC_DIR += $(LIBBACKTRACE_PORT_DIR)/include

# add includes from build for any .go compilation
CUSTOM_GO_FLAGS = -I$(LIBGO_PKG_BUILD)

# additional static libraries to link gccgo executables
LD_LIBGCC = \
$(LIBGO_PKG_BUILD)/libgobegin.lib.a \
$(LIBGO_PKG_BUILD)/libgolibbegin.lib.a \
$(shell $(CC) $(CC_MARCH) -print-libgcc-file-name) \
$(shell $(CC) $(CC_MARCH) -print-file-name=libgcc_eh.a)
