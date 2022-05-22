#
# \brief  Using Seoul on Genode
# \author Norman Feske
# \author Markus Partheymueller
# \author Alexander Boettcher
# \date   2011-11-21

set use_fs_rump $use_block_vdi
set use_drv_ahci [expr $use_block_vdi || $use_block_sata]
set use_vfs_block [expr $use_block_ram || $use_genode_iso]

create_boot_directory

import_from_depot [depot_user]/src/[base_src] \
                  [depot_user]/src/event_filter \
                  [depot_user]/src/init \
                  [depot_user]/src/nitpicker \
                  [depot_user]/src/pc_usb_host_drv \
                  [depot_user]/src/usb_hid_drv \
                  [depot_user]/src/vfs \
                  [depot_user]/src/vfs_import \
                  [depot_user]/src/ps2_drv \
                  [depot_user]/raw/drivers_interactive-pc \
                  [depot_user]/pkg/themed_wm \
                  [depot_user]/src/report_rom \
                  [depot_user]/src/platform_drv \
                  [depot_user]/src/acpi_drv \
                  [depot_user]/src/pci_decode \
                  [depot_user]/src/rom_filter

if {$use_fs_rump} {
	import_from_depot [depot_user]/src/rump
}

if {$use_part_block} {
	import_from_depot [depot_user]/src/part_block
}

if {![info exists use_usb]} {
	set use_usb 1
}

if {![info exists use_virtio]} {
	set use_virtio 0
}

if {![info exists use_audio]} {
	set use_audio 0
}

if {$use_audio} {
	import_from_depot [depot_user]/src/bsd_audio_drv
}

#
# Build
#

assert_spec x86

set map_small         "no"
set vmm_vcpu_same_cpu "no"

if {[have_spec sel4]} {
	set map_small  "yes"
	set vmm_vcpu_same_cpu  "yes"

	# seL4 has no AMD SVM support
	if {[have_include "power_on/qemu"]} {
		puts "\n Run script is not supported on this platform. \n";
		exit 0
	}
}

if {[have_spec foc]} {
	# The performance is considerable bad when
	# vmm and vcpu is not on same physical CPU
	set vmm_vcpu_same_cpu  "yes"

	# Qemu SVM has no EPT support
	if {[have_include "power_on/qemu"]} {
		puts "\n Run script is not supported on this platform. \n";
		exit 0
	}
}

set build_components {
	drivers/framebuffer
	drivers/rtc
	app/seoul
}

lappend_if $use_block_vdi       build_components server/vdi_block
lappend_if $use_vfs_block       build_components server/vfs_block
lappend_if $use_drv_ahci        build_components drivers/ahci
lappend_if $use_nic_session     build_components drivers/nic
lappend_if $use_nic_session     build_components server/nic_router
lappend_if $use_genode_iso      build_components server/iso9660
lappend_if $use_top             build_components app/top

build $build_components

# write Seoul config file
set vm_cfg_fd [open "bin/vm_seoul.cfg" w]
puts $vm_cfg_fd "<config map_small=\"$map_small\" vmm_vcpu_same_cpu=\"$vmm_vcpu_same_cpu\" vmm_memory=\"27M\">"
puts $vm_cfg_fd {	 <machine verbose="no">
		<mem start="0x0" end="0x9a000"/>
		<mem start="0x100000" end="0xfffff000"/>
		<!--<ioio/>-->
		<nullio io_base="0x80" />
		<pic io_base="0x20" elcr_base="0x4d0"/>
		<pic io_base="0xa0" irq="2" elcr_base="0x4d1"/>
		<pit io_base="0x40" irq="0"/>
		<scp io_port_a="0x92" io_port_b="0x61"/>
		<kbc io_base="0x60" irq_kbd="1" irq_aux="12"/>
		<keyb ps2_port="0" host_keyboard="0x10000"/>
		<mouse ps2_port="1" host_mouse="0x10001"/>
		<rtc io_base="0x70" irq="8"/>
		<serial io_base="0x3f8" irq="0x4" host_serial="0x4711"/>
		<hostsink host_dev="0x4712" buffer="80"/>
		<vga io_base="0x03c0"/>}

