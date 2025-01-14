MIRROR_FROM_REP_DIR := lib/import/import-mbedtls.mk
MIRROR_FROM_REP_DIR += lib/symbols/mbedtls

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/mbedtls)

content: include/mbedtls LICENSE

include/mbedtls:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/mbedtls/include/mbedtls/* $@/

LICENSE:
	cp $(PORT_DIR)/src/lib/mbedtls/LICENSE $@
