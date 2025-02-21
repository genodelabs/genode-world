FALKON_DIR = $(call select_from_ports,falkon)/src/app/falkon

CMAKE_LISTS_DIR = $(FALKON_DIR)

CMAKE_TARGET_BINARIES = bin/falkon bin/libFalkonPrivate.lib.so

QT6_PORT_LIBS += libQt6Core libQt6Gui libQt6Network libQt6OpenGL libQt6PrintSupport libQt6Sql libQt6Widgets
QT6_PORT_LIBS += libQt6Qml libQt6QmlModels libQt6Quick libQt6QuickWidgets
QT6_PORT_LIBS += libQt6Svg
QT6_PORT_LIBS += libQt6WebChannel
QT6_PORT_LIBS += libQt6WebEngineCore libQt6WebEngineWidgets
QT6_PORT_LIBS += libQt6Core5Compat

LIBS = qt6_cmake egl libc libm mesa qt6_component stdcxx nss3 libcrypto

QT6_GENODE_LIBS_APP += libcrypto.lib.so nss3.lib.so

cmake_prepared.tag: $(addprefix build_dependencies/lib/,$(QT6_GENODE_LIBS_APP))

GENODE_CMAKE_CFLAGS += -I$(FALKON_DIR)/src/lib -I$(CURDIR)/src/lib
QT6_EXTRA_CMAKE_ARGS = -DQT_MAJOR_VERSION=6 -DCMAKE_AUTOMOC=ON -DDISABLE_DBUS=ON -DNO_X11=ON

$(BUILD_BASE_DIR)/bin/falkon_theme.tar:
	$(VERBOSE)tar cf $@ --transform='s/themes/.local\/share\/falkon\/themes/' -C $(FALKON_DIR) themes/chrome

QT6_EXTRA_TARGET_DEPS = $(BUILD_BASE_DIR)/bin/falkon_theme.tar

BUILD_ARTIFACTS += falkon_theme.tar
