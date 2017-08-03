GLIB_PORT_DIR := $(call select_from_ports,glib)
INC_DIR += $(GLIB_PORT_DIR)/include/glib $(call select_from_repositories,include/glib)
