MIRROR_FROM_REP_DIR := lib/mk/freebl3.inc \
                       lib/mk/nss3_common.inc \
                       lib/mk/nss3.inc \
                       lib/mk/nssckbi.inc \
                       lib/mk/softokn3.inc \
                       lib/mk/spec/arm_64/freebl3.mk \
                       lib/mk/spec/arm_64/nss3.mk \
                       lib/mk/spec/arm_64/nssckbi.mk \
                       lib/mk/spec/arm_64/softokn3.mk \
                       lib/mk/spec/x86_64/freebl3.mk \
                       lib/mk/spec/x86_64/nss3.mk \
                       lib/mk/spec/x86_64/nssckbi.mk \
                       lib/mk/spec/x86_64/softokn3.mk

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/nss3)

MIRROR_FROM_PORT_DIR := src/lib/nspr \
                        src/lib/nss

content: $(MIRROR_FROM_PORT_DIR)

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)

content: LICENSE

LICENSE:
	cp $(PORT_DIR)/src/lib/nss/COPYING $@
