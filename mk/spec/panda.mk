#
# Enable peripherals of the platform
#
SPECS += omap4 usb panda gpio framebuffer

#
# Pull in CPU specifics
#
SPECS += arm_v7a

include $(call select_from_repositories,mk/spec/arm_v7a.mk)
