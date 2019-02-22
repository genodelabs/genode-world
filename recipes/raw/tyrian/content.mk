MIRROR_FROM_PORT_AND_REP_DIR := src/app/opentyrian

content: tyrian.tar

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/opentyrian)/src/app/opentyrian

tyrian.tar:
	$(VERBOSE) tar cf $@ -C $(PORT_DIR)/tyrian21 .
