if ![have_installed synergys] {
	puts stderr "\nsynergy server not installed\n"
	exit -1
}

if {[have_spec linux]} {
	puts "\Synergy scenarios not supported on Linux.\n"
	exit
}

proc append_synergy_build_components { } {
	global build_components
	append build_components { drivers/nic server/synergy_input }
}

proc append_synergy_config { } {
	global config

	append config {
	<start name="nic_drv">
		<resource name="RAM" quantum="4M"/>
		<provides><service name="Nic"/></provides>
	</start>

	<start name="synergy_input">
		<resource name="RAM" quantum="8M"/>
		<provides> <service name="Input"/> </provides>
		<config addr="10.0.2.2" name="genode-up"/>
	</start>
	}
}

proc append_synergy_boot_modules { } {
	global boot_modules
	append boot_modules { nic_drv libc.lib.so lwip.lib.so synergy_input }
}


proc write_synergy_host_config { } {
	set fh [ open "bin/synergy.conf" w]
	puts $fh {
section: screens
	localhost:
	genode-up:
	genode-left:
	genode-right:
	genode-down:
end

section: links

	localhost:
		up = genode-up
		left = genode-left
		right = genode-right
		down = genode-down

	genode-up:
		down = localhost

	genode-left:
		up = localhost
		right = localhost
		down = localhost

	genode-right:
		up = localhost
		left = localhost
		down = localhost

	genode-down:
		up = localhost
end
}
	close $fh
}


proc remove_synergy_host_config { } {
	file delete "bin/synergy.conf"
}


proc start_synergy_host { } {
	puts stderr "spawning synergys"
	spawn synergys \
		-f -d DEBUG -n localhost -c bin/synergy.conf
}
