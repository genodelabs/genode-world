include $(REP_DIR)/lib/import/import-opus.mk

LIBS += libc libm

CC_DEF += -DOPUS_BUILD -DUSE_ALLOCA

OPUS_SRC_DIR = $(OPUS_PORT_DIR)/src/lib/opus

-include $(OPUS_SRC_DIR)/silk_sources.mk
-include $(OPUS_SRC_DIR)/celt_sources.mk
-include $(OPUS_SRC_DIR)/opus_sources.mk

INC_DIR += $(OPUS_SRC_DIR)/silk
INC_DIR += $(OPUS_SRC_DIR)/silk/float
INC_DIR += $(OPUS_SRC_DIR)/celt

SRC_C += $(notdir $(SILK_SOURCES) $(SILK_SOURCES_FLOAT))
SRC_C += $(notdir $(CELT_SOURCES))
SRC_C += $(notdir $(OPUS_SOURCES) $(OPUS_SOURCES_FLOAT))

vpath %.c $(OPUS_SRC_DIR)/silk $(OPUS_SRC_DIR)/silk/float
vpath %.c $(OPUS_SRC_DIR)/celt
vpath %.c $(OPUS_SRC_DIR)/opus
vpath %.c $(OPUS_SRC_DIR)/src

SHARED_LIB = yes
