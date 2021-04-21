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

create_boot_directory

lappend import [depot_user]/src/[base_src] \
               [depot_user]/pkg/[drivers_interactive_pkg] \
               [depot_user]/src/init \
               [depot_user]/src/nitpicker \
               [depot_user]/pkg/motif_wm

import_from_depot $import

if {![info exists fb_config]} {
	set fb_config {<config buffered="yes" width="640" height="480"/>} }

set fd [open [run_dir]/genode/fb_drv.config w]
puts $fd $fb_config
close $fd


install_config {
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
		<service name="Gui">
			<child name="wm"/> </service>
		<any-service> <parent/> <any-child/> </any-service>
	</default-route>

	<default caps="100"/>

	<start name="drivers" caps="1000" managing_system="yes">
		<resource name="RAM" quantum="64M"/>
		<binary name="init"/>
		<route>
			<service name="ROM" label="config"> <parent label="drivers.config"/> </service>
			<service name="Timer">   <child name="timer"/> </service>
			<service name="Capture"> <child name="nitpicker"/> </service>
			<service name="Event">   <child name="nitpicker"/> </service>
			<any-service> <parent/> </any-service>
		</route>
		<provides> <service name="Input"/> </provides>
	</start>

	<start name="timer">
		<resource name="RAM" quantum="1M"/>
		<resource name="CPU" quantum="10"/>
		<provides><service name="Timer"/></provides>
	</start>

	<start name="nitpicker">
		<resource name="RAM" quantum="4M"/>
		<provides> <service name="Gui"/> <service name="Capture"/> <service name="Event"/> </provides>
		<config focus="rom" request_framebuffer="no">
			<capture/>
			<event/>

			<domain name="pointer" layer="1" content="client" label="no" origin="pointer" />
			<domain name="default" layer="2" content="client" label="no" hover="always" width="1024" height="768"/>

			<policy label_prefix="pointer" domain="pointer"/>
			<default-policy domain="default"/>
		</config>
	</start>

	<start name="pointer">
		<resource name="RAM" quantum="1M"/>
		<route>
			<service name="Gui"> <child name="nitpicker"/> </service>
			<any-service> <parent/> <any-child/> </any-service>
		</route>
	</start>

	<start name="wm" caps="1000">
		<resource name="RAM" quantum="32M"/>
		<binary name="init"/>
		<provides> <service name="Gui"/> </provides>
		<route>
			<service name="ROM" label="config"> <parent label="wm.config"/> </service>
			<service name="Gui"> <child name="nitpicker"/> </service>
			<any-service> <parent/> <any-child/> </any-service>
		</route>
	</start>

} $app_config {

</config>}

if {[llength $build_components] > 0} {
	build $build_components
}
#
# Pin the nitpicker focus to the window manager by providing a static focus ROM
#
set fd [open [run_dir]/genode/focus w]
puts $fd "<focus label=\"wm -> focus\"/>"
close $fd

file copy -force [genode_dir]/repos/gems/recipes/raw/motif_wm/wm.config [run_dir]/genode/

build_boot_image $boot_modules

run_genode_until forever
