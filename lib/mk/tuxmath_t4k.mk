T4K_DIR := $(call select_from_ports,tuxmath)/src/app/tuxmath/t4k

LIBS    += libc sdl sdl_image sdl_mixer sdl_ttf sdl_net libiconv libxml2

INC_DIR += $(REP_DIR)/src/app/tuxmath/t4k \
           $(T4K_DIR)/src/linebreak

CC_OPT  += -DCOMMON_DATA_PREFIX=\"/data\"

SRC_C   := $(notdir $(wildcard $(T4K_DIR)/src/*.c) \
                    $(wildcard $(T4K_DIR)/src/linebreak/unistr/*.c) \
                    width.c linebreak.c)

CC_WARN += -Wall -Wno-unused-function

vpath %.c $(T4K_DIR)/src
vpath %.c $(T4K_DIR)/src/linebreak
vpath %.c $(T4K_DIR)/src/linebreak/unistr
vpath %.c $(T4K_DIR)/src/linebreak/uniwidth
