TARGET  := tuxmath

TUXMATH_DIR := $(call select_from_ports,tuxmath)/src/app/tuxmath

LIBS    += base libc libm sdl sdl_image sdl_mixer sdl_ttf sdl_net tuxmath_t4k

INC_DIR += $(REP_DIR)/src/app/tuxmath \
           $(TUXMATH_DIR)/t4k/src

CC_OPT  += -DDATA_PREFIX=\"/data\" \
           -DLOCALEDIR=\"/locale\"

SRC_C   := tuxmath.c setup.c titlescreen.c menu.c menu_lan.c game.c \
           factoroids.c fileops_media.c options.c credits.c highscore.c \
           audio.c network.c mathcards.c campaign.c multiplayer.c fileops.c \
           SDL_rotozoom.c lessons.c server.c

#
# Disable sound during the game because the current version of sdl_mixer is
# compiled w/o ogg/mod, which produces a stream of error messages.
#
CC_OPT_game := -DNOSOUND

# suppress build noise caused by warnings in 3rd-party code
CC_WARN  = -Wno-address

vpath %.c $(TUXMATH_DIR)/src

SRC_CC  += getenv.cc

CUSTOM_TARGET_DEPS += tuxmath_data.tar

BUILD_ARTIFACTS := $(TARGET) tuxmath_data.tar

$(TARGET): tuxmath_data.tar
tuxmath_data.tar:
	$(VERBOSE)cd $(TUXMATH_DIR);     tar cf $(PWD)/bin/$@ data
	$(VERBOSE)cd $(TUXMATH_DIR)/t4k; tar rf $(PWD)/bin/$@ data

CC_CXX_WARN_STRICT =
