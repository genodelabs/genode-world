TARGET   = omap4_sd_card_drv
REQUIRES = arm_v7
SRC_CC  += main.cc driver.cc
LIBS    += base

INC_DIR += $(PRG_DIR) $(BASE_DIR)/../os/src/drivers/sd_card

vpath %.cc $(BASE_DIR)/../os/src/drivers/sd_card
vpath %.cc $(PRG_DIR)
