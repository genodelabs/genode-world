/*
 * \brief  Transactional XML editor
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
#include <vfs/file_system_factory.h>
#include <vfs/dir_file_system.h>
#include <report_session/report_session.h>
#include <root/component.h>
#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <base/heap.h>
#include <base/sleep.h>
#include <base/log.h>
#include <base/component.h>
#include <util/xml_generator.h>

namespace Xml_editor {
	using namespace Genode;

	typedef Path<Vfs::MAX_PATH_LEN/2> Path;
	typedef Genode::String<64> Name;

	struct Xml_file;
	struct Report_session_component;
	class Report_root_component;
	struct Main;
}

#define REVISION_ATTR_NAME "edit_rev"

Genode::Env *_env;

struct Xml_editor::Xml_file
{
	Genode::Allocator &alloc;
	Vfs::Vfs_handle   &vfs_handle;

	Path const path;
	unsigned revision = 0;


	/*****************************
	 ** Current and next buffer **
	 *****************************/

	struct Buffer {
		char *ptr = nullptr;
		size_t size = 0;
		Buffer *next;
	};

	Buffer yin;
	Buffer yang;

	Buffer *buffer = &yin;

	Buffer &next_buffer() { return *buffer->next; }

	/**
	 * Return the next buffer, zeroed, and reallocated
	 * if necessary
	 */
	Buffer &next_buffer(size_t min_size)
	{
		Buffer &next = *buffer->next;
		if (next.size < min_size) {
			if (next.ptr)
				alloc.free(next.ptr, next.size);
			next.ptr = (char *)alloc.alloc(min_size);
			next.size = min_size;
		}
		memset(next.ptr, 0x00, next.size);
		return next;
	}

	template<typename FUNC>
	Xml_generator generate(char const *type, size_t min_size, FUNC const &fn)
	{
		Buffer &next = next_buffer(min_size);

		return Xml_generator(next.ptr, next.size, type, fn);
	}

	Xml_node xml() const {
		return Xml_node(buffer->ptr, buffer->size); }

	/**
	 * Flush file changes
	 */
	void sync()
	{
		while (true) {
			if (vfs_handle.fs().queue_sync(&vfs_handle))
				break;
			_env->ep().wait_and_dispatch_one_io_signal();
		}
		while (true) {
			if (vfs_handle.fs().complete_sync(&vfs_handle) != Vfs::File_io_service::SYNC_QUEUED)
				break;
			_env->ep().wait_and_dispatch_one_io_signal();
		}
	}

	void read_file()
	{
		using namespace Vfs;

		Directory_service::Stat sb;
		vfs_handle.ds().stat(path.base(), sb);
		file_size const total = sb.size;

		Buffer &next = next_buffer(total ? total : 4096);
		buffer = &next;

		if (total == 0) {
			strncpy(next.ptr, "<config/>", next.size);
		} else {
			/*
			 * Read in one pass, reading in multiple passes is
			 * too complicated and error prone
			 */
			file_size out = 0;
			while (!vfs_handle.fs().queue_read(&vfs_handle, total))
				_env->ep().wait_and_dispatch_one_io_signal();

			for (;;) {
				auto r = vfs_handle.fs().complete_read(
					&vfs_handle, next.ptr, total, out);
				switch (r) {
				case Vfs::File_io_service::Read_result::READ_QUEUED:
					_env->ep().wait_and_dispatch_one_io_signal();
					break;
				case Vfs::File_io_service::Read_result::READ_OK:
					return;
				default:
					Genode::error("failed to read XML file");
					throw r;
				}
			}
		}
	}

	void write_file(Vfs::file_size length)
	{
		using namespace Vfs;

		Buffer &next = next_buffer();

		vfs_handle.fs().ftruncate(&vfs_handle, length);

		file_size offset = 0;
		while (offset < length) {
			vfs_handle.seek(offset);
			file_size n = 0;
			vfs_handle.fs().write(
				&vfs_handle,
				next.ptr + offset,
				length - offset,
				n);
			offset += n;
		}

		buffer = &next;
	}

	Xml_file(Genode::Allocator &alloc,
	         Vfs::Vfs_handle &handle,
	         Path const &path)
	:
		alloc(alloc), vfs_handle(handle), path(path)
	{
		yin.next = &yang;
		yang.next = &yin;

		read_file();

		try {
			Xml_node const editor_node = xml().sub_node("xml_editor");
			revision = editor_node.attribute_value("rev", 0U);
		}
		catch (Xml_node::Nonexistent_sub_node) { }
		catch (Xml_node::Invalid_syntax) {
			Genode::error("invalid XML at '", path, "'");
		}
	}

	~Xml_file()
	{
		if (yin.ptr)
			alloc.free(yin.ptr, yin.size);
		if (yang.ptr)
			alloc.free(yang.ptr, yang.size);
	}

	size_t add(Xml_node const &new_node)
	{
		Xml_node const current = xml();

		Name const new_name =
			new_node.attribute_value("name", Name());

		current.for_each_sub_node(new_node.type().string(),
		                          [&] (Xml_node const &existing_start) {
			if (existing_start.attribute_value("name", Name()) == new_name) {
				error(new_name, " start node already present in config");
			}
		});


		Xml_generator gen = generate(current.type().string(),
		                             current.size()+new_node.size()*2,
		                             [&] () {
			try {
				Xml_attribute attr = current.attribute(0U);
				while (true) {
					auto attr_name = attr.name();
					if (attr_name != REVISION_ATTR_NAME) {
						Genode::String<256> data;
						attr.value(&data);
						gen.attribute(attr_name.string(), data.string());
					}
					attr = attr.next();
				}
			} catch (Xml_node::Nonexistent_attribute) { }

			/* set revision info as a top-level attribute */
			gen.attribute("edit_rev", ++revision);

			/* add new content node */
			gen.append(new_node.addr(), new_node.size());

			try {
				/* add existing nodes */
				Xml_node node = current.sub_node();
				while (true) {
					gen.append(node.addr(), node.size());
					node = node.next();
				}

			} catch (Xml_node::Nonexistent_sub_node) {
			}
			gen.append("\n");
		});

		return gen.used();
	}

	size_t remove(Xml_node const &node)
	{
		Name const remove_name =
			node.attribute_value("name", Name());

		Xml_node const current = xml();

		Xml_generator gen = generate(current.type().string(),
		                             current.size()*4,
		                             [&] () {
			try {
				Xml_attribute attr = current.attribute(0U);
				while (true) {
					auto attr_name = attr.name();
					if (attr_name != REVISION_ATTR_NAME) {
						Genode::String<256> data;
						attr.value(&data);
						gen.attribute(attr_name.string(), data.string());
					}
					attr = attr.next();
				}
			} catch (Xml_node::Nonexistent_attribute) { }

			/* set revision info as a top-level attribute */
			gen.attribute("edit_rev", ++revision);

			current.for_each_sub_node([&] (Xml_node const &node) {
				/* skip the node we are removing */
				auto name = node.attribute_value("name", Name());
				if (name != remove_name)
				{
					gen.append(node.addr(), node.size());
					gen.append("\n");
				}
			});
			gen.append("\n");
		});

		return gen.used();
	}

	size_t toggle(Xml_node const &node)
	{
		Name const toggle_name = node.attribute_value("name", Name());
		Xml_node const current = xml();
		bool name_present = false;

		current.for_each_sub_node([&] (Xml_node const &other) {
			if (!name_present &&
			    other.type() == node.type() &&
			    toggle_name == other.attribute_value("name", Name()))
			{
				name_present = true;
			}
		});

		return name_present ? remove(node) : add(node);
	}
};


