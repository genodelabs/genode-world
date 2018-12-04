CRYPTOPP_PORT_DIR := $(call port_dir,$(REP_DIR)/ports/cryptopp)
MIRROR_FROM_CRYPTOPP = src/lib/cryptopp include

MIRROR_FROM_REP_DIR := src/app/gtotp_report lib/import/import-cryptopp.mk lib/mk/cryptopp.mk

content: $(MIRROR_FROM_REP_DIR) $(MIRROR_FROM_CRYPTOPP) LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

$(MIRROR_FROM_CRYPTOPP): $(CRYPTOPP_PORT_DIR)
	cp -r $</include $</src .


LICENSE:
	cp $(GENODE_DIR)/LICENSE $@
