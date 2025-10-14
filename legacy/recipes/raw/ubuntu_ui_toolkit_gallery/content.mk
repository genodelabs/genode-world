content: ubuntu_ui_toolkit_gallery.tar

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/ubuntu-ui-toolkit)

include $(GENODE_DIR)/repos/base/recipes/content.inc

ubuntu_ui_toolkit_gallery.tar:
	$(TAR) \
	    -cf $@ \
	    -C $(PORT_DIR)/src/lib/ubuntu-ui-toolkit/examples \
	    ubuntu-ui-toolkit-gallery
