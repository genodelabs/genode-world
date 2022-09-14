/**
 * \brief  Lwext4 file system interface implementation
 * \author Josef Soentgen
 * \date   2017-08-01
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/heap.h>
#include <file_system/util.h>
#include <file_system_session/rpc_object.h>
#include <os/session_policy.h>
#include <root/component.h>

/* library includes */
#include <lwext4/init.h>
#include <ext4.h>

/* local includes */
#include <directory.h>
#include <file.h>
#include <file_system.h>
#include <open_node.h>
#include <symlink.h>

namespace Lwext4_fs {

	using File_system::Packet_descriptor;
	using File_system::Path;

	struct Main;
	struct Root;
	struct Session_component;
}

class Lwext4_fs::Session_component : public File_system::Session_rpc_object
{
	private:

		typedef File_system::Open_node<Node> Open_node;

		Genode::Env &_env;

		Allocator                   &_md_alloc;
		Directory                   &_root;
		Id_space<File_system::Node>  _open_node_registry;
		bool                         _writeable;

		Signal_handler<Session_component> _process_packet_handler;

		Genode::Reporter _stats_reporter { _env , "file_system_stats", "stats" };

		/******************************
		 ** Packet-stream processing **
		 ******************************/

		/**
		 * Perform packet operation
		 *
		 * \return true on success, false on failure
		 */
		void _process_packet_op(Packet_descriptor &packet, Open_node &open_node)
		{
			void     * const content = tx_sink()->packet_content(packet);
			size_t     const length  = packet.length();

			/* resulting length */
			size_t res_length = 0;
			bool succeeded = false;

			switch (packet.operation()) {

			case Packet_descriptor::READ:
				if (content && (packet.length() <= packet.size())) {
						try {
							res_length = open_node.node().read((char *)content, length,
							                                   packet.position());
							succeeded = res_length;
						} catch (Node::Eof) { succeeded = true; }
				}
				break;

			case Packet_descriptor::WRITE:
				if (content && (packet.length() <= packet.size())) {
					res_length = open_node.node().write((char const *)content,
					                                    length, packet.position());
					if (res_length != length) {
						Genode::error("partial write detected ",
						              res_length, " vs ", length);
						/* do not acknowledge */
						return;
					}
					succeeded = true;
				}
				break;

			case Packet_descriptor::WRITE_TIMESTAMP:
				if (tx_sink()->packet_valid(packet) && (packet.length() <= packet.size())) {

					packet.with_timestamp([&] (File_system::Timestamp const time) {
						open_node.node().update_modification_time(time);
						succeeded = true;
					});
				}
				break;

			case Packet_descriptor::CONTENT_CHANGED:
				open_node.register_notify(*tx_sink());
				/* notify_listeners may bounce the packet back*/
				open_node.node().notify_listeners();
				/* otherwise defer acknowledgement of this packet */
				return;

			case Packet_descriptor::READ_READY:
				succeeded = true;
				/* not supported */
				break;

			case Packet_descriptor::SYNC:
				/* for future failure handling */
				try         { File_system::sync(); }
				catch (...) { }

				File_system::stats_update(_stats_reporter);
				succeeded = true;
				break;
			}

			packet.length(res_length);
			packet.succeeded(succeeded);
			tx_sink()->acknowledge_packet(packet);
		}

		void _process_packet()
		{
			Packet_descriptor packet = tx_sink()->get_packet();

			/* assume failure by default */
			packet.succeeded(false);

			auto process_packet_fn = [&] (Open_node &open_node) {
				_process_packet_op(packet, open_node);
			};

			try {
				_open_node_registry.apply<Open_node>(packet.handle(), process_packet_fn);
			} catch (Id_space<File_system::Node>::Unknown_id const &) {
				Genode::error("Invalid_handle");
				tx_sink()->acknowledge_packet(packet);
			}
		}

		void _process_packets()
		{
			while (tx_sink()->packet_avail()) {

				if (!tx_sink()->ready_to_ack())
					return;

				_process_packet();
			}
		}

		static void _assert_valid_path(char const *path)
		{
			if (!path || path[0] != '/')
				throw Lookup_failed();
		}

	public:

