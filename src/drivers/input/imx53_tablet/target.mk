TARGET    = imx53_tablet_input
REQUIRES  = arm_v7
SRC_CC    = main.cc
LIBS      = base
INC_DIR  += $(PRG_DIR)
INC_DIR  += $(call select_from_repositories,include/spec/imx53)
