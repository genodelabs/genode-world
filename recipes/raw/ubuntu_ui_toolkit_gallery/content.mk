content: ubuntu_ui_toolkit_gallery.tar

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/ubuntu-ui-toolkit)

ubuntu_ui_toolkit_gallery.tar:
	tar --mtime='2022-06-29 00:00Z' \
	    -cf $@ \
	    -C $(PORT_DIR)/src/lib/ubuntu-ui-toolkit/examples \
	    ubuntu-ui-toolkit-gallery
