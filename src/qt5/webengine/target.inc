TARGET = qt5_webengine.qmake_target

ifeq ($(filter-out $(SPECS),arm_64),)
CC_MARCH = -march=armv8-a+crc+crypto
endif

QT5_PORT_LIBS += libQt5Core libQt5Gui libQt5Network libQt5Widgets
QT5_PORT_LIBS += libQt5Qml libQt5QmlModels libQt5Quick libQt5QuickWidgets
QT5_PORT_LIBS += libQt5WebChannel

LIBS = qt5_qmake egl expat libc libm mesa nss3 stdcxx

INSTALL_LIBS = lib/libQt5WebEngine.lib.so \
               lib/libQt5WebEngineCore.lib.so \
               lib/libQt5WebEngineWidgets.lib.so \
               qml/QtWebEngine/libqtwebengineplugin.lib.so

BUILD_ARTIFACTS += $(notdir $(INSTALL_LIBS)) \
                  qt5_webengine_resources.tar \
                  qt5_webengine_translations.tar \
                  qt5_webengine_qml.tar

QT5_GENODE_LIBS_SHLIB += nss3.lib.so
qmake_prepared.tag: build_dependencies/lib/nss3.lib.so

#
# Limit the number of parallel ninja jobs to 16 by default to keep RAM
# demand below 32 GiB.
#
NINJAJOBS ?= -j$(shell echo -e "$(shell nproc)\n16" | sort -g | head -n1)

build: qmake_prepared.tag qt5_so_files

	@#
	@# run qmake
	@#

	$(VERBOSE)source env.sh && $(QMAKE) \
		-qtconf build_dependencies/mkspecs/$(QT_PLATFORM)/qt.conf \
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

	$(VERBOSE)ln -sf .$(CURDIR)/build_dependencies install/qt

	@#
	@# strip libs and create symlinks in 'bin' and 'debug' directories
	@#

	for LIB in $(INSTALL_LIBS); do \
		cd $(CURDIR)/install/qt/$$(dirname $${LIB}) && \
			$(OBJCOPY) --only-keep-debug $$(basename $${LIB}) $$(basename $${LIB}).debug && \
			$(STRIP) $$(basename $${LIB}) -o $$(basename $${LIB}).stripped && \
			$(OBJCOPY) --add-gnu-debuglink=$$(basename $${LIB}).debug $$(basename $${LIB}).stripped; \
		ln -sf $(CURDIR)/install/qt/$${LIB}.stripped $(PWD)/bin/$$(basename $${LIB}); \
		ln -sf $(CURDIR)/install/qt/$${LIB}.stripped $(PWD)/debug/$$(basename $${LIB}); \
		ln -sf $(CURDIR)/install/qt/$${LIB}.debug $(PWD)/debug/; \
	done

	@#
	@# create tar archives
	@#

	$(VERBOSE)tar chf $(PWD)/bin/qt5_webengine_resources.tar -C install qt/resources
	$(VERBOSE)tar chf $(PWD)/bin/qt5_webengine_translations.tar -C install qt/translations
	$(VERBOSE)tar chf $(PWD)/bin/qt5_webengine_qml.tar --exclude='*.lib.so' --transform='s/\.stripped//' -C install qt/qml

.PHONY: build

QT5_TARGET_DEPS = build
