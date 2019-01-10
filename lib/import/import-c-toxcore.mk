TOXCORE_PORT_DIR := $(call select_from_ports,c-toxcore)

INC_DIR += $(TOXCORE_PORT_DIR)/include
INC_DIR += $(call select_from_repositories,include/toxcore)
