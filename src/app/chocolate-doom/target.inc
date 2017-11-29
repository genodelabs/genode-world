CHOCOLATE_DOOM_DIR = $(call select_from_ports,chocolate-doom)/src/app/chocolate-doom

# textscreen
SRC_C += $(notdir $(wildcard $(CHOCOLATE_DOOM_DIR)/textscreen/*.c))
# opl
SRC_C += $(notdir $(wildcard $(CHOCOLATE_DOOM_DIR)/opl/*.c))
# pcsound
SRC_C += $(notdir $(wildcard $(CHOCOLATE_DOOM_DIR)/pcsound/*.c))
# common engine source
SRC_C += \
         aes_prng.c      \
         d_event.c       \
         d_iwad.c        \
         d_loop.c        \
         d_mode.c        \
         deh_io.c        \
         deh_main.c      \
         deh_mapping.c   \
         deh_str.c       \
         deh_text.c      \
         gusconf.c       \
         i_cdmus.c       \
         i_endoom.c      \
         i_joystick.c    \
         i_main.c        \
         i_oplmusic.c    \
         i_pcsound.c     \
         i_scale.c       \
         i_sdlmusic.c    \
         i_sdlsound.c    \
         i_sound.c       \
         i_system.c      \
         i_timer.c       \
         i_video.c       \
         i_videohr.c     \
         m_argv.c        \
         m_bbox.c        \
         m_cheat.c       \
         m_config.c      \
         m_controls.c    \
         m_fixed.c       \
         m_misc.c        \
         memio.c         \
         midifile.c      \
         mus2mid.c       \
         net_client.c    \
         net_common.c    \
         net_dedicated.c \
         net_gui.c       \
         net_io.c        \
         net_loop.c      \
         net_packet.c    \
         net_query.c     \
         net_sdl.c       \
         net_server.c    \
         net_structrw.c  \
         sha1.c          \
         tables.c        \
         v_video.c       \
         w_checksum.c    \
         w_file.c        \
         w_file_posix.c  \
         w_file_stdc.c   \
         w_file_win32.c  \
         w_main.c        \
         w_merge.c       \
         w_wad.c         \
         z_zone.c

# game src
SRC_C += $(notdir $(wildcard $(CHOCOLATE_DOOM_DIR)/src/$(GAME)/*.c))

vpath %.c $(CHOCOLATE_DOOM_DIR)/opl
vpath %.c $(CHOCOLATE_DOOM_DIR)/pcsound
vpath %.c $(CHOCOLATE_DOOM_DIR)/src
vpath %.c $(CHOCOLATE_DOOM_DIR)/src/$(GAME)
vpath %.c $(CHOCOLATE_DOOM_DIR)/textscreen

INC_DIR += $(PRG_DIR)/..
INC_DIR += $(CHOCOLATE_DOOM_DIR)/opl
INC_DIR += $(CHOCOLATE_DOOM_DIR)/pcsound
INC_DIR += $(CHOCOLATE_DOOM_DIR)/src
INC_DIR += $(CHOCOLATE_DOOM_DIR)/src/$(GAME)
INC_DIR += $(CHOCOLATE_DOOM_DIR)/textscreen

LIBS := base sdl sdlmain libc libm sdl_mixer sdl_net

# vi:set ft=make :
