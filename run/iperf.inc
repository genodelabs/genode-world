#
# \brief  Test for using iperf
# \author Roland Baer
# \date   2020-08-04
#

if {[expr $use_nic_bridge && $use_nic_router]} {
	puts "Cannot test with both NIC bridge and NIC router.";
	exit 0
}

if {[get_cmd_switch --autopilot] && [have_include "power_on/qemu"]} {
	puts "\nRunning netperf in autopilot on Qemu is not recommended.\n"
	exit
}

if {[have_include "power_on/qemu"] &&
    [expr $use_nic_bridge || $use_nic_router || $use_usb_driver || $use_wifi_driver]} {
	puts "Cannot test this configuration on qemu.";
	exit 0
}

if {[have_board imx53_qsb_tz]} {
	puts "Run script is not supported on this platform."
	exit 0
}
set bridge_mac "02:02:02:02:16:00"

# autopilot: configure disjoint mac-address ranges for x86_32, x86_64, and others
if {[get_cmd_switch --autopilot]} {
	if {[have_spec x86_32]} { set bridge_mac "02:02:02:02:32:00" }
	if {[have_spec x86_64]} { set bridge_mac "02:02:02:02:64:00" }
}

set version "2.0.13"

# sanity check that the right version is used
set wrong_version [catch {
	spawn iperf -v
	expect {
		{iperf version 2.0.13 (21 Jan 2019) pthreads}  { }
		eof { return }
		timeout { return }
	}
}]

if {$wrong_version} {
	puts -nonewline "\nPlease compile a iperf client of version $version "
	puts "for your host system."
	puts "The sources are available in 'contrib/iperf-<hash>' (after you "
	puts "prepared the port by calling 'tool/ports/prepare_port iperf')."
	puts "Please name the binary iperf\n"
	exit 1;
}

# iperf configuration
#set packet_size 1024

proc socket_fs_plugin {} {
	global use_lxip
	if { $use_lxip } { return lxip }
	return lwip
}
create_boot_directory

set packages "
	[depot_user]/src/[base_src]
	[depot_user]/pkg/[drivers_nic_pkg]
	[depot_user]/src/init
	[depot_user]/src/libc
	[depot_user]/src/posix
	[depot_user]/src/vfs
	[depot_user]/src/nic_router
"

append_if [expr !$use_lxip] packages " [depot_user]/src/vfs_lwip          "
append_if $use_lxip         packages " [depot_user]/src/vfs_lxip          "
append_if $use_nic_bridge   packages " [depot_user]/src/nic_bridge        "
append_if $use_usb_driver   packages " [depot_user]/src/platform_drv      "
append_if $use_usb_driver   packages " [depot_user]/src/usb_drv           "
append_if $use_wifi_driver  packages " [depot_user]/src/fs_rom            "
append_if $use_wifi_driver  packages " [depot_user]/src/vfs_import        "
append_if $use_wifi_driver  packages " [depot_user]/src/report_rom        "
append_if $use_wifi_driver  packages " [depot_user]/src/wifi_drv          "
append_if $use_wifi_driver  packages " [depot_user]/src/openssl           "
append_if $use_wifi_driver  packages " [depot_user]/src/vfs_jitterentropy "
append_if $use_wifi_driver  packages " [depot_user]/raw/wifi_firmware     "

import_from_depot $packages

build { app/iperf }

#
# Generate config
#

set lx_ip_addr "10.0.2.55"

proc platform_drv_policy {} {
	global use_wifi_driver
	if { $use_wifi_driver } {
		return {<policy label_prefix="nic_drv"> <pci class="WIFI"/> </policy>}}
	return {<policy label_prefix="nic_drv"> <pci class="USB"/> </policy>}
}

set config {
<config verbose="yes" prio_levels="2">
	<parent-provides>
		<service name="ROM"/>
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

	<start name="timer">
		<resource name="RAM" quantum="1M"/>
		<provides> <service name="Timer"/> </provides>
	</start> }