if {!$use_multiboot} {
	puts $vm_cfg_fd {
		<vbios_disk boot_disknr="0"/>
	}
}

	puts $vm_cfg_fd {
		<vbios_keyboard host_keyboard="0x10000"/>
		<vbios_mem/>
		<vbios_time/>
		<vbios_reset/>}

if {$use_multiboot} {
	if {[info exists use_multiboot_modaddr]} {
		puts $vm_cfg_fd "		<vbios_multiboot modaddr=\"$use_multiboot_modaddr\"/>"
	} else {
		puts $vm_cfg_fd {		<vbios_multiboot/>}
	}
} 

puts $vm_cfg_fd {
		<msi/>
		<ioapic/>
		<pcihostbridge bus_num="0" bus_count="0x10" io_base="0xcf8"
		               mem_base="0xe0000000"/>
		<pmtimer io_port="0x8000"/>}

for {set i 0} {$i < $vcpus_to_be_used} {incr i 1} {
	puts $vm_cfg_fd {
		<vcpu/> <halifax/> <vbios/> <lapic/>}
}

if {!$use_multiboot} {

	if {$use_model_ahci} {
		puts $vm_cfg_fd {
		<ahci mem="0xe0800000" irq="14" bdf="0x30"/>
		<drive sigma0drive="0" controller="0" port="0"/>
		}
	}
	if {$use_model_ide} {
		puts $vm_cfg_fd {
		<ide port0="0x1f0" port1="0x3f6" irq="14" bdf="0x38" disk="0"/>
		}
	}
}

if {$use_nic_session} {
	puts $vm_cfg_fd {
		<!-- <rtl8029 irq="9" port="0x300"/> -->
		<intel82576vf/>
	}
}

if {$use_virtio} {
	puts $vm_cfg_fd {
		<virtio_input irq="11"/>
		<virtio_gpu   irq="13"/>
	}
}

if {$use_audio} {
	puts $vm_cfg_fd {
		<virtio_sound irq="15"/>
	}
}

puts $vm_cfg_fd {
	</machine>
	<multiboot>}

if {$use_multiboot} {
	puts $vm_cfg_fd $multiboot_files
}

puts $vm_cfg_fd {
	 </multiboot>
</config>}
close $vm_cfg_fd

#
# Generate Genode config
#

