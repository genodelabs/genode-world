TARGET = glmark2

LIBS = base libc libm mesa egl stdcxx libpng jpeg

SRC_CC = a.cc \
         base-renderer.cpp \
         benchmark-collection.cpp \
         benchmark.cpp \
         blur-renderer.cpp \
         canvas-generic.cpp \
         copy-renderer.cpp \
         d.cc \
         e.cc \
         gl-headers.cpp \
         gl-state-egl.cpp \
         gl-visual-config.cpp \
         i.cc \
         image-reader.cpp \
         lamp.cc \
         log.cc \
         logo.cc \
         luminance-renderer.cpp \
         main.cpp \
         main-loop.cpp \
         mat.cc \
         m.cc \
         mesh.cpp \
         model.cpp \
         n.cc \
         normal-from-height-renderer.cpp \
         o.cc \
         options.cpp \
         overlay-renderer.cpp \
         program.cc \
         renderer-chain.cpp \
         results-file.cpp \
         s.cc \
         scene-buffer.cpp \
         scene-build.cpp \
         scene-bump.cpp \
         scene-clear.cpp \
         scene-conditionals.cpp \
         scene.cpp \
         scene-default-options.cpp \
         scene-desktop.cpp \
         scene-effect-2d.cpp \
         scene-function.cpp \
         scene-grid.cpp \
         scene-ideas.cpp \
         scene-jellyfish.cpp \
         scene-loop.cpp \
         scene-pulsar.cpp \
         scene-refract.cpp \
         scene-shading.cpp \
         scene-shadow.cpp \
         scene-terrain.cpp \
         scene-texture.cpp \
         shader-source.cc \
         shared-library.cpp \
         simplex-noise-renderer.cpp \
         splines.cc \
         table.cc \
         t.cc \
         terrain-renderer.cpp \
         text-renderer.cpp \
         texture.cpp \
         texture-renderer.cpp \
         util.cc \

SRC_CC += native-state-genode.cc

SRC_C = dummies.c \
        egl.c \
        gles2.c

GLMARK2_DIR := $(call select_from_ports,glmark2)/src/app/glmark2

INC_DIR = $(REP_DIR)/src/app/glmark2 \
          $(GLMARK2_DIR)/src \
          $(GLMARK2_DIR)/src/glad/include \
          $(GLMARK2_DIR)/src/libmatrix \
          $(GLMARK2_DIR)/src/scene-ideas \
          $(GLMARK2_DIR)/src/scene-terrain

CC_OPT = -DUSE_EXCEPTIONS -DGLMARK2_USE_GLESv2 -DGLMARK2_USE_EGL \
         -DGLMARK_VERSION='"2023.01"' -DGLMARK_DATA_PATH='"/data"' \
         -D__GENODE__ -D GLMARK2_USE_GENODE \
         -DGLMARK2_EXECUTABLE='"glmark2"'

LD_OPT    = --export-dynamic


CC_CXX_WARN_STRICT =

vpath %.c   $(GLMARK2_DIR)/src/glad/src
vpath %.cc  $(GLMARK2_DIR)/src
vpath %.cc  $(GLMARK2_DIR)/src/app/glmark2
vpath %.cc  $(GLMARK2_DIR)/src/libmatrix
vpath %.cc  $(GLMARK2_DIR)/src/scene-ideas
vpath %.cpp $(GLMARK2_DIR)/src
vpath %.cpp $(GLMARK2_DIR)/src/scene-terrain

