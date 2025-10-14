MIRROR_FROM_PORT_AND_REP_DIR := src/app/opentyrian

content: tyrian.tar

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/opentyrian)/src/app/opentyrian

include $(GENODE_DIR)/repos/base/recipes/content.inc

tyrian.tar:
	$(VERBOSE)$(TAR) -cf $@ -C $(PORT_DIR)/tyrian21 .
