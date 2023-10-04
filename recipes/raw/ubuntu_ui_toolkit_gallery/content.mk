content: ubuntu_ui_toolkit_gallery.tar

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/ubuntu-ui-toolkit)

TAR_OPT := --owner=0 --group=0 --numeric-owner --mode='go=' --mtime='2022-06-29 00:00+00'

ubuntu_ui_toolkit_gallery.tar:
	tar $(TAR_OPT) \
	    -cf $@ \
	    -C $(PORT_DIR)/src/lib/ubuntu-ui-toolkit/examples \
	    ubuntu-ui-toolkit-gallery
