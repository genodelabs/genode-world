SRC_DIR = src/test/rdrand
include $(GENODE_DIR)/repos/base/recipes/src/content.inc

content: include/spec/x86_64/os/rdrand.h

include/spec/x86_64/os/rdrand.h:
	$(mirror_from_rep_dir)
