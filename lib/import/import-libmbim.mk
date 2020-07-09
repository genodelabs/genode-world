LIBMBIM_PORT_DIR = $(call select_from_ports,libmbim)
INC_DIR += $(LIBMBIM_PORT_DIR)/include/libmbim-glib

LIBMBIM_GENERATED_PORT_DIR := $(call select_from_ports,libmbim_generated)
INC_DIR += $(LIBMBIM_GENERATED_PORT_DIR)/include/libmbim-glib
INC_DIR += $(LIBMBIM_GENERATED_PORT_DIR)/src/lib/libmbim_generated/generated