if { $use_wifi_driver } {
	append_platform_drv_config
	append config {
	<start name="report_rom">
		<resource name="RAM" quantum="2M"/>
		<provides> <service name="Report" /> <service name="ROM" /> </provides>
		<config> <rom/> </config>
	</start>

	<start name="config_fs">
		<binary name="vfs"/>
		<resource name="RAM" quantum="4M"/>
		<provides> <service name="File_system"/> </provides>
		<config>
			<vfs>
				<ram/>
				<import>
					<inline name="wifi_config">
<wifi_config connected_scan_interval="0" scan_interval="10" rfkill="no" verbose="no" verbose_state="no">}
append config "<network ssid=\"$wifi_ssid\" protection=\"WPA2\" passphrase=\"$wifi_psk\"/>"
append  config { </wifi_config>
					</inline>
				</import>
			</vfs>
			<policy label_prefix="config_rom" root="/"/>
		</config>
	</start>

	<start name="config_rom">
		<binary name="fs_rom"/>
		<resource name="RAM" quantum="4M"/>
		<provides><service name="ROM"/></provides>
		<route>
			<service name="File_system"> <child name="config_fs" /> </service>
			<any-service> <parent/> <any-child/> </any-service>
		</route>
	</start>

	<start name="nic_drv" caps="300">
		<binary name="wifi_drv"/>
		<resource name="RAM" quantum="32M"/>
		<config mode="uplink_client">
			<libc stdout="/dev/null" stderr="/dev/log" rtc="/dev/rtc"/>
			<vfs>
				<dir name="dev">
					<log/> <null/>
					<jitterentropy name="random"/>
					<jitterentropy name="urandom"/>
					<inline name="rtc">2018-01-01 00:01</inline>
				</dir>
			</vfs>
		</config>
		<route>
			<service name="Rtc"> <any-child/> </service>
			<service name="File_system"> <child name="config_fs"/> </service>
			<service name="ROM" label="wifi_config"> <child name="config_rom" /> </service>
			<service name="Report"> <child name="report_rom"/> </service>
			<service name="Uplink"> <child name="nic_router"/> </service>
			<any-service> <parent/> <any-child /> </any-service>
		</route>
	</start>}

} elseif { $use_usb_driver } {
	append_platform_drv_config
	append config {
	<start name="nic_drv" caps="150">
		<binary name="usb_drv"/>
		<resource name="RAM" quantum="18M"/>
		<provides><service name="Nic"/></provides>
		<config uhci="no" ehci="no" xhci="yes">
			<nic mac="02:00:00:00:01:01" />
		</config>
	</start> }
} else {
append config {
	<start name="nic_drv" caps="1000" managing_system="yes">
		<resource name="RAM" quantum="32M"/>
		<binary name="init"/>
		<route>
			<service name="ROM" label="config"> <parent label="drivers.config"/> </service>
			<service name="Timer"> <child name="timer"/> </service>
			<service name="Uplink"> <child name="nic_router"/> </service>
			<any-service> <parent/> </any-service>
		</route>
	</start> }
}


append config {
	<start name="nic_router" caps="120">
		<resource name="RAM" quantum="5M"/>
		<provides>
			<service name="Nic"/>
			<service name="Uplink"/>
		</provides>
		<config verbose_domain_state="yes"> }
if {$use_nic_bridge} { append config {
			<policy label_prefix="nic_bridge"       domain="server"/> }
} else { append config {
			<policy label_prefix="iperf_genode" domain="server"/> }
}
append config {
			<policy label_prefix="nic_drv"          domain="uplink"/>
			<domain name="uplink" }
append_if [have_spec linux] config "
			        interface=\"$lx_ip_addr/24\" gateway=\"10.0.2.1\""
append config {
			>
				<nat domain="server" tcp-ports="100" />
				<tcp-forward port="5001" domain="server" to="10.0.3.2" />
				<tcp-forward port="12865" domain="server" to="10.0.3.2" />
			</domain>
			<domain name="server" interface="10.0.3.1/24" verbose_packets="no">
				<dhcp-server ip_first="10.0.3.2"
				             ip_last="10.0.3.2"
				             ip_lease_time_sec="600"/>
			</domain>
		</config>
	</start> }

