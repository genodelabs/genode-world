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

import_from_depot genodelabs/src/[base_src] \
                  genodelabs/pkg/[drivers_interactive_pkg] \
                  genodelabs/src/init


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
		<any-service> <parent/> <any-child/> </any-service>
	</default-route>

	<default caps="100"/>

	<start name="drivers" caps="1000">
		<resource name="RAM" quantum="32M" constrain_phys="yes"/>
		<binary name="init"/>
		<route>
			<service name="ROM" label="config"> <parent label="drivers.config"/> </service>
			<service name="Timer"> <child name="timer"/> </service>
			<any-service> <parent/> </any-service>
		</route>
		<provides>
			<service name="Input"/> <service name="Framebuffer"/>
		</provides>
	</start>

	<start name="timer">
		<resource name="RAM" quantum="1M"/>
		<resource name="CPU" quantum="10"/>
		<provides><service name="Timer"/></provides>
	</start>

} $app_config {

</config>}

build $build_components

build_boot_image $boot_modules

run_genode_until forever
