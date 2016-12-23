#
# uhexen2
#

UHEXEN2_DIR = $(call select_from_ports,uhexen2)/src/app/uhexen2

TARGET = uhexen2

# softobjs
SRC_C = d_edge.c \
        d_fill.c \
        d_init.c \
        d_modech.c \
        d_part.c \
        d_polyse.c \
        d_scan.c \
        d_sky.c \
        d_sprite.c \
        d_surf.c \
        d_vars.c \
        d_zpoint.c \
        r_aclip.c \
        r_alias.c \
        r_bsp.c \
        r_draw.c \
        r_edge.c \
        r_efrag.c \
        r_light.c \
        r_main.c \
        r_misc.c \
        r_part.c \
        r_sky.c \
        r_sprite.c \
        r_surf.c \
        r_vars.c \
        screen.c \
        vid_sdl.c \
        draw.c \
        model.c

SRC_C += bgmnull.c \
         midi_nul.c \
         cd_null.c

SRC_C += snd_sdl.c \
         snd_sys.c \
         snd_mix.c \
         snd_dma.c \
         snd_mem.c \
         snd_codec.c \
         snd_wave.c

# commonobjs
SRC_C += in_sdl.c \
         chase.c \
         cl_demo.c \
         cl_effect.c \
         cl_input.c \
         cl_inlude.c \
         cl_main.c \
         cl_parse.c \
         cl_string.c \
         cl_tent.c \
         cl_cmd.c \
         console.c \
         keys.c \
         menu.c \
         sbar.c \
         view.c \
         wad.c \
         cmd.c \
         q_endian.c \
         link_ops.c \
         sizebuf.c \
         strlcat.c \
         strlcpy.c \
         qsnprint.c \
         msg_io.c \
         common.c \
         debuglog.c \
         quakefs.c \
         crc.c \
         cvar.c \
         cfgfile.c \
         host.c \
         host_cmd.c \
         host_string.c \
         mathlib.c \
         pr_cmds.c \
         pr_edict.c \
         pr_exec.c \
         sv_effect.c \
         sv_main.c \
         sv_move.c \
         sv_phys.c \
         sv_user.c \
         world.c \
         zone.c \
         net_bsd.c \
         net_dgrm.c \
         net_loop.c \
         net_main.c \
         net_udp_genode.c

SRC_CC = sys_genode.cc


INC_DIR += $(UHEXEN2_DIR)/engine/hexen2
INC_DIR += $(UHEXEN2_DIR)/engine/h2shared
INC_DIR += $(UHEXEN2_DIR)/libs/common
INC_DIR += $(UHEXEN2_DIR)/common
INC_DIR += $(REP_DIR)/include/SDL

CC_OPT  += -ffast-math
CC_OPT  += -DSDLQUAKE -D_NO_MIDIDRV -D_NO_CDAUDIO -DPARANOID

CC_OPT  += -DNO_ALSA_AUDIO -DNO_OSS_AUDIO -DNO_SUN_AUDIO

CC_OPT  += -DDEMOBUILD

LIBS = posix libc_lwip lwip libm sdl sdl_mixer config_args

vpath % $(UHEXEN2_DIR)/common
vpath % $(UHEXEN2_DIR)/engine/hexen2
vpath % $(UHEXEN2_DIR)/engine/h2shared
vpath % $(UHEXEN2_DIR)/libs/common
