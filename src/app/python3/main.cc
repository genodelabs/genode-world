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
		using namespace Genode;

		typedef String<128> File_name;

		int res = 0;

		Xml_node const config = _config.xml();

		config.with_sub_node("file",
			[&] (Xml_node const &script) {

				if (!script.has_attribute("name")) {
					warning("<file> node lacks 'name' attribute");
					return;
				}

				File_name const file_name = script.attribute_value("name", File_name());

				FILE * fp = fopen(file_name.string(), "r");

				log("Starting python ...");
				res = PyRun_SimpleFile(fp, file_name.string());
				log("Executed '", file_name, "'");

				fclose(fp);
			},
			[&] () { warning("config lacks <file> sub node"); });

		return res;
	}

	void _initialize()
	{
		using namespace Genode;

		Xml_node const config = _config.xml();

		enum { MAX_NAME_LEN = 128 };
		wchar_t wbuf[MAX_NAME_LEN];

		config.with_optional_sub_node("pythonpath", [&] (Xml_node const &pythonpath) {

			typedef String<MAX_NAME_LEN> Path;
			Path const path = pythonpath.attribute_value("name", Path());

			mbstowcs(wbuf, path.string(), ::strlen(path.string()));

			Py_SetPath(wbuf);
		});

		if (config.attribute_value("verbose", false))
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

		_config.update();
		Genode::Xml_node const config = _config.xml();

		config.with_sub_node("file",
			[&] (Genode::Xml_node const &file) {

				if (file.has_attribute("on-rom-update")) {

					typedef Genode::String<128> Rom_name;

					Rom_name const rom_name =
						file.attribute_value("on-rom-update", Rom_name());

					_update.construct(_env, rom_name.string());
					_update->sigh(_trigger_handler);

					if (_update->dataspace().valid())
						_execute();

				} else {
					_execute();
					_finalize();
				}},
			[&] () { Genode::error("Need <file name=\"filename\"> as argument!"); });
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
