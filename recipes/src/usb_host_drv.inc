LIB_MK := lib/mk/usb_host_include.mk lib/import/import-usb_arch_include.mk \
          $(foreach SPEC,arm arm_64 x86_32 x86_64,lib/mk/spec/$(SPEC)/lx_kit_setjmp.mk)

DDE_LINUX_REP_DIR := $(GENODE_DIR)/repos/dde_linux

PORT_DIR := $(call port_dir,$(DDE_LINUX_REP_DIR)/ports/dde_linux)

MIRROR_FROM_DDE_LINUX_REP_DIR := $(LIB_MK) \
                                 lib/import/import-usb_host_include.mk \
                                 src/include src/lx_kit \
                                 $(shell cd $(DDE_LINUX_REP_DIR); \
                                    find src/drivers/usb_host -type f -name "*.h" \
                                                                  -or -name "*.c" \
                                                                  -or -name "*.inc" \
                                                                  -or -name "README")

MIRROR_FROM_PORT_DIR := $(shell cd $(PORT_DIR); find src/drivers/usb_host -type f | grep -v ".git")
MIRROR_FROM_PORT_DIR := $(filter-out $(MIRROR_FROM_DDE_LINUX_REP_DIR),$(MIRROR_FROM_PORT_DIR))

content: $(MIRROR_FROM_REP_DIR) $(MIRROR_FROM_DDE_LINUX_REP_DIR) $(MIRROR_FROM_PORT_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

$(MIRROR_FROM_DDE_LINUX_REP_DIR):
	mkdir -p $(dir $@)
	cp -r $(DDE_LINUX_REP_DIR)/$@ $@

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp $(PORT_DIR)/$@ $@

content: LICENSE
LICENSE:
	( echo "GNU General Public License version 2, see:"; \
	  echo "https://www.kernel.org/pub/linux/kernel/COPYING" ) > $@