set config {
<config verbose="yes" prio_levels="4">
	<parent-provides>
		<service name="ROM"/>
		<service name="IRQ"/>
		<service name="IO_MEM"/>
		<service name="IO_PORT"/>
		<service name="PD"/>
		<service name="RM"/>
		<service name="CPU"/>
		<service name="VM"/>
		<service name="LOG"/>
		<service name="TRACE"/>
	</parent-provides>
	<default-route>
		<any-service> <parent/> <any-child/> </any-service>
	</default-route>
	<default caps="100"/>

	<start name="timer">
		<resource name="RAM" quantum="1M"/>
		<provides><service name="Timer"/></provides>
	</start>

	<start name="platform_reports" priority="-1">
		<binary name="report_rom"/>
		<resource name="RAM" quantum="1M"/>
		<provides>
			<service name="Report"/>
			<service name="ROM"/>
		</provides>
		<config>
			<policy label="pci_decode -> system" report="acpi_drv -> acpi"/>
			<policy label="rom_filter -> pci_devices" report="pci_decode -> devices"/>
			<policy label="usb_hid_drv -> report" report="usb_drv -> devices"/>
		</config>
	</start>

	<start name="acpi_drv" caps="350" priority="-1">
		<resource name="RAM" quantum="4M"/>
		<route>
			<service name="Report"> <child name="platform_reports"/> </service>
			<any-service> <parent/> <any-child/> </any-service>
		</route>
	</start>

	<start name="pci_decode" caps="350">
		<resource name="RAM" quantum="1M"/>
		<route>
			<service name="ROM" label="system"> <child name="platform_reports"/> </service>
			<service name="Report"> <child name="platform_reports"/> </service>
			<any-service> <parent/> <any-child/> </any-service>
		</route>
	</start>

	<start name="platform_drv" caps="400" managing_system="yes" priority="-1">
		<resource name="RAM" quantum="4M"/>
		<provides>
			<service name="Platform"/>
		</provides>
		<config>
			<policy label_prefix="ps2_drv">  <device name="ps2"/>    </policy>
			<policy label_prefix="ahci_drv"> <pci class="AHCI"/>     </policy>
			<policy label_prefix="nic_drv">  <pci class="ETHERNET"/> </policy>
			<policy label_prefix="audio_drv">
				<pci class="AUDIO"/> <pci class="HDAUDIO"/> </policy>
			<policy label_prefix="fb_drv"  info="yes"> <pci class="VGA"/> </policy>
			<policy label_prefix="usb_drv" info="yes"> <pci class="USB"/> </policy>
		</config>
		<route>
			<service name="ROM" label="devices"> <child name="rom_filter"/> </service>
			<any-service> <parent/> <any-child/> </any-service>
		</route>
	</start>

	<start name="rom_filter" caps="70">
		<resource name="RAM" quantum="1M"/>
		<provides> <service name="ROM"/> </provides>
		<config buffer="32K">
			<input name="devices"/>
			<input name="pci_devices"/>
			<output node="devices">
				<input name="devices"     skip_toplevel="yes"/>
				<input name="pci_devices" skip_toplevel="yes"/>
			</output>
		</config>
		<route>
			<service name="ROM" label="pci_devices"> <child name="platform_reports"/> </service>
			<service name="LOG"> <parent/> </service>
			<service name="PD">  <parent/> </service>
			<service name="CPU"> <parent/> </service>
			<service name="ROM"> <parent/> </service>
		</route>
	</start>

	<start name="rtc_drv" priority="-1">
		<resource name="RAM" quantum="1M"/>
		<provides><service name="Rtc"/></provides>
	</start>

	<start name="ps2_drv" priority="-1">
		<resource name="RAM" quantum="3M"/>
		<config/>
		<route>
			<service name="Event"> <child name="event_filter" label="ps2"/> </service>
			<any-service><parent/> <any-child/> </any-service> </route>
	</start>

	<start name="event_filter" caps="90" priority="-1">
		<resource name="RAM" quantum="1280K"/>
		<provides> <service name="Event"/> </provides>
		<route>
			<service name="ROM" label="config"> <parent label="event_filter.config"/> </service>
			<service name="Event"> <child name="nitpicker"/> </service>
			<any-service><parent/><any-child/></any-service>
		</route>
	</start>}

append_if $use_usb config {
	<start name="usb_drv" priority="-1" caps="120">
		<binary name="pc_usb_host_drv"/>
		<resource name="RAM" quantum="12M"/>
		<provides> <service name="Usb"/> </provides>
		<config>
			<report devices="yes"/>
			<policy label_prefix="usb_hid_drv" class="0x3"/>
		</config>
		<route>
			<service name="Report"> <child name="report_rom"/> </service>
			<any-service><parent/><any-child/></any-service>
		</route>
	</start>

	<start name="usb_hid_drv" priority="-1" caps="140">
		<resource name="RAM" quantum="11M"/>
		<config use_report="yes"/>
		<route>
			<service name="Report"> <child name="report_rom"/> </service>
			<service name="Event"> <child name="event_filter" label="usb"/> </service>
			<service name="ROM" label="report"> <child name="report_rom"/> </service>
			<service name="Usb"> <child name="usb_drv"/> </service>
			<any-service><parent/><any-child/></any-service>
		</route>
	</start>}

