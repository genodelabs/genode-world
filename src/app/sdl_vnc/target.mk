SDLVNCVIEWER_PORT_DIR := $(call select_from_ports,libvnc)

TARGET = sdl_vnc

SRC_C = SDLvncviewer.c scrap.c

LIBS = vncclient sdl libc zlib

vpath %.c $(SDLVNCVIEWER_PORT_DIR)/src/lib/vnc/client_examples