		/**
		 * Constructor
		 */
		Session_component(Genode::Env &env,
		                  size_t       tx_buf_size,
		                  char const  *root_dir,
		                  bool         writeable,
		                  Allocator   &md_alloc,
		                  bool         report_stats)
		:
			Session_rpc_object(env.ram().alloc(tx_buf_size), env.rm(), env.ep().rpc_ep()),
			_env(env),
			_md_alloc(md_alloc),
			_root(*new (&_md_alloc) Directory(root_dir, false)),
			_writeable(writeable),
			_process_packet_handler(env.ep(), *this, &Session_component::_process_packets)
		{
			_tx.sigh_packet_avail(_process_packet_handler);
			_tx.sigh_ready_to_ack(_process_packet_handler);

			_stats_reporter.enabled(report_stats);
		}

		/**
		 * Destructor
		 */
		~Session_component()
		{
			Dataspace_capability ds = tx_sink()->dataspace();
			_env.ram().free(static_cap_cast<Ram_dataspace>(ds));
			destroy(&_md_alloc, &_root);
		}

		/***************************
		 ** File_system interface **
		 ***************************/

		File_handle file(Dir_handle dir_handle, Name const &name, Mode mode, bool create)
		{
			if (!valid_name(name.string()))
				throw Invalid_name();

			auto file_fn = [&] (Open_node &open_node) {

				Node &dir = open_node.node();

				if (!_writeable) {
					if (create || (mode != STAT_ONLY && mode != READ_ONLY)) {
						throw Permission_denied();
					}
				}

				/* should already contain _root_dir */
				Absolute_path absolute_path(dir.name());

				try {
					absolute_path.append("/");
					absolute_path.append(name.string());
				} catch (Path_base::Path_too_long) {
					throw Invalid_name();
				}

				File *file = new (&_md_alloc) File(absolute_path.base(),
				                                   mode, create);

				Open_node *open_file =
					new (&_md_alloc) Open_node(*file, _open_node_registry);

				return open_file->id();
			};

			try {
				return File_handle {
					_open_node_registry.apply<Open_node>(dir_handle, file_fn).value
				};
			} catch (Id_space<File_system::Node>::Unknown_id const &) {
				throw Invalid_handle();
			}
		}

		Symlink_handle symlink(Dir_handle dir_handle, Name const &name, bool create)
		{
			if (!valid_name(name.string())) { throw Invalid_name(); }

			auto file_fn = [&] (Open_node &open_node) {

				Node &dir = open_node.node();

				if (!_writeable && create) { throw Permission_denied(); }

				Absolute_path absolute_path(_root.name());

				try {
					absolute_path.append(dir.name());
					absolute_path.append("/");
					absolute_path.append(name.string());
				} catch (Path_base::Path_too_long) {
					throw Invalid_name();
				}

				Symlink *link = new (&_md_alloc) Symlink(absolute_path.base(), create);

				Open_node *open_file =
					new (&_md_alloc) Open_node(*link, _open_node_registry);

				return open_file->id();
			};

			try {
				return Symlink_handle {
					_open_node_registry.apply<Open_node>(dir_handle, file_fn).value
				};
			} catch (Id_space<File_system::Node>::Unknown_id const &) {
				throw Invalid_handle();
			}
		}

		Dir_handle dir(Path const &path, bool create)
		{
			char const *path_str = path.string();
			_assert_valid_path(path_str);

			/* skip leading '/' */
			path_str++;

			if (!_writeable && create)
				throw Permission_denied();

			if (!path.valid_string())
				throw Name_too_long();

			Absolute_path absolute_path(_root.name());

			try {
				absolute_path.append(path_str);
				absolute_path.remove_trailing('/');
			} catch (Path_base::Path_too_long) {
				throw Name_too_long();
			}

			Directory *dir = new (&_md_alloc) Directory(absolute_path.base(), create);

			Open_node *open_dir =
				new (_md_alloc) Open_node(*dir, _open_node_registry);

			return Dir_handle { open_dir->id().value };
		}

		Node_handle node(Path const &path)
		{
			char const *path_str = path.string();

			_assert_valid_path(path_str);

			Absolute_path absolute_path(_root.name());

			try {
				absolute_path.append(path.string());
				absolute_path.remove_trailing('/');
			} catch (Path_base::Path_too_long) {
				throw Lookup_failed();
			}

			try {
				Node *node = new (&_md_alloc) Node(absolute_path.base());

				Open_node *open_node =
					new (_md_alloc) Open_node(*node, _open_node_registry);

				return open_node->id();
			} catch (...) { throw; }
		}

		void close(Node_handle handle)
		{
			auto close_fn = [&] (Open_node &open_node) {

				try {
					Absolute_path absolute_path(_root.name());
					absolute_path.append(open_node.node().name());
				} catch (Path_base::Path_too_long) { }
				Node &node = open_node.node();
				destroy(_md_alloc, &open_node);
				destroy(_md_alloc, &node);
			};

			try {
				_open_node_registry.apply<Open_node>(handle, close_fn);
			} catch (Id_space<File_system::Node>::Unknown_id const &) {
				throw Invalid_handle();
			}
		}

