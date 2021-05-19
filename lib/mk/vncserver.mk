include $(REP_DIR)/lib/import/import-vncserver.mk

LIBS += libc zlib jpeg libpng

INC_DIR += $(LIBVNC_PORT_DIR)/src/lib/vnc/common
INC_DIR += $(LIBVNC_PORT_DIR)/src/lib/vnc/libvncserver

SRC_C := main.c \
         rfbserver.c \
         rfbregion.c \
         auth.c \
         sockets.c \
         stats.c \
         corre.c \
         hextile.c \
         rre.c \
         translate.c \
         cutpaste.c \
         httpd.c \
         cursor.c \
         font.c \
         draw.c \
         selbox.c \
         d3des.c \
         vncauth.c \
         cargs.c \
         minilzo.c \
         ultra.c \
         scale.c \
         zlib.c \
         zrle.c \
         zrleoutstream.c \
         zrlepalettehelper.c \
         tight.c \
         turbojpeg.c

SHARED_LIB = yes

vpath minilzo.c   $(LIBVNC_PORT_DIR)/src/lib/vnc/common
vpath d3des.c     $(LIBVNC_PORT_DIR)/src/lib/vnc/common
vpath vncauth.c   $(LIBVNC_PORT_DIR)/src/lib/vnc/common
vpath turbojpeg.c $(LIBVNC_PORT_DIR)/src/lib/vnc/common
vpath %.c         $(LIBVNC_PORT_DIR)/src/lib/vnc/libvncserver