append_if $use_nic_bridge config {
	<start name="nic_bridge">
		<resource name="RAM" quantum="5M"/>
		<provides><service name="Nic"/></provides>
		<config mac="} $bridge_mac {">
			<default-policy/>
		</config>
		<route>
			<service name="Nic"> <child name="nic_router"/> </service>
			<any-service> <parent/> <any-child/> </any-service>
		</route>
	</start> }

append config {
	<start name="iperf_genode" caps="320" priority="-1">
	<binary name="iperf"/>
		<resource name="RAM" quantum="64M"/>
		<config>
			<arg value="iperf"/>
			<arg value="-s"/>
			<libc stdout="/dev/log" stderr="/dev/log" rtc="/dev/rtc" socket="/socket"/>
			<vfs>
				<dir name="dev">
					<log/> <inline name="rtc">2020-01-01 00:01</inline>
				</dir>
				<dir name="socket">
					<} [socket_fs_plugin] {}
append config { dhcp="yes"/>
				</dir>
			</vfs>
		</config>}
append_if $use_nic_bridge config {
		<route>
			<service name="Nic"> <child name="nic_bridge"/> </service>
			<any-service> <parent/> <any-child/> </any-service>
		</route>}
append_if [expr !$use_nic_bridge] config {
		<route>
			<service name="Nic"> <child name="nic_router"/> </service>
			<any-service> <parent/> <any-child/> </any-service>
		</route>}
append config {
	</start>
</config>
}

install_config $config

build_boot_image { iperf stdcxx.lib.so }

#
# Execute test case
#

# qemu config
append qemu_args "  -nographic "


append_qemu_nic_args "hostfwd=tcp::12865-:12865,hostfwd=tcp::49153-:49153"

set ip_match_string ""
set force_ports ""
if $use_lxip {
	set ip_match_string "ipaddr=(\[0-9\]+\.\[0-9\]+\.\[0-9\]+\.\[0-9\]+).*\n"
} else {
	set ip_match_string "address=(\[0-9\]+\.\[0-9\]+\.\[0-9\]+\.\[0-9\]+).*\n"
}
if $use_nic_router {
	set ip_match_string "nic_router\\\] \\\[uplink\\\] dynamic IP config: interface (\[0-9]{1,3}.\[0-9]{1,3}.\[0-9]{1,3}.\[0-9]{1,3}).*\n"
	set force_ports "-p 5001"
}

if {[have_board linux]} {
	run_genode_until {.*TCP window size.*\n} 60
} else {
	run_genode_until $ip_match_string 60
}

set serial_id [output_spawn_id]

if {[have_include "power_on/qemu"]} {
	set ip_addr "localhost"
	set force_ports "-p 49153"
} elseif [have_board linux] {
	set ip_addr $lx_ip_addr
} else {
	regexp $ip_match_string $output all ip_addr
	puts ""
}

# give the TCP/IP stack some time to settle down
sleep 3

set iperf_tests "w5K w50K w500K"

# start iperf client connecting to iperf server running native on Genode
foreach iperf_test $iperf_tests {
	puts "\n---------------------------- $iperf_test -----------------------"
	spawn iperf -fk -$iperf_test -t5 -c$ip_addr $force_ports
	set iperf_id $spawn_id

	set spawn_id_list [list $iperf_id $serial_id]

	# reset output, so that we get on the second run not the result of the first
	set output ""
	run_genode_until "Kbits/sec" 120 $spawn_id_list

	# get throughput from iperf output ("160 Kbits/sec")
	set throughput [regexp -all -inline {(\d+(\.\d+)*) Kbits\/sec} $output]
	set throughput [lindex $throughput 1]

	# format output parseable for post proccessing scripts
	puts -nonewline "\n! PERF: $iperf_test"
	if {$use_nic_bridge} { puts -nonewline "_bridge" }
	if {$use_nic_router} { puts -nonewline "_router" }
	if {$use_usb_driver} { puts -nonewline "_xhci"   }
	puts "              $throughput KBit/s ok"
	sleep 1
}
