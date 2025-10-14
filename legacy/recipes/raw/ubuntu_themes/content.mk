content: ubuntu-themes.tar

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/ubuntu-themes)

include $(GENODE_DIR)/repos/base/recipes/content.inc

ubuntu-themes.tar:
	$(TAR) \
	    -cf $@ \
	    --transform='s/ubuntu-themes/usr\/share\/icons/' \
	    --transform='s/suru-icons/suru/' \
	    -C $(PORT_DIR) \
	    ubuntu-themes/suru-icons \
	    ubuntu-themes/ubuntu-mobile
