include $(REP_DIR)/lib/mk/av.inc

include $(REP_DIR)/lib/import/import-avdevice.mk

LIBAVDEVICE_DIR = $(call select_from_ports,libav)/src/lib/libav/libavdevice

-include $(LIBAVDEVICE_DIR)/Makefile

LIBS += avformat

vpath % $(LIBAVDEVICE_DIR)
