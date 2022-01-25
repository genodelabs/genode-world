SRC_GO_PKG := greet.go srv.go
#im.go
#

MPRG_REL_DIR := $(subst $(REP_DIR)/src/,,$(PRG_DIR))
MPRG_REL_DIR := $(MPRG_REL_DIR:/=)
GO_PKG_BUILD := $(BUILD_BASE_DIR)/$(MPRG_REL_DIR)
SRCPATH      := $(REP_DIR)/src/$(MPRG_REL_DIR)/src

#include $(PRG_DIR)/gobase.mk

include $(REP_DIR)/lib/mk/gobuild.inc

# for network apps
LIBS   += net

CC_CXX_WARN_STRICT =
