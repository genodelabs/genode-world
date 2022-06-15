LIBGO_DIR := $(call select_from_ports,libgo)/src/lib/libgo
# LIBATOMIC_DIR := $(call select_from_ports,libgo)/src/lib/libatomic
# LIBBACKTRACE_DIR := $(call select_from_ports,libgo)/src/lib/libbacktrace
# LIBFFI_DIR := $(call select_from_ports,libgo)/src/lib/libffi

LIBS += libc libatomic libbacktrace libffi