struct Xml_editor::Report_session_component : Genode::Rpc_object<Report::Session>
{
	Session_label const label;

	Attached_ram_dataspace ram_ds;

	Xml_file &xml_file;

	bool const verbose = true;

	Report_session_component(Genode::Env &env, size_t buffer_size,
	                         Xml_file &file,
	                         Session_label const &session_label)
	:
		label(session_label),
		ram_ds(env.ram(), env.rm(), buffer_size),
		xml_file(file)
	{ }


	/******************************
	 ** Report session interface **
	 ******************************/

	Dataspace_capability dataspace() override {
		return ram_ds.cap(); }

	void submit(size_t length) override
	{
		size_t content_size = 0;

		auto add_fn = [&] (Xml_node const &action) {
			action.for_each_sub_node([&] (Xml_node const &subnode) {
				if (verbose)
					log("'", label, "' adds '", subnode.attribute_value("name", Name()), "'");
				content_size = xml_file.add(subnode);
			});
		};

		auto remove_fn = [&] (Xml_node const &action) {
			action.for_each_sub_node([&] (Xml_node const &subnode) {
				if (verbose)
					log("'", label, "' removes '", subnode.attribute_value("name", Name()), "'");
				content_size = xml_file.remove(subnode);
			});
		};

		auto toggle_fn = [&] (Xml_node const &action) {
			action.for_each_sub_node([&] (Xml_node const &subnode) {
				if (verbose)
					log("'", label, "' toggles '", subnode.attribute_value("name", Name()), "'");
				content_size = xml_file.toggle(subnode);
			});
		};

		try {
			Xml_node edit_node(ram_ds.local_addr<char const>(), length);

			edit_node.for_each_sub_node("toggle", toggle_fn);
			edit_node.for_each_sub_node("remove", remove_fn);
			edit_node.for_each_sub_node("add", add_fn);
			if (content_size) {
				xml_file.write_file(content_size);
				xml_file.sync();
			}
		} catch (Xml_node::Invalid_syntax) {
			error("invalid XML received from '", label, "'");
		} catch (Genode::Xml_generator::Buffer_exceeded) {
			error("Genode::Xml_generator::Buffer_exceeded");
		} catch (...) {
			error("failed to process action from '", label, "'");
			throw;
		}
	}