		Status status(Node_handle node_handle)
		{
			auto status_fn = [&] (Open_node &open_node) {
				return open_node.node().status();
			};

			try {
				return _open_node_registry.apply<Open_node>(node_handle, status_fn);
			} catch (Id_space<File_system::Node>::Unknown_id const &) {
				throw Invalid_handle();
			}
		}

		unsigned num_entries(Dir_handle dir_handle) override
		{
			auto fn = [&] (Open_node &dir_node) {
				return dir_node.node().num_entries();
			};

			try {
				return _open_node_registry.apply<Open_node>(dir_handle, fn);
			} catch (Id_space<File_system::Node>::Unknown_id const &) {
				throw Invalid_handle();
			}
		}

		void control(Node_handle, Control) override { }

		void unlink(Dir_handle dir_handle, Name const &name)
		{
			if (!valid_name(name.string()))
				throw Invalid_name();

			if (!_writeable)
				throw Permission_denied();

			auto unlink_fn = [&] (Open_node &open_node) {

				Absolute_path absolute_path(_root.name());

				try {
					absolute_path.append(open_node.node().name());
					absolute_path.append("/");
					absolute_path.append(name.string());
				} catch (Path_base::Path_too_long) {
					throw Invalid_name();
				}

				/* XXX do not call ext4_ functions directly */
				{
					struct ext4_inode _inode;
					unsigned int      _ino;
					int err = ext4_raw_inode_fill(absolute_path.base(), &_ino, &_inode);
					/* silent error because the look up is allowed to fail */
					if (err) { throw Lookup_failed(); }

					unsigned int const v = _inode.mode & 0xf000;
					switch (v) {
					case EXT4_INODE_MODE_DIRECTORY:
						err = ext4_dir_rm(absolute_path.base());
						break;
					case EXT4_INODE_MODE_FILE:
					default:
						err = ext4_fremove(absolute_path.base());
						break;
					}
					if (err) {
						Genode::error("unlink: error: ", err);
						throw Invalid_name();
					}
				}
			};

			try {
				_open_node_registry.apply<Open_node>(dir_handle, unlink_fn);
			} catch (Id_space<File_system::Node>::Unknown_id const &) {
				throw Invalid_handle();
			}
		}

		void truncate(File_handle file_handle, file_size_t size)
		{
			if (!_writeable)
				throw Permission_denied();

			auto truncate_fn = [&] (Open_node &open_node) {
				open_node.node().truncate(size);
			};

			try {
				_open_node_registry.apply<Open_node>(file_handle, truncate_fn);
			} catch (Id_space<File_system::Node>::Unknown_id const &) {
				throw Invalid_handle();
			}
		}

		void move(Dir_handle from_dir_handle, Name const &from_name,
		          Dir_handle   to_dir_handle, Name const   &to_name)
		{
			if (!_writeable) { throw Permission_denied(); }

			auto move_fn = [&] (Open_node &open_from_dir_node) {

				auto inner_move_fn = [&] (Open_node &open_to_dir_node) {

					Node &from_dir = open_from_dir_node.node();
					Node &to_dir = open_to_dir_node.node();

					char const *from_str = from_name.string();
					char const   *to_str =   to_name.string();

					Absolute_path absolute_from_path(_root.name());
					Absolute_path absolute_to_path(_root.name());

					try {
						absolute_from_path.append(from_dir.name());
						absolute_from_path.append("/");
						absolute_from_path.append(from_name.string());
						absolute_to_path.append(to_dir.name());
						absolute_to_path.append("/");
						absolute_to_path.append(to_name.string());
					} catch (Path_base::Path_too_long) {
						throw Invalid_name();
					}

					char const *from_base = absolute_from_path.base();
					char const *to_base = absolute_to_path.base();

					if (!(valid_name(from_str) && valid_name(to_str)))
						throw Lookup_failed();

					/* lwext4 will complain if target and source are the same */
					int const err = ext4_frename(from_base, to_base);
					if (err && err != EEXIST) {
						Genode::error("move: error: ", err);
						throw Permission_denied();
					}
				};

				try {
					_open_node_registry.apply<Open_node>(to_dir_handle, inner_move_fn);
				} catch (Id_space<File_system::Node>::Unknown_id const &) {
					throw Invalid_handle();
				}
			};

			try {
				_open_node_registry.apply<Open_node>(from_dir_handle, move_fn);
			} catch (Id_space<File_system::Node>::Unknown_id const &) {
				throw Invalid_handle();
			}
		}
};

