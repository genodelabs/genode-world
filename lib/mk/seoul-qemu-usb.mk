include $(call select_from_repositories,lib/mk/qemu-usb.inc)

LIBS  = qemu-usb_include
LIBS += seoul_libc_support format
