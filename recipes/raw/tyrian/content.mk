MIRROR_FROM_PORT_AND_REP_DIR := src/app/opentyrian

content: tyrian.tar

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/opentyrian)/src/app/opentyrian

TAR_OPT := --owner=0 --group=0 --numeric-owner --mode='go=' --mtime='1970-01-01 00:00+00'

tyrian.tar:
	$(VERBOSE) tar $(TAR_OPT) -cf $@ -C $(PORT_DIR)/tyrian21 .
