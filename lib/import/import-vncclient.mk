LIBVNCCLIENT_PORT_DIR := $(call select_from_ports,libvnc)

INC_DIR += $(LIBVNCCLIENT_PORT_DIR)/src/lib/vnc \
           $(REP_DIR)/include/libvnc
