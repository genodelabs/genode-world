content: glmark2_assets.tar

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/glmark2)/src/app/glmark2

TAR_OPT := --owner=0 --group=0 --numeric-owner --mode='go=' --mtime='2021-04-29 00:00+00'

glmark2_assets.tar:
	tar $(TAR_OPT) -cf $@ -C $(PORT_DIR)/data .
