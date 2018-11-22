LIBS          = libc
SHARED_LIB    = yes
NATIVE_BASE   = $(call select_from_ports,jdk)/src/app/jdk/jdk/src/java.management/share/native/
JDK_BASE      = $(call select_from_ports,jdk)/src/app/jdk/jdk/src/java.base
JDK_GENERATED = $(call select_from_ports,jdk_generated)/src/app/jdk

SRC_C = ClassLoadingImpl.c \
        GarbageCollectorImpl.c \
        HotspotThread.c \
        management.c \
        MemoryImpl.c \
        MemoryManagerImpl.c \
        MemoryPoolImpl.c \
        ThreadImpl.c \
        VMManagementImpl.c

INC_DIR += $(JDK_GENERATED)/include/java.management \
           $(JDK_BASE)/share/native/include \
           $(JDK_BASE)/share/native/libjava \
           $(JDK_BASE)/unix/native/include \
           $(JDK_BASE)/unix/native/libjava \
           $(NATIVE_BASE)/include

vpath %c $(NATIVE_BASE)/libmanagement
