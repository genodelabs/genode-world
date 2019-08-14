SHARED_LIB = yes

FDT_PORT := $(call select_from_ports,fdt)/src/lib/fdt/libfdt

INC_DIR  += $(REP_DIR)/src/lib/fdt $(FDT_PORT)

CC_C_OPT += -include $(REP_DIR)/src/lib/fdt/libfdt_env.h
LD_OPT   += --version-script=$(REP_DIR)/src/lib/fdt/symbol.map

SRC_C = fdt.c fdt_ro.c fdt_wip.c fdt_sw.c fdt_rw.c fdt_strerror.c \
        fdt_empty_tree.c fdt_addresses.c fdt_overlay.c

SRC_CC = libfdt_env.cc

vpath %.c  $(FDT_PORT)
vpath %.cc $(REP_DIR)/src/lib/fdt
