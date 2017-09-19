include $(REP_DIR)/lib/mk/fdk-aac.inc

SRC_CC := $(notdir $(wildcard $(FDK_AAC_SRC_DIR)/libSBRenc/src/*.cpp))

vpath %.cpp $(FDK_AAC_SRC_DIR)/libSBRenc/src
