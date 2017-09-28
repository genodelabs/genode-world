/*
 * \brief  Transactional XML editor terminal frontend
 * \author Emery Hemingway
 * \date   2017-04-29
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <os/reporter.h>
#include <vfs/file_system_factory.h>
#include <vfs/dir_file_system.h>
#include <terminal_session/connection.h>
#include <base/attached_rom_dataspace.h>
#include <base/heap.h>
#include <base/component.h>

/* Cli_monitor includes */
#include <command_line.h>
#include <line_editor.h>


namespace Xml_term_edit {
	using namespace Genode;
	using namespace Cli_monitor;

	struct Command;
	struct Add_command;
	struct Del_command;
	struct Exit_command;
	struct Main;
}

Genode::Env *_env;

struct Xml_term_edit::Command : Cli_monitor::Command
{
	Vfs::Dir_file_system &vfs;
	Genode::Allocator    &alloc;
	Reporter             &report;
	Vfs::Vfs_handle      *root_handle = nullptr;

	Command(char const *name,
	        char const *desc,
	        Command_registry     &cmds,
	        Vfs::Dir_file_system &vfs,
	        Genode::Allocator    &alloc,
	        Reporter             &report)
	:
		Cli_monitor::Command(name, desc),
		vfs(vfs), alloc(alloc), report(report)
	{
		auto r = vfs.open(
			"/", Vfs::Directory_service::OPEN_MODE_RDONLY, &root_handle, alloc);
		if (r !=  Vfs::Directory_service::Open_result::OPEN_OK) {
			Genode::error("failed to open VFS root directory");
			throw r;
		}

		cmds.insert(this);
	}

	void _for_each_argument(Argument_fn const &fn) const override
	{
		typedef Vfs::File_io_service::Read_result Result;

		enum { DIRENT_COUNT = 4096 / sizeof(Vfs::Directory_service::Dirent) };
		Vfs::Directory_service::Dirent dirents[DIRENT_COUNT];
		memset(dirents, 0x00, sizeof(dirents));

		root_handle->seek(0);

		for (;;) {
			while (!vfs.queue_read(root_handle, sizeof(dirents))) {
				_env->ep().wait_and_dispatch_one_io_signal();
			}
			Vfs::file_size read_count = 0;
			Result r;
			for (;;) {
				r = vfs.complete_read(
					root_handle, (char*)&dirents, sizeof(dirents), read_count);
				if (r == Result::READ_QUEUED) {
					_env->ep().wait_and_dispatch_one_io_signal();
				}
				else
					break;
			}

			if (r != Result::READ_OK) {
				Genode::error("failed to read subsystems");
				return;
			}

			if (read_count == 0) return;
			root_handle->advance_seek(read_count);
			read_count = read_count / DIRENT_COUNT;

			for (unsigned i = 0; i < read_count; i++) {
				Vfs::Directory_service::Dirent const &e = dirents[i];

				switch (e.type) {
				case Vfs::Directory_service::DIRENT_TYPE_FILE:
					/* check if the VFS returned junk */
					if (e.name[0] != '\0')
						fn(Argument(e.name, ""));
					break;
				case Vfs::Directory_service::DIRENT_TYPE_END:
					return;
				default:
					break;
				}
			}
		}
	}

	void insert_file_content(Command_line &cmd, Xml_generator gen)
	{
		using namespace Vfs;

		Path<128> path;
		Vfs_handle *handle;
		{
			char name[128] = { '\0' };
			if (cmd.argument(0, name, sizeof(name)) == false) {
				error("Error: no configuration name specified\n");
				return;
			}
			path.import(name);
		}

		Directory_service::Stat sb;
		vfs.stat(path.base(), sb);

		/* XXX: error handling */
		if (!sb.size)
			return;

		typedef Directory_service::Open_result Open_result;
		Open_result res = vfs.open(
			path.base(),
			Directory_service::OPEN_MODE_RDONLY,
			&handle,
			alloc);
		switch (res) {
		case Open_result::OPEN_OK:
			break;
		default:
			error("failed to open '", path, "'");
			/* XXX: log and write error info to the terminal */
			return;
		}
		Vfs_handle::Guard guard(handle);

		char buf[1024];
		file_size offset = 0;
		while (offset < sb.size) {
			file_size n = 0;
			file_size count = min(sizeof(buf), sb.size-offset);
			handle->fs().complete_read(handle, buf, count, n);
			if (!n)
				return;
			gen.append(buf, n);
			offset += n;
			handle->advance_seek(n);
		}
	}
};


