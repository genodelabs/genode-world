ifeq ($(CONTRIB_DIR),)
UBUNTU_UI_TOOLKIT_PORT_DIR := $(call select_from_repositories,src/lib/ubuntu-ui-toolkit)
UBUNTU_UI_TOOLKIT_API_DIR = $(call select_from_repositories,include/UbuntuToolkit)/../..
else
UBUNTU_UI_TOOLKIT_PORT_DIR := $(call select_from_ports,ubuntu-ui-toolkit)/src/lib/ubuntu-ui-toolkit
UBUNTU_UI_TOOLKIT_API_DIR  = $(UBUNTU_UI_TOOLKIT_PORT_DIR)/genode/api
endif

build_dependencies/include/UbuntuMetrics: build_dependencies/include
	$(VERBOSE)ln -sf $(UBUNTU_UI_TOOLKIT_API_DIR)/include/$(notdir $@) $@

build_dependencies/include/UbuntuToolkit: build_dependencies/include
	$(VERBOSE)ln -sf $(UBUNTU_UI_TOOLKIT_API_DIR)/include/$(notdir $@) $@

build_dependencies/lib/libUbuntu%.lib.so: build_dependencies/lib
ifeq ($(CONTRIB_DIR),)
	$(VERBOSE)ln -sf $(BUILD_BASE_DIR)/var/libcache/$(basename $(basename $(notdir $@)))/$(basename $(basename $(notdir $@))).abi.so $@
else
	$(VERBOSE)ln -sf $(BUILD_BASE_DIR)/bin/$(notdir $@) $@
endif

build_dependencies/mkspecs/modules/qt_lib_Ubuntu%.pri: build_dependencies/mkspecs
	$(VERBOSE)ln -sf $(UBUNTU_UI_TOOLKIT_API_DIR)/mkspecs/modules/$(notdir $@) $@

qmake_prepared.tag: \
                    build_dependencies/include/UbuntuMetrics \
                    build_dependencies/include/UbuntuToolkit \
                    build_dependencies/lib/libUbuntuGestures.lib.so \
                    build_dependencies/lib/libUbuntuMetrics.lib.so \
                    build_dependencies/lib/libUbuntuToolkit.lib.so \
                    build_dependencies/mkspecs/modules/qt_lib_UbuntuGestures.pri \
                    build_dependencies/mkspecs/modules/qt_lib_UbuntuGestures_private.pri \
                    build_dependencies/mkspecs/modules/qt_lib_UbuntuMetrics.pri \
                    build_dependencies/mkspecs/modules/qt_lib_UbuntuToolkit_private.pri \
                    build_dependencies/mkspecs/modules/qt_lib_UbuntuToolkit.pri
