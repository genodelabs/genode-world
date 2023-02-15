SRC_GO_PKG := github.com/containerd/containerd/cmd/containerd

#
# should prepare goroot port per each ARCH before!
# checked: x86 linux/linux; arm_v8a hw/virt_qemu_arm_v8a
# trick: after first make... goto build/<ARCH>/test/runc and 
#        replace src/github.com/opencontainers/runc to git clone , like
#        git clone https://github.com/tor-m6/containerd.git build/x86_64/test/containerd/src/github.com/containerd/containerd/

GOBUILD_VERBOSE := -x -v

GOBUILD_FLAGS = -tags "no_btrfs no_cri no_devmapper inno static_build" -work

GOBUILD_REBUILD_ALL = yes
# GO_BUILD_LIB = no
# DO_GET = no

include $(REP_DIR)/lib/mk/gobuild.inc

# for network apps
LIBS   += net

CC_CXX_WARN_STRICT =
