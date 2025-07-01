TARGET = qt6_webengine.cmake_target

ifeq ($(CONTRIB_DIR),)
QT6_WEBENGINE_DIR       = $(call select_from_repositories,src/lib/qt6_webengine)
else
QT6_WEBENGINE_PORT_DIR := $(call select_from_ports,qt6_webengine)
QT6_WEBENGINE_DIR       = $(QT6_WEBENGINE_PORT_DIR)/src/lib/qt6_webengine
endif

QT6_PORT_LIBS  = libQt6Core libQt6Gui libQt6Network libQt6OpenGL libQt6Widgets
QT6_PORT_LIBS += libQt6Qml libQt6QmlModels libQt6Quick libQt6QuickWidgets
QT6_PORT_LIBS += libQt6WebChannel libQt6WebChannelQuick

# avoid error about missing TLS init function for 'thread_local' variables
GENODE_CMAKE_CFLAGS += -fno-extern-tls-init

LIBS = qt6_cmake ldso_so_support libc libm egl mesa qt6_component stdcxx nss3 expat

INSTALL_LIBS = lib/libQt6WebEngineCore.lib.so \
               lib/libQt6WebEngineQuick.lib.so \
               lib/libQt6WebEngineQuickDelegatesQml.lib.so \
               lib/libQt6WebEngineWidgets.lib.so \
               qml/QtWebEngine/libqtwebenginequickplugin.lib.so

BUILD_ARTIFACTS = $(notdir $(INSTALL_LIBS)) \
                  qt6_webengine_resources.tar \
                  qt6_webengine_translations.tar \
                  qt6_webengine_qml.tar

cmake_prepared.tag: $(addprefix build_dependencies/lib/,expat.lib.so nss3.lib.so)

ifeq ($(filter-out $(SPECS),arm_64),)
GENODE_CMAKE_SYSTEM_PROCESSOR="aarch64"
else ifeq ($(filter-out $(SPECS),x86_64),)
GENODE_CMAKE_SYSTEM_PROCESSOR="x86_64"
endif

#
# Limit the number of parallel ninja jobs to 16 by default to keep RAM
# demand below 32 GiB.
#
export NINJAFLAGS ?= -j$(shell echo -e "$(shell nproc)\n16" | sort -g | head -n1)

build: cmake_prepared.tag qt6_so_files

	@#
	@# run cmake
	@#

	@#
	@# -DFEATURE_webengine_embedded_build=OFF
	@#  when cross-compiling, this feature is on by default
	@#  and creates a scrollbar which does not work correctly
	@#  with a mouse
	@#

	$(VERBOSE)cmake \
		-G "Unix Makefiles" \
		-DQT_HOST_PATH="$(QT_TOOLS_DIR)" \
		-DCMAKE_PREFIX_PATH="$(CURDIR)/build_dependencies" \
		-DCMAKE_MODULE_PATH="$(CURDIR)/build_dependencies/lib/cmake/Modules" \
		-DCMAKE_TOOLCHAIN_FILE="$(CURDIR)/build_dependencies/lib/cmake/Modules/Platform/Genode.cmake" \
		-DCMAKE_SYSTEM_NAME="FreeBSD" \
		-DCMAKE_SYSTEM_PROCESSOR="$(GENODE_CMAKE_SYSTEM_PROCESSOR)" \
		-DTEST_architecture_arch="$(GENODE_CMAKE_SYSTEM_PROCESSOR)" \
		-DCMAKE_AR="$(AR)" \
		-DCMAKE_C_COMPILER="$(CC)" \
		-DCMAKE_C_FLAGS="$(GENODE_CMAKE_CFLAGS)" \
		-DCMAKE_CXX_COMPILER="$(CXX)" \
		-DCMAKE_CXX_FLAGS="$(GENODE_CMAKE_CFLAGS)" \
		-DCMAKE_EXE_LINKER_FLAGS="$(GENODE_CMAKE_LFLAGS_APP)" \
		-DCMAKE_SHARED_LINKER_FLAGS="$(GENODE_CMAKE_LFLAGS_SHLIB)" \
		-DCMAKE_MODULE_LINKER_FLAGS="$(GENODE_CMAKE_LFLAGS_SHLIB)" \
		-DQT_QMAKE_TARGET_MKSPEC=$(QT_PLATFORM) \
		-DCMAKE_INSTALL_PREFIX=/qt \
		-DFEATURE_webengine_embedded_build=OFF \
		-DFEATURE_qtpdf_build=OFF \
		$(QT6_WEBENGINE_DIR) \
		$(QT6_OUTPUT_FILTER)

	@#
	@# build
	@#

	$(VERBOSE)$(MAKE) VERBOSE=$(MAKE_VERBOSE)

	@#
	@# install into local 'install' directory
	@#

	$(VERBOSE)$(MAKE) VERBOSE=$(MAKE_VERBOSE) DESTDIR=install install

	@#
	@# remove shared library existence checks since many libs are not
	@# present and not needed at build time
	@#

	$(VERBOSE)find $(CURDIR)/install/qt/lib/cmake -name "*.cmake" \
	          -exec sed -i "/list(APPEND _IMPORT_CHECK_TARGETS /d" {} \;

	@#
	@# strip libs and create symlinks in 'bin' and 'debug' directories
	@#

	$(VERBOSE)for LIB in $(INSTALL_LIBS); do \
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

	$(VERBOSE)tar chf $(PWD)/bin/qt6_webengine_resources.tar -C install qt/resources
	$(VERBOSE)tar chf $(PWD)/bin/qt6_webengine_translations.tar -C install qt/translations
	$(VERBOSE)tar chf $(PWD)/bin/qt6_webengine_qml.tar --exclude='*.lib.so' --transform='s/\.stripped//' -C install qt/qml

.PHONY: build

QT6_TARGET_DEPS = build
