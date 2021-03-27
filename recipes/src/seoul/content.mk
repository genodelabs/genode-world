PORT_DIR_SEOUL := $(call port_dir,$(REP_DIR)/ports/seoul)

SRC_DIR = src/app/seoul

content: $(SRC_DIR) src/include

$(SRC_DIR):
	mkdir -p $@
	cp -rH $(REP_DIR)/$@/* $@/
	cp -r $(PORT_DIR_SEOUL)/$@/* $@/
	cp $(PORT_DIR_SEOUL)/$@/LICENSE .

BASE_SRC_INCLUDE := src/include/base/internal/crt0.h \
                    src/include/base/internal/globals.h \
                    src/include/base/internal/unmanaged_singleton.h

src/include:
	mkdir -p $@/base/internal
	for file in $(BASE_SRC_INCLUDE); do \
		cp $(GENODE_DIR)/repos/base/$$file $$file; \
	done

MIRROR_FROM_WORLD_DIR := lib/mk/seoul_libc_support.mk

content: $(MIRROR_FROM_WORLD_DIR)

$(MIRROR_FROM_WORLD_DIR):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/world/$@ $(dir $@)


MIRROR_FROM_LIBPORTS := lib/mk/libc-common.inc

content: $(MIRROR_FROM_LIBPORTS)

$(MIRROR_FROM_LIBPORTS):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/libports/$@ $(dir $@)

content:

MIRROR_FROM_BASE := lib/mk/cxx.mk

content: $(MIRROR_FROM_BASE)

$(MIRROR_FROM_BASE):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/base/$@ $(dir $@)

PORT_DIR := $(call port_dir,$(GENODE_DIR)/repos/libports/ports/libc)

include $(REP_DIR)/lib/mk/seoul_libc_support.mk

MIRROR_FROM_LIBC := $(addprefix src/lib/libc/lib/libc/,$(SRC_C)) \
                    src/lib/libc/lib/libc/locale/mblocal.h \
                    src/lib/libc/lib/libc/locale/xlocale_private.h \
                    src/lib/libc/lib/libc/locale/setlocale.h \
                    src/lib/libc/lib/libc/include/libc_private.h \

MIRROR_FROM_OS := include/pointer/shape_report.h

content: $(MIRROR_FROM_LIBC) $(MIRROR_FROM_OS)

$(MIRROR_FROM_OS):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/os/$@ $(dir $@)

$(MIRROR_FROM_LIBC):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)
