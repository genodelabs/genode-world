TARGET   = sd_card_drv
REQUIRES = zynq_sdhci
SRC_CC   = main.cc
LIBS     = base server
INC_DIR += $(PRG_DIR)
INC_DIR += $(REP_DIR)/../os/src/drivers/sd_card/
INC_DIR += $(REP_DIR)/../os/src/drivers/sd_card/spec/rpi
