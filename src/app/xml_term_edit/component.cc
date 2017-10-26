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
#include <terminal_session/connection.h>
#include <base/attached_rom_dataspace.h>
#include <base/heap.h>
#include <libc/component.h>

/* Cli_monitor includes */
#include <command_line.h>
#include <line_editor.h>

/* Libc includes */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

namespace Xml_term_edit {
	using namespace Genode;
	using namespace Cli_monitor;

	struct Command;
	struct Add_command;
	struct Del_command;
	struct Exit_command;
	struct Main;
}

struct Xml_term_edit::Command : Cli_monitor::Command
{
	Genode::Allocator    &alloc;
	Reporter             &report;

	Command(char const *name,
	        char const *desc,
	        Command_registry     &cmds,
	        Genode::Allocator    &alloc,
	        Reporter             &report)
	:
		Cli_monitor::Command(name, desc),
		alloc(alloc), report(report)
	{
		cmds.insert(this);
	}

	/**
	 * TODO: this is too slow, cache it
	 */
	void _for_each_argument(Argument_fn const &fn) const override
	{
		DIR *dirp = opendir("/");
		if (dirp == NULL) {
			Genode::error("failed to read root directory");
			return;
		}

		dirent *dp;
		while ((dp = readdir(dirp)) != NULL) {
			if (dp->d_type == DT_REG) {
				fn(Argument(dp->d_name, ""));
			}
		}
		closedir(dirp);
	}

	void insert_file_content(Command_line &cmd, Xml_generator gen)
	{
		Path<128> path;
		{
			char name[128] = { '\0' };
			if (cmd.argument(0, name, sizeof(name)) == false) {
				error("Error: no configuration name specified\n");
				return;
			}
			path.import(name);
		}

		int fd = open(path.base(), O_RDONLY);
		if (fd == -1) {
			Genode::error("failed to open '", path, "'");
			return;
		}

		char buf[1024];
		for (;;) {
			auto n = read(fd, buf, sizeof(buf));
			if (n > 0) {
				gen.append(buf, n);
			} else {
				if (n < 0)
					Genode::error("failed to read '", path, "'");
				break;
			}
		}
		close(fd);
	}
};


struct Xml_term_edit::Add_command : Xml_term_edit::Command
{
	Add_command(Command_registry     &cmds,
	            Genode::Allocator    &alloc,
	            Reporter             &report)
	: Command("add", "add a new subsystem to init", cmds, alloc, report)
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
	            Genode::Allocator    &alloc,
	            Reporter             &report)
	: Command("del", "delete a subsystem from init", cmds, alloc, report)
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

	Heap heap { env.ram(), env.rm() };

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

	Add_command add_command { cmds, heap, reporter };
	Del_command del_command { cmds, heap, reporter };
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
	Libc::with_libc([&] () {

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
	});
}

void Libc::Component::construct(Libc::Env &env)
{
	Libc::with_libc([&] () {
		static Xml_term_edit::Main inst(env);
	});
}
