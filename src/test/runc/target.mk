SRC_GO_PKG := github.com/opencontainers/runc

#
# should prepare goroot port per each ARCH before!
# checked: x86 linux/linux; arm_v8a hw/virt_qemu_arm_v8a
# trick: after first make... goto build/<ARCH>/test/runc and 
#        replace src/github.com/opencontainers/runc to git clone , like
#        git clone https://github.com/tor-m6/runc.git build/x86_64/test/runc/src/github.com/opencontainers/runc/

#GOBUILD_VERBOSE := -x -v

GOBUILD_FLAGS = -tags inno

include $(REP_DIR)/lib/mk/gobuild.inc

# for network apps
LIBS   += net

CC_CXX_WARN_STRICT =
