LIB_MK_FILES := java.inc jdk_version.inc jimage.mk jli.mk verify.mk \
                jnet.mk jvm.inc jzip.mk management.mk nio.mk \
               $(foreach SPEC,x86_64 arm, \
                spec/$(SPEC)/java.mk spec/$(SPEC)/jvm.mk) \

MIRROR_FROM_REP_DIR := src/app/jdk \
                       $(addprefix lib/mk/,$(LIB_MK_FILES)) \
                       lib/import/import-jli.mk

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

MIRROR_FROM_LIBPORTS := include/libc-plugin

content: $(MIRROR_FROM_LIBPORTS)

$(MIRROR_FROM_LIBPORTS):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/libports/$@ $@

PORT_DIR      := $(call port_dir,$(REP_DIR)/ports/jdk)
GENERATED_DIR := $(call port_dir,$(REP_DIR)/ports/jdk_generated)

src/app/jdk/hotspot: $(MIRROR_FROM_REP_DIR)
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/src/app/jdk/* src/app/jdk/

src/app/jdk/src: src/app/jdk/hotspot
	cp -r $(GENERATED_DIR)/src/app/jdk/include \
	      $(GENERATED_DIR)/src/app/jdk/src \
	      src/app/jdk

content: src/app/jdk/src

content: LICENSE

LICENSE:
	cp $(PORT_DIR)/src/app/jdk/$@ $@
