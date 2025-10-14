content: glmark2_assets.tar

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/glmark2)/src/app/glmark2

include $(GENODE_DIR)/repos/base/recipes/content.inc

glmark2_assets.tar:
	$(TAR) -cf $@ -C $(PORT_DIR)/data .
