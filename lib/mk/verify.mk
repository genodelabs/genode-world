LIBS        = libc
SHARED_LIB  = yes
JDK_BASE    = $(call select_from_ports,jdk)/src/app/jdk/jdk/src/java.base
VERIFY_BASE = $(JDK_BASE)/share/native/libverify

SRC_C += check_format.c check_code.c

include $(REP_DIR)/lib/mk/jdk_version.inc

CC_WARN += -Wno-unused-variable \
           -Wno-unused-function

INC_DIR += $(JDK_BASE)/share/native/include \
           $(JDK_BASE)/share/native/libjava \
           $(JDK_BASE)/unix/native/include \
           $(JDK_BASE)/unix/native/libjava

vpath %.c $(VERIFY_BASE)

# vi: set ft=make :
