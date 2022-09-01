
if {[have_board linux] && $use_i965} {
	puts "i965 driver not supported on Linux."
	exit 1
}

if {[have_include power_on/qemu] && $use_i965} {
	puts "i965 driver not supported in Qemu."
	exit 1
}


set build_components {
	core init
	timer
	drivers/input
	drivers/framebuffer
	drivers/gpu/intel
	server/liquid_framebuffer
	server/nitpicker
	server/input_event_bridge
	lib/mesa_softpipe
	app/launchpad
	app/pointer
}

lappend_if $use_i965 build_components lib/mesa_iris

lappend build_components $demo_component

source ${genode_dir}/repos/base/run/platform_drv.inc
append_platform_drv_build_components

build $build_components

create_boot_directory

set config {
<config prio_levels="2">
	<parent-provides>
		<service name="CPU"/>
		<service name="IRQ"/>
		<service name="IO_MEM"/>
		<service name="IO_PORT"/>
		<service name="LOG"/>
		<service name="PD"/>
		<service name="RM"/>
		<service name="ROM"/>
	</parent-provides>
	<default caps="100"/>
	<default-route>
		<any-service> <parent/> <any-child/> </any-service>
	</default-route>
}

append_if [have_board linux] config {
	<start name="fb_sdl" caps="200" ld="no">
		<resource name="RAM" quantum="4M"/>
		<provides> <service name="Input"/> </provides>
	</start>
	<start name="input_event_bridge">
		<resource name="RAM" quantum="1M"/>
		<provides>
			<service name="Event"/>
			<service name="Input"/>
		</provides>
		<config/>
		<route>
			<any-service> <parent/> </any-service>
		</route>
	</start>
	<alias name="input_drv" child="input_event_bridge"/> }

append_platform_drv_config

append_if [expr ![have_board linux]] config {
	<start name="fb_drv">
		<resource name="RAM" quantum="10M"/>
		<route><any-service> <parent /> <any-child/> </any-service> </route>
	</start>}

append_if [have_board pc] config {
	<start name="ps2_drv">
		<resource name="RAM" quantum="2M"/>
		<provides> <service name="Input"/> </provides>
	</start>
	<alias name="input_drv" child="ps2_drv"/> }

append config {
	<start name="timer">
		<resource name="RAM" quantum="1M"/>
		<provides><service name="Timer"/></provides>
	</start>
	<start name="nitpicker">
		<resource name="RAM" quantum="1M"/>
		<provides> <service name="Gui"/> <service name="Capture"/> </provides>
		<config request_framebuffer="no">
			<capture/>
			<domain name="pointer" layer="1" content="client" label="no" origin="pointer" />
			<domain name="default" layer="2" content="client" label="no" focus="click" hover="always" />
			<policy label_prefix="pointer" domain="pointer"/>
			<default-policy                domain="default"/>
		</config>
		<route>
			<service name="Input"> <child name="input_drv"/> </service>
			<any-service> <parent/> <any-child/> </any-service>
		</route>
	</start>
	<start name="pointer">
		<resource name="RAM" quantum="1M"/>
	</start>}

#
#
#

set backend_library mesa_gpu-softpipe.lib.so
if {$use_i965} {
set backend_library mesa_gpu-iris.lib.so
}

set launchpad_config {
<config>
	<launcher name="init" ram_quota="128M" caps="500">
		<config verbose="yes">
			<parent-provides>
				<service name="ROM"/>
				<service name="IRQ"/>
				<service name="IO_MEM"/>
				<service name="IO_PORT"/>
				<service name="PD"/>
				<service name="RM"/>
				<service name="CPU"/>
				<service name="LOG"/>
				<service name="Gui"/>
				<service name="Timer"/>
				<service name="Gpu"/>
			</parent-provides>

			<start name="liquid_fb" caps="100">
				<resource name="RAM" quantum="16M"/>
				<provides> <service name="Framebuffer"/> <service name="Input"/> </provides>
				<config resize_handle="on" animate="off" title="gears" width="640" height="480" xpos="100" ypos="100"/>
				<route>
				<any-service> <parent/> <any-child/> </any-service>
				</route>
			</start>}

append launchpad_config "
			<start name=\"$demo_binary\" caps=\"$demo_caps\">
				<resource name=\"RAM\" quantum=\"$demo_ram_quota\"/>"

append launchpad_config $demo_config

append launchpad_config "
				<route>
					<service name=\"ROM\" label=\"mesa_gpu_drv.lib.so\">
						<parent label=\"$backend_library\"/>
					</service>"
append launchpad_config {
					<any-service> <parent/> <any-child/> </any-service>
				</route>
			</start>
			</config>
	</launcher>
</config>}

append config {
	<start name="launchpad" caps="6666">
		<resource name="RAM" quantum="2048M"/>}
append config $launchpad_config
append config {
	</start>}

append_if $use_i965 config {
	<start name="intel_fb_drv" caps="1000">
		<binary name="intel_gpu_drv"/>
		<resource name="RAM" quantum="40M"/>
		<provides> <service name="Gpu"/> </provides>
		<route>
			<any-service> <parent /> <any-child/> </any-service>
		</route>
	</start>}

append config {
</config>}

install_config $config

set boot_modules {
	core init ld.lib.so timer nitpicker pointer liquid_fb
	launchpad
	input_event_bridge

	libc.lib.so vfs.lib.so libm.lib.so
	egl.lib.so mesa.lib.so stdcxx.lib.so
	expat.lib.so glapi.lib.so sdl.lib.so

	mesa_gpu-softpipe.lib.so
}

append_if $use_i965 boot_modules {
	mesa_gpu-iris.lib.so drm.lib.so intel_gpu_drv
}

append boot_modules $demo_modules

lappend_if [have_board         linux]  boot_modules fb_sdl
lappend_if [expr ![have_board  linux]] boot_modules fb_drv
lappend_if [have_board            pc]  boot_modules ps2_drv

append_platform_drv_boot_modules

build_boot_image $boot_modules

append qemu_args " -m 768"

run_genode_until forever
