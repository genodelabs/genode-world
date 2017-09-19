FDK_AAC_SRC_DIR := $(call select_from_ports,fdk-aac)/src/lib/fdk-aac

include $(REP_DIR)/lib/import/import-fdk-aac.mk

LIBS += libc

CC_OPT  += -fno-exceptions -fno-rtti
CC_WARN += -Wno-sign-compare

INC_DIR += \
	$(FDK_AAC_SRC_DIR)/libAACdec/include \
	$(FDK_AAC_SRC_DIR)/libAACenc/include \
	$(FDK_AAC_SRC_DIR)/libSBRdec/include \
	$(FDK_AAC_SRC_DIR)/libSBRenc/include \
	$(FDK_AAC_SRC_DIR)/libMpegTPDec/include \
	$(FDK_AAC_SRC_DIR)/libMpegTPEnc/include \
	$(FDK_AAC_SRC_DIR)/libSYS/include \
	$(FDK_AAC_SRC_DIR)/libFDK/include \
	$(FDK_AAC_SRC_DIR)/libPCMutils/include \
