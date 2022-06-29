ifeq ($(CONTRIB_DIR),)
UBUNTU_UI_TOOLKIT_PORT_DIR := $(call select_from_repositories,src/lib/ubuntu-ui-toolkit)
UBUNTU_UI_TOOLKIT_API_DIR = $(call select_from_repositories,include/UbuntuToolkit)/../..
else
UBUNTU_UI_TOOLKIT_PORT_DIR := $(call select_from_ports,ubuntu-ui-toolkit)/src/lib/ubuntu-ui-toolkit
UBUNTU_UI_TOOLKIT_API_DIR  = $(UBUNTU_UI_TOOLKIT_PORT_DIR)/genode/api
endif

qmake_root/include/UbuntuMetrics: qmake_root/include
	$(VERBOSE)ln -sf $(UBUNTU_UI_TOOLKIT_API_DIR)/include/$(notdir $@) $@

qmake_root/include/UbuntuToolkit: qmake_root/include
	$(VERBOSE)ln -sf $(UBUNTU_UI_TOOLKIT_API_DIR)/include/$(notdir $@) $@

qmake_root/lib/libUbuntu%.lib.so: qmake_root/lib
ifeq ($(CONTRIB_DIR),)
	$(VERBOSE)ln -sf $(BUILD_BASE_DIR)/var/libcache/$(basename $(basename $(notdir $@)))/$(basename $(basename $(notdir $@))).abi.so $@
else
	$(VERBOSE)ln -sf $(BUILD_BASE_DIR)/bin/$(notdir $@) $@
endif

qmake_root/mkspecs/modules/qt_lib_Ubuntu%.pri: qmake_root/mkspecs
	$(VERBOSE)ln -sf $(UBUNTU_UI_TOOLKIT_API_DIR)/mkspecs/modules/$(notdir $@) $@

qmake_prepared.tag: \
                    qmake_root/include/UbuntuMetrics \
                    qmake_root/include/UbuntuToolkit \
                    qmake_root/lib/libUbuntuGestures.lib.so \
                    qmake_root/lib/libUbuntuMetrics.lib.so \
                    qmake_root/lib/libUbuntuToolkit.lib.so \
                    qmake_root/mkspecs/modules/qt_lib_UbuntuGestures.pri \
                    qmake_root/mkspecs/modules/qt_lib_UbuntuGestures_private.pri \
                    qmake_root/mkspecs/modules/qt_lib_UbuntuMetrics.pri \
                    qmake_root/mkspecs/modules/qt_lib_UbuntuToolkit_private.pri \
                    qmake_root/mkspecs/modules/qt_lib_UbuntuToolkit.pri