append_if $use_audio config {
	<start name="audio_drv">
		<binary name="pci_audio_drv"/>
		<resource name="RAM" quantum="8M"/>
		<provides>
			<service name="Audio_in"/>
			<service name="Audio_out"/>
		</provides>
		<config/>
	</start>}

append_if $use_drv_ahci config {
	<start name="ahci_drv" priority="-1">
		<resource name="RAM" quantum="1M" />
		<provides><service name="Block"/></provides>
		<config>}

append_if [expr $use_block_sata && !$use_part_block] config {
			<policy label="seoul -> disk0" device="0" writeable="yes"/>}

append_if [expr $use_block_vdi && !$use_part_block] config {
			<policy label="rump_fs -> " device="0" writeable="yes"/>}

append_if [expr $use_block_vdi && $use_part_block] config {
			<policy label="part_block -> " device="0" writeable="yes"/>}

append_if [expr $use_block_sata && $use_part_block] config {
			<policy label="part_block -> " device="0" writeable="yes"/>}

append_if $use_drv_ahci config {
		</config>
	</start>}

append_if $use_part_block config {
	<start name="part_block" priority="-1">
		<resource name="RAM" quantum="10M"/>
		<provides><service name="Block"/></provides>
		<route>
			<any-service><child name="ahci_drv"/> <parent/></any-service>
		</route>
		<config>
			<policy label="seoul -> disk0" partition="5" writeable="yes"/>
			<policy label="rump_fs -> " partition="4" writeable="yes"/>
		</config>
	</start>}

append_if $use_block_ram config {
	<start name="vfs_block" priority="-1" caps="300">
		<resource name="RAM" quantum="266M" />
		<provides><service name="Block"/></provides>
		<config>
			<vfs>
				<ram/>
				<import>
					<rom name="seoul-disc.raw"/>
				</import>
			</vfs>
			<default-policy file="/seoul-disc.raw" block_size="512"
			                writeable="yes"/>
		</config>
	</start>}

append_if $use_fs_rump config {
	<start name="rump_fs" priority="-1" caps="200">
		<binary name="vfs"/>
		<resource name="RAM" quantum="32M"/>
		<provides><service name="File_system"/></provides>
		<config ld_verbose="yes">
			<vfs>
				<rump fs="ext2fs" ram="28M"/>
			</vfs>
			<default-policy root="/" writeable="yes"/>
		</config>
		<route>
			<service name="Timer"><child name="timer"/></service>}

append_if [expr $use_fs_rump && $use_part_block] config {
			<service name="Block"><child name="part_block"/></service>}
append_if [expr $use_fs_rump && !$use_part_block] config {
			<service name="Block"><child name="ahci_drv"/></service>}

append_if $use_fs_rump config {
			<any-service> <parent/> </any-service>
		</route>
	</start>}

append_if $use_block_vdi config {
	<start name="vdi_block">
		<resource name="RAM" quantum="8M"/>
		<provides> <service name="Block"/> </provides>
		<config file="/seoul_stress32.vdi" writeable="yes">
			<vfs> <fs buffer_size="1M"/> </vfs>
			<policy label="seoul -> disk0" device="0" writeable="yes"/>
		</config>
		<route>
			<service name="File_system"><child name="rump_fs"/></service>
			<any-service> <parent/> <any-child /> </any-service>
		</route>
	</start>}

append_if $use_genode_iso config {
	<start name="vfs_block" priority="-1" caps="100">
		<resource name="RAM" quantum="16M" />
		<provides><service name="Block"/></provides>
		<config>
			<vfs>
				<rom name="genode.iso"/>
			</vfs>
			<default-policy file="/genode.iso" block_size="2048"/>
		</config>
	</start>

	<start name="iso9660" priority="-1">
		<resource name="RAM" quantum="16M"/>
		<provides><service name="ROM"/></provides>
		<route>
			<service name="Block"><child name="vfs_block"/></service>
			<any-service><parent/></any-service>
		</route>
	</start>}

