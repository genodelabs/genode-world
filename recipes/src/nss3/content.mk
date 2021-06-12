MIRROR_FROM_REP_DIR := lib/mk/nss3.inc \
                       lib/mk/freebl3.mk \
                       lib/mk/nss3.mk \
                       lib/mk/nssckbi.mk \
                       lib/mk/softokn3.mk \
                       src/lib/freebl3/target.mk \
                       src/lib/nssckbi/target.mk \
                       src/lib/softokn3/target.mk

content: $(MIRROR_FROM_REP_DIR) src/lib/nss3/target.mk

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

src/lib/nss3/target.mk:
	mkdir -p $(dir $@)
	echo "LIBS = nss3" > $@

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
