SRC_GO_PKG := greet.go srv.go
# im.go
# greet.go srv.go
#
# should prepare goroot port before!

GOBUILD_VERBOSE := -x

include $(REP_DIR)/lib/mk/gobuild.inc

# for network apps
LIBS   += net

CC_CXX_WARN_STRICT =
