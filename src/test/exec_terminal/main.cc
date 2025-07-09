/*
 * \brief  Component starting bash in a sub-init to execute a specific command
 * \author Sid Hussmann
 * \author Norman Feske
 * \date   2019-05-11
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 * Copyright (C) 2019 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/log.h>
#include <base/component.h>
#include <base/attached_rom_dataspace.h>
#include <os/reporter.h>

namespace Exec_terminal {

	class Main;

	using namespace Genode;
}


class Exec_terminal::Main
{
	private:

		Env&                   _env;
		Attached_rom_dataspace _config               { _env, "exec_terminal.config" };
		Signal_handler<Main>   _config_handler       { _env.ep(), *this, &Main::_handle_config };
		Expanding_reporter     _init_config_reporter { _env, "config", "config" };
		unsigned int           _version              { 0 };

		void _handle_config();

		void _gen_init_config(Generator &, Node const &config) const;

		void _gen_sub_init_config(Generator &, Node const &config) const;

		static void _gen_service_node(Generator &g, char const *name)
		{
			g.node("service",[&] { g.attribute("name", name); });
		}

		static void _gen_parent_provides(Generator &g)
		{
			g.node("parent-provides",[&] {
				_gen_service_node(g, "CPU");
				_gen_service_node(g, "File_system");
				_gen_service_node(g, "LOG");
				_gen_service_node(g, "PD");
				_gen_service_node(g, "RM");
				_gen_service_node(g, "ROM");
				_gen_service_node(g, "Report");
				_gen_service_node(g, "Terminal");
				_gen_service_node(g, "Timer");
			});
		}

	public:

		Main(Env& env) : _env(env)
		{
			_config.sigh(_config_handler);
			_handle_config();
		}
};


void Exec_terminal::Main::_handle_config()
{
	_config.update();

	Node const config = _config.node();

	log(config);
	if (config.has_type("empty"))
		return;

	_version++;

	_init_config_reporter.generate([&] (Generator &g) {

		if (!config.has_attribute("exit"))
			_gen_init_config(g, config);
	});
}


void Exec_terminal::Main::_gen_init_config(Generator &g, Node const &config) const
{
	_gen_parent_provides(g);

	g.node("start",[&] {
		g.attribute("name",    "init");
		g.attribute("caps",    900);
		g.attribute("version", _version);

		g.node("resource",[&] {
			g.attribute("name", "RAM");
			g.attribute("quantum", "70M"); });

		g.node("config", [&] {
			_gen_sub_init_config(g, config); });

		g.node("route",[&] {
			g.node("any-service",[&] {
				g.node("parent",[&] { }); }); });
	});
}


void Exec_terminal::Main::_gen_sub_init_config(Generator &g, Node const &config) const
{
	g.attribute("verbose", "no");

	_gen_parent_provides(g);

	auto gen_parent_route = [&] (auto name) {
		g.node("service",[&] {
			g.attribute("name", name);
			g.node("parent",[&] {}); }); };

	auto gen_ram = [&] (auto ram) {
		g.node("resource",[&] {
			g.attribute("name", "RAM");
			g.attribute("quantum", ram); }); };

	auto gen_provides_service = [&] (auto name) {
		g.node("provides", [&] {
			g.node("service", [&] {
				g.attribute("name", name); }); }); };

	g.node("start",[&] {
		g.attribute("name", "vfs");
		g.attribute("caps", 120);
		gen_ram("32M");
		gen_provides_service("File_system");
		g.node("config",[&] {
			g.node("vfs",[&] {
				g.node("tar",[&] { g.attribute("name", "bash.tar"); });
				g.node("tar",[&] { g.attribute("name", "coreutils-minimal.tar"); });
				g.node("tar",[&] { g.attribute("name", "vim-minimal.tar"); });
				g.node("dir",[&] {
					g.attribute("name", "rw");
					g.node("fs",[&] { g.attribute("label", "rw -> /"); });
				});
				g.node("dir", [&] {
					g.attribute("name", "dev");
					g.node("terminal", [&] { });
					g.node("inline", [&] {
						g.attribute("name", "rtc");
						g.append_quoted("2018-01-01 00:01");
					});
				});
				g.node("dir",[&] {
					g.attribute("name", "tmp");
					g.node("ram",[&] { });
				});
				g.node("inline", [&] {
					g.attribute("name", ".bash_profile");
					g.append_quoted("echo Hello from Genode! > /dev/log");
				});
			});
			g.node("default-policy", [&] {
				g.attribute("root",      "/");
				g.attribute("writeable", "yes");
			});
		});
		g.node("route",[&] {
			gen_parent_route("CPU");
			gen_parent_route("LOG");
			gen_parent_route("PD");
			gen_parent_route("ROM");
			gen_parent_route("File_system");
			gen_parent_route("Terminal");
		});
	});

	auto gen_vfs_route = [&] {
		g.node("service",[&] {
			g.attribute("name", "File_system");
			g.node("child",[&] { g.attribute("name", "vfs"); }); }); };

	g.node("start",[&] {
		g.attribute("name", "vfs_rom");
		g.attribute("caps", 100);
		gen_ram("16M");
		gen_provides_service("ROM");
		g.node("binary",  [&] { g.attribute("name", "fs_rom"); });
		g.node("config",  [&] { });
		g.node("route",   [&] {
			gen_parent_route("CPU");
			gen_parent_route("LOG");
			gen_parent_route("PD");
			gen_parent_route("ROM");
			gen_vfs_route();
		});
	});

	g.node("start",[&] {
		g.attribute("name", "/bin/bash");
		g.attribute("caps", 500);
		gen_ram("16M");

		/* exit sub init when leaving bash */
		g.node("exit",[&] {
			g.attribute("propagate", "yes"); });

		g.node("config",[&] {

			g.node("libc",[&] {
				g.attribute("stdin",  "/dev/terminal");
				g.attribute("stdout", "/dev/terminal");
				g.attribute("stderr", "/dev/terminal");
				g.attribute("rtc",    "/dev/rtc");
			});

			g.node("vfs",[&] {
				g.node("fs",[&] { g.attribute("label", "rw -> /"); });
				g.node("dir", [&] {
					g.attribute("name", "dev");
					g.node("null", [&] { });
					g.node("log",  [&] { });
				});
			});

			auto gen_env = [&] (auto key, auto value) {
				g.node("env",[&] {
					g.attribute("key",   key);
					g.attribute("value", value); }); };

			auto gen_arg = [&] (auto value) {
				g.node("arg",[&] {
					g.attribute("value", value); }); };

			gen_env("TERM",     "screen");
			gen_env("HOME",     "/");
			gen_env("PATH",     "/bin");
			gen_env("HISTFILE", "");
			gen_env("IGNOREOF", "3");

			gen_arg("/bin/bash");

			if (config.has_attribute("command")) {

				typedef String<128> Command;
				Command const command = config.attribute_value("command", Command());

				if (command.valid()) {
					gen_arg("-c");

					/*
					 * XXX appending " ; true" is done to force bash to fork.
					 * Bash fails to return the proper exit code otherwise.
					 */
					gen_arg(String<200>(command, " ; true"));
				}
			} else {
				gen_env("PS1", "noux@$PWD> ");
				gen_arg("--login");
			}
		});
		g.node("route",[&] {
			g.node("service",[&] {
				g.attribute("name", "ROM");
				g.attribute("label_last", "/bin/bash");
				g.node("child",[&] { g.attribute("name", "vfs_rom"); });
			});
			g.node("service",[&] {
				g.attribute("name", "ROM");
				g.attribute("label_prefix", "/bin");
				g.node("child",[&] { g.attribute("name", "vfs_rom"); });
			});
			gen_parent_route("CPU");
			gen_parent_route("LOG");
			gen_parent_route("PD");
			gen_parent_route("RM");
			gen_parent_route("ROM");
			gen_parent_route("Timer");
			gen_vfs_route();
		});
	});
}


void Component::construct(Genode::Env &env)
{
	static Exec_terminal::Main main(env);
}
