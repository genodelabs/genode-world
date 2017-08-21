set demo_component test/sdl_opengl/lesson36
set demo_binary    sdl_opengl-lesson36
set demo_ram_quota 64M
set demo_caps      200

set demo_config {
<config ld_verbose="true">
	<libc stdout="/dev/log" stderr="/dev/log"/>
	<vfs>
		<dir name="dev"> <log/> </dir>
	</vfs>
</config>}

set demo_modules {
	sdl_opengl-lesson36
}

source ${genode_dir}/repos/world/run/sdl_opengl.inc
