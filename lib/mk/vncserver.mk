include $(REP_DIR)/lib/import/import-vncserver.mk

LIBS += libc zlib jpeg libpng

INC_DIR += $(LIBVNC_PORT_DIR)/src/lib/vnc/common
INC_DIR += $(LIBVNC_PORT_DIR)/src/lib/vnc/libvncserver

SRC_C := libvncserver/main.c \
         libvncserver/rfbserver.c \
         libvncserver/rfbregion.c \
         libvncserver/auth.c \
         libvncserver/sockets.c \
         common/sockets.c \
         libvncserver/stats.c \
         libvncserver/corre.c \
         libvncserver/hextile.c \
         libvncserver/rre.c \
         libvncserver/translate.c \
         libvncserver/cutpaste.c \
         libvncserver/httpd.c \
         libvncserver/cursor.c \
         libvncserver/font.c \
         libvncserver/draw.c \
         libvncserver/selbox.c \
         common/d3des.c \
         common/vncauth.c \
         libvncserver/cargs.c \
         common/minilzo.c \
         libvncserver/ultra.c \
         libvncserver/scale.c \
         libvncserver/zlib.c \
         libvncserver/zrle.c \
         libvncserver/zrleoutstream.c \
         libvncserver/zrlepalettehelper.c \
         libvncserver/tight.c \
         common/turbojpeg.c \
         common/crypto_included.c

SHARED_LIB = yes

vpath %.c         $(LIBVNC_PORT_DIR)/src/lib/vnc
