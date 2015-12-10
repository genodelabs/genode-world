include $(call select_from_repositories,lib/import/import-testu01.mk)

TESTU01_DIR = $(call select_from_ports,testu01)
TESTU01_SRC_DIR = $(TESTU01_DIR)/src/lib/testu01/TestU01-1.2.3

LIBS += libc

CC_WARN += -Wno-unused-variable -Wno-maybe-uninitialized -Wno-return-type

TESTU01_SRC_C = unif01.c ulcg.c umrg.c ucarry.c utaus.c ugfsr.c uinv.c uquad.c ucubic.c ulec.c utezuka.c umarsa.c uweyl.c uknuth.c uwu.c unumrec.c uvaria.c usoft.c ugranger.c ucrypto.c ufile.c udeng.c utouzin.c uautomata.c uxorshift.c ubrent.c rijndael-alg-fst.c tu01_sha1.c scatter.c swrite.c sres.c smultin.c sknuth.c smarsa.c sstring.c svaria.c snpair.c swalk.c sentrop.c sspectral.c scomp.c sspacings.c vectorsF2.c bbattery.c ffam.c fcong.c ffsr.c ftab.c fres.c fcho.c fmultin.c fmarsa.c fknuth.c fwalk.c fstring.c fspectral.c fvaria.c fnpair.c
PROBDIST_SRC_C = fmass.c fdist.c fbar.c finv.c gofs.c gofw.c statcoll.c wdist.c
MYLIB_SRC_C = addstr.c bitset.c mystr.c num.c num2.c tables.c util.c

EXAMPLES_SRC_C = mrg32k3a.c xorshift.c

SRC_C = $(TESTU01_SRC_C) $(PROBDIST_SRC_C) $(MYLIB_SRC_C) $(EXAMPLES_SRC_C)

SRC_CC = genode_chrono.cc genode_gdef.cc

vpath %.cc $(REP_DIR)/src/lib/testu01
vpath %.c $(TESTU01_SRC_DIR)/testu01
vpath %.c $(TESTU01_SRC_DIR)/probdist
vpath %.c $(TESTU01_SRC_DIR)/mylib
vpath %.c $(TESTU01_SRC_DIR)/examples