append_if $use_nic_session config {

	<start name="nic_drv" priority="-2">
		<binary name="ipxe_nic_drv"/>
		<resource name="RAM" quantum="8M"/>
		<route>
			<service name="Uplink"><child name="nic_router"/></service>
			<any-service><parent/><any-child/></any-service>
		</route>
	</start>

	<start name="nic_router" caps="200" priority="-1">
		<resource name="RAM" quantum="10M"/>
		<provides>
			<service name="Nic"/>
			<service name="Uplink"/>
		</provides>
		<config verbose_domain_state="yes">

			<policy label_prefix="seoul"      domain="downlink"/>
			<policy label_prefix="nic_drv"    domain="uplink"/>

			<domain name="uplink">

				<nat domain="downlink"
				     tcp-ports="16384"
				     udp-ports="16384"
				     icmp-ids="16384"/>

			</domain>

			<domain name="downlink" interface="10.0.3.1/24">

				<dhcp-server ip_first="10.0.3.55" ip_last="10.0.3.155" dns_config_from="uplink"/>

				<tcp dst="0.0.0.0/0"><permit-any domain="uplink" /></tcp>
				<udp dst="0.0.0.0/0"><permit-any domain="uplink" /></udp>
				<icmp dst="0.0.0.0/0" domain="uplink"/>

			</domain>

		</config>
	</start>
}

append config {
	<start name="fb_drv" priority="-1" caps="130">
		<binary name="vesa_fb_drv"/>
		<resource name="RAM" quantum="16M"/>
		<route>
			<service name="Timer">    <child name="timer"/></service>
			<service name="Capture">  <child name="nitpicker"/></service>
			<service name="Platform"> <any-child/></service>
			<any-service> <parent/> </any-service>
		</route>}
append_if [have_include "power_on/qemu"] config {
		<config width="1280" height="960"/>}
append_if [expr ![have_include "power_on/qemu"]] config {
		<config/>}
append config {
	</start>

	<start name="seoul" priority="-2" caps="1200">
		<binary name="seoul"/>
		<resource name="RAM" quantum="} $memory_vmm_vm {"/>
		<route>
			<service name="Timer"><child name="timer"/></service>
			<service name="ROM" label="config">
				<parent label="vm_seoul.cfg"/> </service>}

append_if $use_genode_iso config {
			<service name="ROM" unscoped_label="seoul"> <parent/> </service>
			<service name="ROM" unscoped_label="ld.lib.so"> <parent/> </service>
			<service name="ROM" label="platform_info"> <parent/> </service>
			<service name="ROM"><child name="iso9660"/></service>}
append_if $use_block_vdi config {
			<service name="Block"><child name="vdi_block"/></service>}
append_if [expr $use_block_sata && $use_part_block] config {
			<service name="Block"><child name="part_block"/></service>}
