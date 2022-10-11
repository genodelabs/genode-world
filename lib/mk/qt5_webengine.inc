include $(call select_from_repositories,lib/import/import-qt5_qmake.mk)

ifeq ($(filter-out $(SPECS),arm_64),)
GENODE_QMAKE_CFLAGS += -march=armv8-a+crc+crypto
endif

QT5_PORT_LIBS += libQt5Core libQt5Gui libQt5Network libQt5Widgets
QT5_PORT_LIBS += libQt5Qml libQt5QmlModels libQt5Quick libQt5QuickWidgets
QT5_PORT_LIBS += libQt5WebChannel

LIBS = egl expat libc libm mesa nss3 qt5_component stdcxx $(QT5_PORT_LIBS)

QT5_GENODE_LIBS_SHLIB += nss3.lib.so
qmake_prepared.tag: qmake_root/lib/nss3.lib.so

#
# Limit the number of parallel ninja jobs to 16 by default to keep RAM
# demand below 32 GiB.
#
NINJAJOBS ?= -j$(shell echo -e "$(shell nproc)\n16" | sort -g | head -n1)

built.tag: qmake_prepared.tag

	@#
	@# run qmake
	@#

	$(VERBOSE)source env.sh && $(QMAKE) \
		-qtconf qmake_root/mkspecs/$(QMAKE_PLATFORM)/qt.conf \
		$(QT_DIR)/qtwebengine/qtwebengine.pro \
		-- \
		-no-feature-webengine-system-ninja \
		-no-webengine-alsa \
		-no-webengine-embedded-build \
		-qt-webengine-icu \
		-qt-webengine-ffmpeg \
		-qt-webengine-opus \
		-qt-webengine-webp \
		-no-webengine-pepper-plugins \
		-no-webengine-printing-and-pdf \
		-no-webengine-pulseaudio \
		-no-webengine-sndio \
		-no-webengine-spellchecker \
		-no-webengine-native-spellchecker \
		-no-webengine-extensions \
		-no-webengine-geolocation \
		-no-webengine-kerberos \
		-webengine-proprietary-codecs \
		-webengine-webrtc \
		-webengine-widgets \
		-webengine-qml \
		$(QT5_OUTPUT_FILTER)

	@#
	@# build
	@#

	$(VERBOSE)source env.sh && $(MAKE) sub-src NINJAJOBS=$(NINJAJOBS) $(QT5_OUTPUT_FILTER)

	@#
	@# install into local 'install' directory
	@#

	$(VERBOSE)$(MAKE) INSTALL_ROOT=$(CURDIR)/install sub-src-install_subtargets $(QT5_OUTPUT_FILTER)

	$(VERBOSE)ln -sf .$(CURDIR)/qmake_root install/qt

	@#
	@# create stripped versions
	@#

	$(VERBOSE)cd $(CURDIR)/install$(CURDIR)/qmake_root/lib && \
		$(STRIP) libQt5WebEngine.lib.so -o libQt5WebEngine.lib.so.stripped && \
		$(STRIP) libQt5WebEngineCore.lib.so -o libQt5WebEngineCore.lib.so.stripped && \
		$(STRIP) libQt5WebEngineWidgets.lib.so -o libQt5WebEngineWidgets.lib.so.stripped

	$(VERBOSE)cd $(CURDIR)/install/qt/qml/QtWebEngine && \
		$(STRIP) libqtwebengineplugin.lib.so -o libqtwebengineplugin.lib.so.stripped

	@#
	@# create symlinks in 'bin' directory
	@#

	$(VERBOSE)ln -sf $(CURDIR)/install/qt/lib/libQt5WebEngine.lib.so.stripped $(PWD)/bin/libQt5WebEngine.lib.so
	$(VERBOSE)ln -sf $(CURDIR)/install/qt/lib/libQt5WebEngineCore.lib.so.stripped $(PWD)/bin/libQt5WebEngineCore.lib.so
	$(VERBOSE)ln -sf $(CURDIR)/install/qt/lib/libQt5WebEngineWidgets.lib.so.stripped $(PWD)/bin/libQt5WebEngineWidgets.lib.so

	$(VERBOSE)ln -sf $(CURDIR)/install/qt/qml/QtWebEngine/libqtwebengineplugin.lib.so.stripped $(PWD)/bin/libqtwebengineplugin.lib.so

	@#
	@# create symlinks in 'debug' directory
	@#

	$(VERBOSE)ln -sf $(CURDIR)/install/qt/lib/libQt5WebEngine.lib.so $(PWD)/debug/
	$(VERBOSE)ln -sf $(CURDIR)/install/qt/lib/libQt5WebEngineCore.lib.so $(PWD)/debug/
	$(VERBOSE)ln -sf $(CURDIR)/install/qt/lib/libQt5WebEngineWidgets.lib.so $(PWD)/debug/

	$(VERBOSE)ln -sf $(CURDIR)/install/qt/qml/QtWebEngine/libqtwebengineplugin.lib.so $(PWD)/debug/

	@#
	@# create tar archives
	@#

	$(VERBOSE)tar chf $(PWD)/bin/qt5_webengine_resources.tar -C install qt/resources
	$(VERBOSE)tar chf $(PWD)/bin/qt5_webengine_translations.tar -C install qt/translations
	$(VERBOSE)tar chf $(PWD)/bin/qt5_webengine_qml.tar --exclude='*.lib.so' --transform='s/\.stripped//' -C install qt/qml

	@#
	@# mark as done
	@#

	$(VERBOSE)touch $@


ifeq ($(called_from_lib_mk),yes)
all: built.tag
endif
