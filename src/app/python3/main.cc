/*
 * \brief  Running python script on Genode
 * \author Johannes Schlatow
 * \date   2010-02-17
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Python includes */
#include <python3/Python.h>

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/env.h>
#include <libc/component.h>
#include <base/log.h>

/* libc includes */
#include <fcntl.h>

namespace Python
{
	struct Rom_watcher;
	struct Main;
}

struct Python::Main
{
	Genode::Env       &_env;

	Genode::Attached_rom_dataspace _config = { _env, "config" };
	Genode::Constructible<Genode::Rom_connection> _update { };


	int _execute()
	{
		enum {
			MAX_NAME_LEN = 128
		};

		char filename[MAX_NAME_LEN];

		Genode::Xml_node script = _config.xml().sub_node("file");
		script.attribute("name").value(filename, sizeof(filename));

		FILE * fp = fopen(filename, "r");

		Genode::log("Starting python ...");
		int res = PyRun_SimpleFile(fp, filename);
		Genode::log("Executed '", Genode::Cstring(filename), "'");

		fclose(fp);
		return res;
	}

	void _initialize()
	{
		enum {
			MAX_NAME_LEN = 128
		};

		wchar_t wbuf[MAX_NAME_LEN];

		if (_config.xml().has_sub_node("pythonpath")) {
			char pythonpath[MAX_NAME_LEN];
			Genode::Xml_node path = _config.xml().sub_node("pythonpath");

			path.attribute("name").value(pythonpath, sizeof(pythonpath));
			mbstowcs(wbuf, pythonpath, strlen(pythonpath));

			Py_SetPath(wbuf);
		}

		if (_config.xml().attribute_value("verbose", false))
			Py_VerboseFlag = 1;

		//don't need the 'site' module
		Py_NoSiteFlag = 1;
		//don't support interactive mode, yet
		Py_InteractiveFlag = 0;
		Py_Initialize();
	}

	void _finalize()
	{
		Py_Finalize();
	}

	void _handle_config()
	{
		_initialize();

		if (_config.xml().has_sub_node("file")) {
			Genode::Xml_node filenode = _config.xml().sub_node("file");
			if (filenode.has_attribute("on-rom-update")) {
				char rom_name[128];
				filenode.attribute("on-rom-update").value(rom_name, sizeof(rom_name));

				_update.construct(_env, rom_name);
				_update->sigh(_trigger_handler);

				if (_update->dataspace().valid())
					_execute();

				return;
			}
			else {
				_execute();
			}
		}
		else {
			Genode::error("Need <file name=\"filename\"> as argument!");
		}

		/* if there was a on-rom-update attribute, we finalize and wait for next config update */
		_finalize();
	}

	void _handle_trigger()
	{
		Libc::with_libc([&] () { _execute(); });
	}

	Genode::Signal_handler<Main> _config_handler {
		_env.ep(), *this, &Main::_handle_config };

	Genode::Signal_handler<Main> _trigger_handler {
		_env.ep(), *this, &Main::_handle_trigger };

	Main(Genode::Env &env) : _env(env)
	{
		_config.sigh(_config_handler);
		_handle_config();
	}
};

void Libc::Component::construct(Libc::Env &env)
{
	Libc::with_libc([&] () { static Python::Main main(env); });
}
