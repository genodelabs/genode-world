include $(call select_from_repositories,lib/import/import-qt5_qmake.mk)

QT5_PORT_LIBS += libQt5Core libQt5Gui libQt5Network
QT5_PORT_LIBS += libQt5Qml libQt5Quick

LIBS = egl expat libc libm mesa qt5_component stdcxx $(QT5_PORT_LIBS)

built.tag: qmake_prepared.tag

	@#
	@# run qmake
	@#

	$(VERBOSE)source env.sh && $(QMAKE) \
		-qtconf qmake_root/mkspecs/$(QMAKE_PLATFORM)/qt.conf \
		$(QT_DIR)/qtwebchannel/qtwebchannel.pro \
		$(QT5_OUTPUT_FILTER)

	@#
	@# build
	@#

	$(VERBOSE)source env.sh && $(MAKE) sub-src $(QT5_OUTPUT_FILTER)

	@#
	@# install into local 'install' directory
	@#

	$(VERBOSE)$(MAKE) INSTALL_ROOT=$(CURDIR)/install sub-src-install_subtargets $(QT5_OUTPUT_FILTER)

	$(VERBOSE)ln -sf .$(CURDIR)/qmake_root install/qt

	@#
	@# create stripped versions
	@#

	$(VERBOSE)cd $(CURDIR)/install$(CURDIR)/qmake_root/lib && \
		$(STRIP) libQt5WebChannel.lib.so -o libQt5WebChannel.lib.so.stripped

	@#
	@# create symlinks in 'bin' directory
	@#

	$(VERBOSE)ln -sf $(CURDIR)/install/qt/lib/libQt5WebChannel.lib.so.stripped $(PWD)/bin/libQt5WebChannel.lib.so

	@#
	@# create symlinks in 'debug' directory
	@#

	$(VERBOSE)ln -sf $(CURDIR)/install/qt/lib/libQt5WebChannel.lib.so $(PWD)/debug/

	@#
	@# mark as done
	@#

	$(VERBOSE)touch $@


ifeq ($(called_from_lib_mk),yes)
all: built.tag
endif
