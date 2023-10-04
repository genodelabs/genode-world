content: ubuntu-themes.tar

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/ubuntu-themes)

TAR_OPT := --owner=0 --group=0 --numeric-owner --mode='go=' --mtime='2022-06-29 00:00+00'

ubuntu-themes.tar:
	tar $(TAR_OPT) \
	    -cf $@ \
	    --transform='s/ubuntu-themes/usr\/share\/icons/' \
	    --transform='s/suru-icons/suru/' \
	    -C $(PORT_DIR) \
	    ubuntu-themes/suru-icons \
	    ubuntu-themes/ubuntu-mobile
