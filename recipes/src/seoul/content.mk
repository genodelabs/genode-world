PORT_DIR_SEOUL := $(call port_dir,$(REP_DIR)/ports/seoul)

SRC_DIR = src/app/seoul

content: $(SRC_DIR) src/include

$(SRC_DIR):
	mkdir -p $@
	cp -rH $(REP_DIR)/$@/* $@/
	cp -r $(PORT_DIR_SEOUL)/$@/* $@/
	cp $(PORT_DIR_SEOUL)/$@/LICENSE .

BASE_SRC_INCLUDE := src/include/base/internal/crt0.h \
                    src/include/base/internal/globals.h

src/include:
	mkdir -p $@/base/internal
	for file in $(BASE_SRC_INCLUDE); do \
		cp $(GENODE_DIR)/repos/base/$$file $$file; \
	done

MIRROR_FROM_WORLD_DIR := lib/mk/seoul-qemu-usb.mk

content: $(MIRROR_FROM_WORLD_DIR)

$(MIRROR_FROM_WORLD_DIR):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/world/$@ $(dir $@)

MIRROR_FROM_LIBPORTS := lib/import/import-qemu-usb_include.mk \
                        lib/mk/qemu-usb_include.mk \
                        lib/mk/qemu-usb.inc \
                        include/qemu \
                        src/lib/qemu-usb

content: $(MIRROR_FROM_LIBPORTS)

$(MIRROR_FROM_LIBPORTS):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/libports/$@ $(dir $@)

QEMU_USB_PORT_DIR := $(call port_dir,$(GENODE_DIR)/repos/libports/ports/qemu-usb)

MIRROR_FROM_QEMU_USB_PORT_DIR := src/lib/qemu

content: $(MIRROR_FROM_QEMU_USB_PORT_DIR)

$(MIRROR_FROM_QEMU_USB_PORT_DIR):
	mkdir -p $(dir $@)
	cp -r $(QEMU_USB_PORT_DIR)/$@ $(dir $@)

content:

MIRROR_FROM_BASE := lib/mk/cxx.mk

content: $(MIRROR_FROM_BASE)

$(MIRROR_FROM_BASE):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/base/$@ $(dir $@)

MIRROR_FROM_OS := include/pointer/shape_report.h

content: $(MIRROR_FROM_OS)

$(MIRROR_FROM_OS):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/os/$@ $(dir $@)
