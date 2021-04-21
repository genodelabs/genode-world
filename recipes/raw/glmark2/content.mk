content: glmark2_assets.tar

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/glmark2)/src/app/glmark2

glmark2_assets.tar:
	tar --mtime='2021-04-29 00:00Z' -cf $@ -C $(PORT_DIR)/data .