	void response_sigh(Signal_context_capability) override
	{
		warning(__func__, " not implemented");
	}

	size_t obtain_response()
	{
		warning(__func__, " not implemented");
		return 0;
	}
};


class Xml_editor::Report_root_component :
	public Genode::Root_component<Report_session_component>
{
	private:

		Genode::Env &_env;

		Attached_rom_dataspace &_config;

		Xml_file &_xml_file;

	protected:

		Report_session_component *_create_session(char const *args) override
		{
			using namespace Genode;

			size_t const ram_quota =
				Arg_string::find_arg(args, "ram_quota").aligned_size();

			/* read report buffer size from session arguments */
			size_t const buffer_size =
				Arg_string::find_arg(args, "buffer_size").aligned_size();

			size_t const session_size =
				max(sizeof(Report_session_component), 4096U) + buffer_size;

			Session_label const label = label_from_args(args);

			if (ram_quota < session_size) {
				Genode::error("insufficient ram donation from ", label);
				throw Insufficient_ram_quota();
			}

			if (buffer_size == 0) {
				Genode::error("zero-length report requested by ", label);
				throw Service_denied();
			}

			try {
			return new (md_alloc())
				Xml_editor::Report_session_component(
					_env, buffer_size, _xml_file, label);
			}
			catch (Out_of_ram)             { error("Out_of_ram"); }
			catch (Out_of_caps)            { error("Out_of_caps"); }
			catch (Service_denied)         { error("Service_denied"); }
			catch (Insufficient_cap_quota) { error("Insufficient_cap_quota"); }
			catch (Insufficient_ram_quota) { error("Insufficient_ram_quota"); }
			throw ~0;
		}

	public:

		Report_root_component(Genode::Env &env,
		                      Genode::Allocator &md_alloc,
		                      Attached_rom_dataspace &config,
		                      Xml_file &file)
		:
			Root_component<Report_session_component>(env.ep(), md_alloc),
			_env(env), _config(config), _xml_file(file)
		{ }
};


