CMUS_DIR := $(call select_from_ports,cmus)/src/app/cmus

TARGET := cmus
LIBS   := base libc posix ncurses libiconv

INC_DIR += $(CMUS_DIR)
INC_DIR += $(PRG_DIR)

CC_OPT += -DVERSION=\"2.8.0-rc3\" -DHAVE_CONFIG

#
# We need symbols provided by the binary
#
LD_OPT += --export-dynamic

SRC_C := \
         ape.c \
         browser.c \
         buffer.c \
         cache.c \
         channelmap.c \
         cmdline.c \
         cmus.c \
         command_mode.c \
         comment.c \
         convert.c \
         cue.c \
         cue_utils.c \
         debug.c \
         discid.c \
         editable.c \
         expr.c \
         file.c \
         filters.c \
         format_print.c \
         gbuf.c \
         glob.c \
         help.c \
         history.c \
         http.c \
         id3.c \
         input.c \
         job.c \
         keys.c \
         keyval.c \
         lib.c \
         load_dir.c \
         locking.c \
         mergesort.c \
         misc.c \
         options.c \
         output.c \
         path.c \
         pcm.c \
         pl.c \
         play_queue.c \
         player.c \
         prog.c \
         rbtree.c \
         read_wrapper.c \
         search.c \
         search_mode.c \
         server.c \
         spawn.c \
         tabexp.c \
         tabexp_file.c \
         track.c \
         track_info.c \
         tree.c \
         u_collate.c \
         uchar.c \
         ui_curses.c \
         window.c \
         worker.c \
         xmalloc.c \
         xstrjoin.c

vpath %.c $(CMUS_DIR)
