TARGET = ubuntu_ui_extras.cmake_target

UBUNTU_UI_EXTRAS_PORT_DIR = $(call select_from_ports,ubuntu-ui-extras)/src/lib/ubuntu-ui-extras

CMAKE_LISTS_DIR = $(UBUNTU_UI_EXTRAS_PORT_DIR)

CMAKE_TARGET_BINARIES = \
	install/usr/local/lib/qt5/qml/Ubuntu/Components/Extras/libubuntu-ui-extras-plugin.lib.so

QT5_PORT_LIBS = libQt5Core libQt5Gui libQt5Widgets libQt5Network libQt5Test libQt5Xml
QT5_PORT_LIBS += libQt5Qml libQt5QmlModels libQt5Quick

LIBS = qt5_cmake libc libm mesa qt5_component stdcxx

build: build_with_cmake qt5_so_files
	$(VERBOSE)ln -snf qt5 install/usr/local/lib/qt
	$(VERBOSE)tar cf $(BUILD_BASE_DIR)/bin/ubuntu-ui-extras_qml.tar --exclude='*.lib.so' --transform='s/\.stripped//' -C install/usr/local/lib qt/qml

.PHONY: build

BUILD_ARTIFACTS += ubuntu-ui-extras_qml.tar

QT5_TARGET_DEPS = build