struct Xml_editor::Main
{
	Genode::Env &env;

	Attached_rom_dataspace config { env, "config" };

	Heap heap { env.ram(), env.rm() };

	void die(char const *msg)
	{
		error(msg);
		env.parent().exit(~0);
		sleep_forever();
	}

	/*********
	 ** VFS **
	 *********/

	Xml_node vfs_config()
	{
		try {
			return config.xml().sub_node("vfs");
		} catch (Xml_node::Nonexistent_sub_node) {
			/* XXX: spin for config update? */
			die("no VFS configuration defined");
			throw;
		}
	}

	struct Io_response_handler : Vfs::Io_response_handler
	{
		void handle_io_response(Vfs::Vfs_handle::Context *) override { }
	} io_response_handler;

	Vfs::Global_file_system_factory vfs_factory { heap };

	Vfs::Dir_file_system vfs_root {
		env, heap, vfs_config(), io_response_handler, vfs_factory };

	/* Handle on the output file */
	Vfs::Vfs_handle *vfs_handle;


	/************************
	 ** Init configuration **
	 ************************/

	Path parse_xml_file_path()
	{
		try {
			/* get user string */
			Genode::String<Path::capacity()> raw;
			Xml_attribute attr = config.xml().attribute("output");
			attr.value(&raw);
			/* return canonicalized path */
			return Path(raw.string());
		} catch (Xml_node::Nonexistent_attribute) {
			/* XXX: spin for config update? */
			die("output file must be defined with <config output=\"...\"/>");
			throw;
		}
	}

	Path const xml_file_path = parse_xml_file_path();

	Vfs::Vfs_handle &open_output_handle()
	{
		using namespace Vfs;
		typedef Directory_service::Open_result Open_result;

		unsigned mode = Directory_service::OPEN_MODE_RDWR;

		Vfs_handle *handle;

		Open_result res = vfs_root.open(xml_file_path.base(), mode, &handle, heap);
		if (res == Open_result::OPEN_ERR_UNACCESSIBLE) {
			mode |= Directory_service::OPEN_MODE_CREATE;
			res = vfs_root.open(xml_file_path.base(), mode, &handle, heap);
		}

		switch (res) {
		case Open_result::OPEN_OK:
			return *handle;

		case Open_result::OPEN_ERR_UNACCESSIBLE:
			die("OPEN_ERR_UNACCESSIBLE"); break;
		case Open_result::OPEN_ERR_NO_PERM:
			die("OPEN_ERR_NO_PERM"); break;
		case Open_result::OPEN_ERR_EXISTS:
			die("OPEN_ERR_EXISTS"); break;
		case Open_result::OPEN_ERR_NAME_TOO_LONG:
			die("OPEN_ERR_NAME_TOO_LONG"); break;
		case Open_result::OPEN_ERR_NO_SPACE:
			die("OPEN_ERR_NO_SPACE"); break;
		case Open_result::OPEN_ERR_OUT_OF_RAM:
			die("OPEN_ERR_OUT_OF_RAM"); break;
		case Open_result::OPEN_ERR_OUT_OF_CAPS:
			die("OPEN_ERR_OUT_OF_CAPS"); break;
		}
		throw ~0;
	}

	Xml_file xml_file { heap, open_output_handle(), xml_file_path };

	Sliced_heap report_heap { env.ram(), env.rm() };

	Report_root_component report_root { env, report_heap, config, xml_file };

	Main(Genode::Env &env) : env(env)
	{
		env.parent().announce(env.ep().manage(report_root));
	}
};


void Component::construct(Genode::Env &env)
{
	_env = &env;
	static Xml_editor::Main inst(env);
}
