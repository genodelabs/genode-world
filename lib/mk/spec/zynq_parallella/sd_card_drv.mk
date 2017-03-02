# use Raspberry Pi SD card driver 

TMP := $(call select_from_repositories,src/drivers/sd_card/spec/rpi/driver.cc)
OS_DIR := $(TMP:%src/drivers/sd_card/spec/rpi/driver.cc=%)

INC_DIR += $(OS_DIR)/src/drivers/sd_card/spec/rpi
SRC_CC  += $(OS_DIR)/src/drivers/sd_card/spec/rpi/driver.cc

INC_DIR += $(OS_DIR)/src/drivers/sd_card
SRC_CC  += main.cc
LIBS    += base

vpath %.cc $(OS_DIR)/src/drivers/sd_card
