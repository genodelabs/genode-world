SRC_GO_PKG := greet.go srv.go
#im.go

include $(REP_DIR)/lib/mk/gobuild.inc

# for network apps
LIBS   += net

CC_CXX_WARN_STRICT =
