SRC_DIR = src/test/rdrand
include $(GENODE_DIR)/repos/base/recipes/src/content.inc

MIRROR_FROM_REP_DIR := include/spec/x86_64/world/rdrand.h include/world/rdrand.h
content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)
