LIBFFI_DIR := $(call select_from_ports,libffi)/src/lib/libffi
#SRC_C = $(notdir $(wildcard $(LIBATOMIC_DIR)/SDLnet*.c))

#vpath %.c $(LIBATOMIC_DIR)

#INC_DIR += $(LIBATOMIC_DIR)

LIBS += libc

#SHARED_LIB = no
