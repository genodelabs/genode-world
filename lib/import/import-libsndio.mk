SNDIO_PORT_DIR := $(call select_from_ports,sndio)

ifeq ($(CONTRIB_DIR),)
INC_DIR += $(call select_from_repositories,include/sndio)
else
INC_DIR += $(SNDIO_PORT_DIR)/include/sndio
endif
