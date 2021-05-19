TARGET = test-vncserver
LIBS   = libc vncserver zlib posix

INC_DIR  += $(LIBVNC_PORT_DIR)/src/lib/vnc/examples
SRC_C = example.c

vpath example.c $(LIBVNC_PORT_DIR)/src/lib/vnc/examples