struct Xml_term_edit::Add_command : Xml_term_edit::Command
{
	Add_command(Command_registry     &cmds,
	            Vfs::Dir_file_system &vfs,
	            Genode::Allocator    &alloc,
	            Reporter             &report)
	: Command("add", "add a new subsystem to init", cmds, vfs, alloc, report)
	{ }

	void execute(Command_line &cmd, Terminal::Session &terminal) override
	{
		Reporter::Xml_generator gen(report, [&] () {
			gen.node("add", [&] () {
				insert_file_content(cmd, gen); }); });
	}
};


struct Xml_term_edit::Del_command : Xml_term_edit::Command
{
	Del_command(Command_registry     &cmds,
	            Vfs::Dir_file_system &vfs,
	            Genode::Allocator    &alloc,
	            Reporter             &report)
	: Command("del", "delete a subsystem from init", cmds, vfs, alloc, report)
	{ }

	void execute(Command_line &cmd, Terminal::Session &terminal) override
	{
		Reporter::Xml_generator gen(report, [&] () {
			gen.node("remove", [&] () {
				insert_file_content(cmd, gen); }); });
	}
};


struct Xml_term_edit::Exit_command : Cli_monitor::Command
{
	Genode::Parent &parent;

	Exit_command(Command_registry &cmds, Genode::Parent &parent)
	: Cli_monitor::Command("exit", ""), parent(parent) {
		cmds.insert(this); }

	void execute(Command_line &cmd, Terminal::Session &terminal) override {
		parent.exit(0); }
};


struct Xml_term_edit::Main
{
	Genode::Env &env;

	Attached_rom_dataspace config { env, "config" };

	Xml_node vfs_config()
	{
		try {
			return config.xml().sub_node("vfs");
		} catch (Xml_node::Nonexistent_sub_node) {
			warning("no VFS configuration defined");
			return Xml_node("<vfs><fs writeable=\"0\"/></vfs>");
		}
	}

	struct Io_response_handler : Vfs::Io_response_handler
	{
		void handle_io_response(Vfs::Vfs_handle::Context *) override { }
	} io_response_handler;

	Heap heap { env.ram(), env.rm() };

	Vfs::Global_file_system_factory vfs_factory { heap };

	Vfs::Dir_file_system vfs_root {
		env, heap, vfs_config(), io_response_handler, vfs_factory };

	Terminal::Connection term { env, "edit" };

	Reporter reporter { env, "xml_editor", "edit", env.ram().avail_ram().value / 2 };

	Command_registry cmds;

	Cli_monitor::Command *lookup_command(char const *buf)
	{
		Cli_monitor::Token token(buf);
		for (Cli_monitor::Command *curr = cmds.first(); curr; curr = curr->next())
			if (strcmp(token.start(), curr->name().string(), token.len()) == 0
			 && strlen(curr->name().string()) == token.len())
				return curr;
		return nullptr;
	}

	Add_command add_command { cmds, vfs_root, heap, reporter };
	Del_command del_command { cmds, vfs_root, heap, reporter };
	Exit_command exit_command { cmds, env.parent() };

	enum { COMMAND_MAX_LEN = 1024 };
	char cmd_buf[COMMAND_MAX_LEN];

	Line_editor editor {
		"> ", cmd_buf, sizeof(cmd_buf), term, cmds };

	void handle_term();

	Signal_handler<Main> term_handler {
		env.ep(), *this, &Main::handle_term };

	Main(Genode::Env &env) : env(env)
	{
		reporter.enabled(true);
		term.read_avail_sigh(term_handler);
	}
};


void Xml_term_edit::Main::handle_term()
{
	while (term.avail() && !editor.completed()) {
		char c = 0;
		term.read(&c, 1);
		editor.submit_input(c);
	}

	if (editor.completed()) {
		auto *cmd = lookup_command(cmd_buf);
		if (cmd) {
			Cli_monitor::Command_line cmd_line(cmd_buf, *cmd);
			cmd->execute(cmd_line, term);
		}
		editor.reset();
	}
}

void Component::construct(Genode::Env &env)
{
	_env = &env;
	static Xml_term_edit::Main inst(env);
}
