FESRV_SRC = $(call select_from_ports,fesrv)/src/app/fesrv/fesvr/

SRC_CC = main.cc                 \
         linux_syscalls.cc       \
         context.cc              \
         device.cc               \
         dummy.cc                \
         elfloader.cc            \
         htif.cc                 \
         htif_eth.cc             \
         htif_hexwriter.cc       \
         htif_rs232.cc           \
         htif_zedboard.cc        \
         memif.cc                \
         option_parser.cc        \
         packet.cc               \
         rfb.cc                  \
         syscall.cc              \
         term.cc

TARGET   = fesrv
LIBS     = stdcxx posix
INC_DIR += $(FESRV_SRC)

# Some defines to keep the fesrv compilation process happy
CC_OPT      += -DTARGET_ARCH=\"\" -DTARGET_DIR=\"\"
CC_OPT_htif += -DPREFIX=\"\"

vpath %.cc $(FESRV_SRC)