append config {
			<service name="Gui"> <child name="wm"/> </service>
			<service name="Rtc"> <child name="rtc_drv"/> </service>
			<service name="Report" label="shape"> <child name="wm"/> </service>
			<service name="Report"> <child name="report_rom"/> </service>
			<any-service><parent/><any-child/></any-service>
		</route>
	</start>

	<start name="nitpicker" priority="-1" caps="120">
		<resource name="RAM" quantum="8M"/>
		<provides>
			<service name="Gui"/> <service name="Capture"/> <service name="Event"/>
		</provides>
		<config>
			<capture/> <event/>
			<report focus="yes" hover="yes" />
			<domain name="pointer" layer="1" content="client" label="no" origin="pointer" />
			<domain name="panel"   layer="2" content="client" label="no" focus="none" />
			<domain name=""        layer="3" content="client" label="no" focus="click" hover="always"/>

			<policy label_prefix="pointer"    domain="pointer"/>

			<default-policy domain=""/>
		</config>
		<route>
			<service name="Timer">  <child name="timer"/></service>
			<service name="Report"> <child name="report_rom"/></service>
			<any-service><parent/><any-child/></any-service>
		</route>
	</start>

	<start name="pointer" priority="-1">
		<resource name="RAM" quantum="2M"/>
		<provides> <service name="Report"/> </provides>
		<config shapes="yes" verbose="no"/>
		<route>
			<service name="Gui"> <child name="nitpicker"/>  </service>
			<service name="ROM" label="hover"> <child name="report_rom"/> </service>
			<service name="ROM" label="xray"> <child name="report_rom"/> </service>
			<any-service> <parent/> </any-service>
		</route>
	</start>

	<start name="report_rom">
		<resource name="RAM" quantum="1M"/>
		<provides> <service name="Report"/> <service name="ROM"/> </provides>
		<config>
			<policy label="pointer -> hover" report="nitpicker -> hover"/>
			<policy label="pointer -> xray"  report="nitpicker -> xray"/>
			<policy label="usb_hid_drv -> report" report="usb_drv -> devices"/>
		</config>
	</start>

	<start name="wm" caps="1000" priority="-1">
		<resource name="RAM" quantum="32M"/>
		<binary name="init"/>
		<provides>
			<service name="Gui"/> <service name="Report"/> <service name="ROM"/>
		</provides>
		<route>
			<service name="ROM"    label="config"> <parent label="wm.config"/> </service>
			<service name="Report" label_last="shape">  <child name="pointer"/> </service>
			<service name="Gui"> <child name="nitpicker"/> </service>
			<any-service> <parent/> <any-child/> </any-service>
		</route>
	</start>}

append_if $use_top config {
	<start name="top">
		<resource name="RAM" quantum="2M"/>
		<config period_ms="10000"/>
	</start>}

append config {
</config>}

install_config $config

#
# Boot modules
#

# non PCI devices for platform_drv, e.g. ps2 and pit
file copy [select_from_repositories board/[board]/devices] [run_dir]/genode/devices

# generic modules
set boot_modules {
	rtc_drv vesa_fb_drv
	seoul
	vm_seoul.cfg
}

lappend_if $use_block_vdi     boot_modules vdi_block
lappend_if $use_vfs_block     boot_modules vfs_block
lappend_if $use_drv_ahci      boot_modules ahci_drv
lappend_if $use_nic_session   boot_modules ipxe_nic_drv
lappend_if $use_nic_session   boot_modules nic_router

lappend_if $use_genode_iso    boot_modules iso9660
lappend_if $use_genode_iso    boot_modules genode.iso

lappend_if $use_block_ram     boot_modules seoul-disc.raw

lappend_if $use_top           boot_modules top

#
# Add OS binaries of guest
#

if {$use_multiboot} {
	set guest_os_binary_missing 0
	set binary_counter 0
	foreach binary $guest_os_binaries {
		if {![file exists bin/$binary]} {
			puts stderr "Error: guest OS binary \"bin/$binary\" does not exist"
			set guest_os_binary_missing 1
		}

		if {[info exists sha1_os_binaries]} {
			set sha1 [exec sha1sum bin/$binary]
			set sha1 [regexp -inline {[0-9a-h]+} $sha1]
			if {[string compare $sha1 [lindex $sha1_os_binaries $binary_counter]]} {
				puts "SHA1 sum of binary does not match with expected one - abort"
				puts "$binary $sha1 != [lindex $sha1_os_binaries $binary_counter]"
				set guest_os_binary_missing 1
			}
		}
		incr binary_counter 1
	}

	if {$guest_os_binary_missing} { exit 1 }

	append boot_modules $guest_os_binaries
}

build_boot_image $boot_modules

# A copy of the config is placed in the run folder.
exec rm -f bin/vm_seoul.cfg


if {$use_audio} {
	append qemu_args " -device ES1370 "
}
