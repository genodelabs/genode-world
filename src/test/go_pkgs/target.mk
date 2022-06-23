SRC_GO_PKG := im.go
# greet.go srv.go
#

GOBUILD_VERBOSE := -x

include $(REP_DIR)/lib/mk/gobuild.inc

# for network apps
LIBS   += net

CC_CXX_WARN_STRICT =
