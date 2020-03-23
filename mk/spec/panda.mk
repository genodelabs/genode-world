#
# Enable peripherals of the platform
#
SPECS += omap4 usb panda gpio framebuffer

#
# Pull in CPU specifics
#
SPECS += arm_v7a

include $(BASE_DIR)/mk/spec/arm_v7a.mk
