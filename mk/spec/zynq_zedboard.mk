#
# Pull in CPU specifics
#
SPECS += zynq cadence_gem zynq_i2c

include $(call select_from_repositories,mk/spec/zynq.mk)
