include $(REP_DIR)/lib/mk/ada.inc

ADALIB     = $(ADA_RTS)/adalib
ADAINCLUDE = $(ADA_RTS)/adainclude

SRC_C += \
		init.c \
		exit.c \
		argv.c \
		posix_common.c \
		posix_fat.c

SRC_ADS += system.ads \
		s-soflin.ads \
		s-imgint.ads \
		s-stoele.ads \
		s-secsta.ads \
		interfac.ads \
		a-uncdea.ads \
		a-ioexce.ads \
		interfac.ads \
		s-crtl.ads \
		s-stalib.ads \
		gnat.ads \
		ada.ads \
		g-souinf.ads \
		g-trasym.ads \
		s-unstyp.ads

SRC_ADB += g-io.adb \
	   a-except.adb \
		a-tags.adb \
		a-finali.adb \
		s-htable.adb \
		s-wchcon.adb \
		s-wchstw.adb \
		s-valllu.adb \
		s-strhas.adb \
		s-valuti.adb \
		s-wchcnv.adb \
		s-wchjis.adb \
		s-casuti.adb \
		s-exctab.adb \
		s-finmas.adb \
		s-addima.adb \
		s-io.adb \
		s-finroo.adb \
		s-stopoo.adb \
		s-imgboo.adb \
		s-pooglo.adb \
		s-stratt.adb \
		i-c.adb \
		s-memory.adb \
		a-stream.adb \
		s-parame.adb \
		a-elchha.adb \
	   s-stache.adb \
		s-excdeb.adb \
		s-traent.adb \
		s-traceb.adb \
		g-traceb.adb \
		a-exctra.adb \
		s-trasym.adb \
		s-except.adb \
		a-comlin.adb \
		s-init.adb \
		s-arit64.adb \
		a-calend.adb \
		s-imglli.adb \
		s-osprim.adb \
		s-conca2.adb \
		s-conca3.adb \
		s-conca4.adb \
		s-conca5.adb \
		s-conca6.adb \
		s-conca7.adb \
		s-conca8.adb \
		s-conca9.adb \

# Ada packages that implement runtime functionality
SRC_ADB += \
		ss_utils.adb \
		string_utils.adb \
		platform.adb

vpath platform.% $(ADA_RUNTIME_LIB_DIR)
vpath string_utils.% $(ADA_RUNTIME_LIB_DIR)
vpath ss_utils.% $(ADA_RUNTIME_LIB_DIR)
vpath init.c $(ADA_RUNTIME_LIB_DIR)
vpath exit.c $(ADA_RUNTIME_LIB_DIR)
vpath argv.c $(ADA_RUNTIME_LIB_DIR)
vpath s-init.adb $(ADA_RUNTIME_COMMON_DIR)
vpath platform.% $(ADA_RUNTIME_LIB_DIR)
vpath string_utils.% $(ADA_RUNTIME_LIB_DIR)
vpath ss_utils.% $(ADA_RUNTIME_LIB_DIR)
vpath a-except.adb $(ADA_RUNTIME_DIR)

vpath %.c   $(ADA_RUNTIME_PLATFORM_DIR)
vpath %.ads $(ADA_RTS_SOURCE)
vpath %.adb $(ADA_RTS_SOURCE)

SHARED_LIB = yes
LIBS += libc
CUSTOM_ADA_FLAGS = --RTS=$(ADA_RTS) -c -gnatg -gnatp -gnatpg -gnatn2