class Lwext4_fs::Root : public Root_component<Session_component>
{
	private:

		Genode::Env &_env;

		int _sessions { 0 };

		bool _report_stats { false };
		bool _verbose      { false };

		Genode::Attached_rom_dataspace _config_rom { _env, "config" };

		Genode::Signal_handler<Lwext4_fs::Root> _config_sigh {
			_env.ep(), *this, &Lwext4_fs::Root::_handle_config_update };

		void _handle_config_update()
		{
			_config_rom.update();

			if (!_config_rom.valid()) { return; }

			Genode::Xml_node config = _config_rom.xml();

			_verbose = config.attribute_value("verbose", false);

			try {
				_report_stats = config.sub_node("report")
				                      .attribute_value("stats", false);
			} catch (...) { }
		}


	protected:

		Session_component *_create_session(const char *args)
		{
			if (!_config_rom.valid()) {
				Genode::error("no valid config found");
				throw Service_denied();
			}

			using namespace Genode;

			/*
			 * Determine client-specific policy defined implicitly by
			 * the client's label.
			 */

			Genode::Path<MAX_PATH_LEN> session_root;
			bool writeable = false;

			Session_label const label = label_from_args(args);

			size_t ram_quota =
				Arg_string::find_arg(args, "ram_quota").aligned_size();
			size_t tx_buf_size =
				Arg_string::find_arg(args, "tx_buf_size").aligned_size();

			if (!tx_buf_size)
				throw Service_denied();

			if (tx_buf_size > ram_quota) {
				Genode::error("insufficient 'ram_quota' from ", label.string(),
				              " got ", ram_quota, "need ", tx_buf_size);
				throw Insufficient_ram_quota();
			}

			Session_policy policy(label, _config_rom.xml());

			/* determine policy root offset */
			{
				typedef String<MAX_PATH_LEN> Tmp;
				Tmp const tmp = policy.attribute_value("root", Tmp());
				if (tmp.valid())
					session_root.import(tmp.string(), "/mnt");
			}

			/*
			 * Determine if the session is writeable.
			 * Policy overrides client argument, both default to false.
			 */
			if (policy.attribute_value("writeable", false))
				writeable = Arg_string::find_arg(args, "writeable").bool_value(false);

			/* apply client's root offset */
			{
				char tmp[MAX_PATH_LEN];
				Arg_string::find_arg(args, "root").string(tmp, sizeof(tmp), "/");
				if (Genode::strcmp("/", tmp, sizeof(tmp))) {
					session_root.append("/");
					session_root.append(tmp);
				}
			}
			session_root.remove_trailing('/');

			char const *root_dir = session_root.base();

			try {
				if (++_sessions == 1) {
					File_system::mount_fs(_config_rom.xml());
				}
			} catch (...) { throw Service_denied(); }

			try {
				return new (md_alloc())
					Session_component(_env, tx_buf_size, root_dir, writeable, *md_alloc(),
					                  _report_stats);

			} catch (Lookup_failed) {
				Genode::error("File system root directory \"", root_dir, "\" does not exist");
				throw Service_denied();
			}
		}

		void _destroy_session(Session_component *session)
		{
			Genode::destroy(md_alloc(), session);

			try {
				if (--_sessions == 0) { File_system::unmount_fs(); }
			} catch (...) { }
		}


	public:

		/**
		 * Constructor
		 */
		Root(Genode::Env &env, Allocator &md_alloc)
		:
			Root_component<Session_component>(env.ep(), md_alloc),
			_env(env)
		{
			_config_rom.sigh(_config_sigh);
			_handle_config_update();
		}
};


struct Lwext4_fs::Main
{
	Genode::Env &_env;

	Heap _heap { _env.ram(), _env.rm() };

	Sliced_heap _sliced_heap { _env.ram(), _env.rm() };

	Root fs_root { _env, _sliced_heap };

	Main(Genode::Env &env) : _env(env)
	{
		Lwext4::malloc_init(_env, _heap);

		ext4_blockdev *bd = Lwext4::block_init(_env, _heap);
		File_system::init(bd);

		env.parent().announce(env.ep().manage(fs_root));
		Genode::log("--- lwext4 started ---");
	}
};


void Component::construct(Genode::Env &env)
{
	env.exec_static_constructors();

	static Lwext4_fs::Main inst(env);
}
