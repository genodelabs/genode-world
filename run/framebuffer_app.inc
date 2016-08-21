#
# This is a skeleton for simple interactive system scenarios that solely rely
# on a framebuffer and input device. The following variables should/can be
# defined prior including this file:
#
# fb_config        - optional XML config to be passed to the fb_drv
# build_components - application-specific build supplements
# boot_modules     - application-specific boot modules
# app_config       - start node(s) of init for running the application
#

if {![info exists fb_config]} {
	set fb_config {<config buffered="yes" width="640" height="480"/>} }


#
# Build
#

append build_components {
	core init
	drivers/timer
	drivers/framebuffer drivers/input
}

lappend_if [have_spec usb] build_components drivers/usb

source ${genode_dir}/repos/base/run/platform_drv.inc

# override default platform driver policy
proc platform_drv_policy {} {
	return {
			<policy label="ps2_drv"> <device name="PS2"/> </policy>
			<policy label="usb_drv"> <pci class="USB"/> </policy>
			<policy label="fb_drv">  <pci class="VGA"/> </policy>
			<policy label="audio_drv"> <pci class="AUDIO"/> <pci class="HDAUDIO"/> </policy>}
}

append_platform_drv_build_components

build $build_components

create_boot_directory


#
# Generate config
#

append config {
<config prio_levels="4">
	<parent-provides>
		<service name="ROM"/>
		<service name="RAM"/>
		<service name="IRQ"/>
		<service name="IO_MEM"/>
		<service name="IO_PORT"/>
		<service name="PD"/>
		<service name="RM"/>
		<service name="CPU"/>
		<service name="LOG"/>
	</parent-provides>
	<default-route>
		<any-service> <parent/> <any-child/> </any-service>
	</default-route>}

append_platform_drv_config

if {[have_spec sdl]} {
append config {
	<start name="fb_sdl">
		<resource name="RAM" quantum="4M"/>
		<provides>
			<service name="Input"/>
			<service name="Framebuffer"/>
		</provides>}
append config $fb_config
append config {
	</start>}
}

if {[have_spec framebuffer]} {
append config {
	<start name="fb_drv" priority="-1">
		<resource name="RAM" quantum="6M"/>
		<resource name="CPU" quantum="10"/>
		<provides><service name="Framebuffer"/></provides>}
append config $fb_config
append config {
	</start>}
}

append_if [have_spec ps2] config {
	<start name="ps2_drv" priority="-1">
		<resource name="RAM" quantum="1M"/>
		<provides><service name="Input"/></provides>
	</start>}

append_if [expr ![have_spec ps2] && [have_spec usb]] config {
	<start name="usb_drv">
		<resource name="RAM" quantum="12M"/>
		<resource name="CPU" quantum="50"/>
		<provides><service name="Input"/></provides>
		<config ehci="yes" uhci="yes" xhci="no"> <hid/> </config>
	</start> }

append config {
	<start name="timer">
		<resource name="RAM" quantum="1M"/>
		<resource name="CPU" quantum="10"/>
		<provides><service name="Timer"/></provides>
	</start>}

append config $app_config

append config {
</config>}

install_config $config


#
# Boot modules
#

# generic modules
append boot_modules { core init timer ld.lib.so }

# platform-specific modules
lappend_if [have_spec        linux] boot_modules fb_sdl
lappend_if [have_spec  framebuffer] boot_modules fb_drv
lappend_if [have_spec          ps2] boot_modules ps2_drv
lappend_if [have_spec          usb] boot_modules usb_drv

append_platform_drv_boot_modules

build_boot_image $boot_modules

append qemu_args " -m 256 "

run_genode_until forever
