SDL2_NET_PORT_DIR := $(call select_from_ports,sdl2_net)
SDL2_NET_DIR      := $(SDL2_NET_PORT_DIR)/src/lib/sdl2_net

LIBS += libc sdl2

INC_DIR += $(SDL2_NET_DIR)

#
# In case we use the depot add the location
# to the global include path.
#
ifeq ($(CONTRIB),)
REP_INC_DIR += include/SDL2
endif

SRC_C := $(notdir $(wildcard $(SDL2_NET_DIR)/SDLnet*.c))

vpath %.c $(SDL2_NET_DIR)

SHARED_LIB := yes

CC_CXX_WARN_STRICT :=
