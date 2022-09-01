OPENSC_DIR := $(call port_dir,$(REP_DIR)/ports/opensc)

MIRROR_FROM_OPENSC_DIR := \
	$(shell \
		cd $(OPENSC_DIR); \
		find src/opensc -type f | \
		grep -v "\.git") \

MIRROR_FROM_REP_DIR += \
	lib/mk/opensc_pkcs11.mk \
	$(shell cd $(REP_DIR) && find src/lib/opensc_pkcs11 -type f)

content: $(MIRROR_FROM_OPENSC_DIR)

$(MIRROR_FROM_OPENSC_DIR):
	mkdir -p $(dir $@)
	cp -r $(OPENSC_DIR)/$@ $(dir $@)

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

content: LICENSE

LICENSE:
	cp $(OPENSC_DIR)/src/opensc/COPYING $@
