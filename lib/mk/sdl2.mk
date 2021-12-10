SDL2_PORT_DIR := $(call select_from_ports,sdl2)
SDL2_DIR      := $(SDL2_PORT_DIR)/src/lib/sdl2

# build shared object
SHARED_LIB = yes

CC_OPT  += -DGENODE

CC_OPT += -DSDL_VIDEO_OPENGL=1
CC_OPT += -DSDL_VIDEO_OPENGL_EGL=1

CC_WARN += -Wno-unused-variable

# because of AARCH_64 : /gcc/aarch64-none-elf/8.3.0/include/arm_neon.h: narrowing conversion
CC_WARN += -Wno-narrowing

INC_DIR += $(SDL2_PORT_DIR)/include
INC_DIR += $(SDL2_PORT_DIR)/include/SDL2

#
# In case we use the depot add the location
# to the global include path.
#
ifeq ($(CONTRIB),)
REP_INC_DIR += include/SDL2
endif

# backends
SRC_CC   = \
           video/SDL_genode_fb_video.cc \
           video/SDL_genode_fb_events.cc \
           loadso/SDL_loadso.cc

INC_DIR += $(REP_DIR)/include/SDL2 \
           $(REP_DIR)/src/lib/sdl2 \
           $(REP_DIR)/src/lib/sdl2/thread \
           $(REP_DIR)/src/lib/sdl2/video

# main files
SRC_C    = SDL.c \
           SDL_assert.c \
           SDL_dataqueue.c \
           SDL_error.c \
           SDL_hints.c \
           SDL_log.c
INC_DIR += $(SDL2_DIR)/src

# atomic subsystem
SRC_C += $(addprefix atomic/,$(notdir $(wildcard $(SDL2_DIR)/src/atomic/*.c)))

# audio subsystem
SRC_C += $(addprefix audio/,$(notdir $(wildcard $(SDL2_DIR)/src/audio/*.c)))
INC_DIR += $(SDL2_DIR)/src/audio

SRC_C += $(addprefix audio/dsp/,$(notdir $(wildcard $(SDL2_DIR)/src/audio/dsp/*.c)))
INC_DIR += $(SDL2_DIR)/src/audio/dsp

# sensor subsystem
SRC_C += $(addprefix sensor/,$(notdir $(wildcard $(SDL2_DIR)/src/sensor/*.c)))

# cpuinfo subsystem
SRC_C   += cpuinfo/SDL_cpuinfo.c

# event subsystem
SRC_C += $(addprefix events/,$(notdir $(wildcard $(SDL2_DIR)/src/events/*.c)))
INC_DIR += $(SDL2_DIR)/src/events

# file I/O subsystem
SRC_C   += file/SDL_rwops.c

# filesystem subsystem
SRC_C   += filesystem/unix/SDL_sysfilesystem.c

# haptic subsystem
SRC_C   += haptic/SDL_haptic.c \
           haptic/dummy/SDL_syshaptic.c
INC_DIR += $(SDL2_DIR)/src/haptic

# joystick subsystem
SRC_C   += joystick/SDL_joystick.c \
           joystick/SDL_gamecontroller.c \
           joystick/dummy/SDL_sysjoystick.c
INC_DIR += $(SDL2_DIR)/src/joystick

# render subsystem
SRC_C   += $(addprefix render/,$(notdir $(wildcard $(SDL2_DIR)/src/render/*.c)))
SRC_C   += $(addprefix render/software/,$(notdir $(wildcard $(SDL2_DIR)/src/render/software/*.c)))
INC_DIR += $(SDL2_DIR)/src/render $(SDL2_DIR)/src/render/software

# stdlib files
SRC_C   += stdlib/SDL_getenv.c \
           stdlib/SDL_malloc.c \
           stdlib/SDL_qsort.c \
           stdlib/SDL_stdlib.c \
           stdlib/SDL_string.c

# thread subsystem
SRC_C   += thread/SDL_thread.c \
           thread/pthread/SDL_syscond.c \
           thread/pthread/SDL_sysmutex.c \
           thread/pthread/SDL_systls.c \
           thread/pthread/SDL_syssem.c \
           thread/pthread/SDL_systhread.c

# timer subsystem
SRC_C   += timer/unix/SDL_systimer.c

# video subsystem
SRC_C += $(addprefix video/,$(notdir $(wildcard $(SDL2_DIR)/src/video/*.c)))
INC_DIR += $(SDL2_DIR)/src/video
SRC_C += $(addprefix video/yuv2rgb/,$(notdir $(wildcard $(SDL2_DIR)/src/video/yuv2rgb/*.c)))

SRC_CC  += sdl_main.cc

# we need libc
LIBS = libc egl mesa

# backend path
vpath % $(REP_DIR)/src/lib/sdl2

vpath % $(SDL2_DIR)/src

CC_CXX_WARN_STRICT_CONVERSION =
