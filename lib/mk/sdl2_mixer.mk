SDL2_MIXER_PORT_DIR := $(call select_from_ports,sdl2_mixer)
SRC_DIR := $(SDL2_MIXER_PORT_DIR)/src/lib/sdl2_mixer

LIBS += libc libm sdl2 libFLAC libogg libvorbis

SRC_C := $(addprefix src/,$(notdir $(wildcard $(SRC_DIR)/src/*.c))) \
         $(addprefix src/codecs/,$(notdir $(wildcard $(SRC_DIR)/src/codecs/*.c)))

INC_DIR += $(SDL2_MIXER_PORT_DIR)/include/SDL2
INC_DIR += $(SRC_DIR)/src
INC_DIR += $(SRC_DIR)/src/codecs

#
# In case we use the depot add the location
# to the global include path.
#
ifeq ($(CONTRIB),)
REP_INC_DIR += include/SDL2
endif


# suppress warnings of 3rd-party code
#CC_OPT_music           = -Wno-unused-label -Wno-unused-function
#CC_OPT_load_aiff       = -Wno-unused-but-set-variable
#CC_OPT_wavestream      = -Wno-unused-but-set-variable
#CC_OPT_effect_position = -Wno-misleading-indentation

CC_OPT += -DMUSIC_OGG -DMUSIC_FLAC_LIBFLAC

# fix opusfile.h header file location <opus/…>
#CC_OPT += -DMUSIC_OPUS

vpath %.c $(SRC_DIR)

SHARED_LIB := yes

CC_CXX_WARN_STRICT =
