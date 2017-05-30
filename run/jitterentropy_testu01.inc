#
# \brief  Jitterentropy TestU01 tests
# \author Emery Hemingway
# \date   2015-12-11
#

#
# Configuration variables
#
# $test_name
#

set build_components {
	core init drivers/timer
	lib/vfs/jitterentropy
}

lappend build_components test/$test_name

build $build_components

create_boot_directory

set config {
<config>
	<default caps="128"/>
	<parent-provides>
		<service name="CPU"/>
		<service name="LOG"/>
		<service name="PD"/>
		<service name="RM"/>
		<service name="ROM"/>
		<service name="SIGNAL"/>
	</parent-provides>
	<default-route>
		<any-service> <parent/> <any-child/> </any-service>
	</default-route>
	<default caps="100"/>
	<start name="timer">
		<resource name="RAM" quantum="1M"/>
		<provides> <service name="Timer"/> </provides>
	</start>
	<start name="}
append config $test_name
append config {">
		<exit propagate="yes"/>
		<resource name="RAM" quantum="1G"/>
		<config file="/jitterentropy">
			<libc stdout="/log" stderr="/log"/>
			<vfs> <log/> <jitterentropy/> </vfs>
		</config>
	</start>
</config>
}

install_config $config

# generic modules
set boot_modules {
	core init timer
	ld.lib.so libc.lib.so libm.lib.so
	vfs_jitterentropy.lib.so
}

lappend boot_modules $test_name

build_boot_image $boot_modules

append qemu_args " -nographic"

# vi: set ft=tcl :
