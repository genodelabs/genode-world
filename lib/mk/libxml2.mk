LIBXML2_DIR := $(call select_from_ports,libxml2)/src/lib/libxml2

INC_DIR += $(LIBXML2_DIR)/include $(REP_DIR)/src/lib/libxml2

LIBS += libc

SRC_C := buf.c \
         c14n.c \
         catalog.c \
         chvalid.c \
         debugXML.c \
         dict.c \
         encoding.c \
         entities.c \
         error.c \
         globals.c \
         hash.c \
         HTMLparser.c \
         HTMLtree.c \
         legacy.c \
         list.c \
         nanoftp.c \
         nanohttp.c \
         parserInternals.c \
         parser.c \
         pattern.c \
         relaxng.c \
         SAX2.c \
         SAX.c \
         schematron.c \
         threads.c \
         tree.c \
         uri.c \
         valid.c \
         xinclude.c \
         xlink.c \
         xmlIO.c \
         xmlmemory.c \
         xmlmodule.c \
         xmlreader.c \
         xmlregexp.c \
         xmlsave.c \
         xmlschemas.c \
         xmlschemastypes.c \
         xmlstring.c \
         xmlunicode.c \
         xmlwriter.c \
         xpath.c \
         xpointer.c \
         xzlib.c

CC_WARN := -Wall -Wno-implicit-function-declaration -Wno-unused-function \
           -Wno-unused-but-set-variable -Wno-format-extra-args -Wno-format

vpath %.c $(LIBXML2_DIR)
