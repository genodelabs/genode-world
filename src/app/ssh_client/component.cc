/*
 * \brief  SSH client as a Terminal client
 * \author Prashanth Mundkur
 * \author Emery Hemingway
 * \date   2018-06-18
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <libc/select.h>
#include <terminal_session/connection.h>
#include <libc/component.h>
#include <base/sleep.h>
#include <base/node.h>

/* Libssh includes */
#include <libssh/libssh.h>
#include <libssh/callbacks.h>

/* Libc includes */
#include <stdio.h>
#include <stdlib.h>


namespace Ssh_client {
	using namespace Genode;
	struct Main;

	typedef Genode::String<64> String;

	String string_attr(Node const &node,
	                   char const *key, char const *def = "") {
		return node.attribute_value(key, String(def)); }
}


struct Ssh_client::Main
{
	Libc::Env &_env;

	Terminal::Connection _terminal { _env };

	Genode::Signal_handler<Main> _terminal_handler {
		_env.ep(), *this, &Main::_handle_terminal };

	Genode::Signal_handler<Main> _size_handler {
		_env.ep(), *this, &Main::_handle_size };

	Libc::Select_handler<Main> _select_handler {
		*this, &Main::_select_ready };

	ssh_session _session = ssh_new();
	ssh_channel _channel = NULL;

	typedef Genode::String<128> String;
	String _hostname { };
	String _password { };

	/* must the host be known */
	bool _host_known = true;

	void _exit(int code)
	{
		if (_session) {
			if (_channel) {
				ssh_channel_free(_channel);
			}
			ssh_free(_session);
		}

		ssh_finalize();
		_env.parent().exit(code);
		Genode::sleep_forever();
	}

	void _die()
	{
		if (_session)
			Genode::error(ssh_get_error(_session));
		_exit(~0);
	}

	void _handle_terminal()
	{
		Libc::with_libc([&] () {
			while (_terminal.avail()) {
				char buf[256];
				size_t n = _terminal.read(buf, sizeof(buf));
				ssh_channel_write(_channel, buf, n);
			}
		});
	}

	void _handle_size()
	{
		Libc::with_libc([&] () {
			auto size = _terminal.size();
			if (size.columns()*size.lines() == 0) {
				/* shutdown if terminal is resized to zero */
				ssh_channel_close(_channel);
				ssh_disconnect(_session);
				_exit(0);
			} else {
				ssh_channel_change_pty_size(_channel, size.columns(), size.lines());
			}
		});
	}

	void _handle_channel(int nready)
	{
		Libc::with_libc([&] () {
			char buffer[256];

			fd_set readfds;
			fd_set noop;

			if (ssh_channel_is_eof(_channel)) _exit(0);

			while (nready) {
				while (true) {
					int n = ssh_channel_read_nonblocking(_channel, buffer, sizeof(buffer), 0);
					if (!n) break;
					if (n < 0) _die();
					_terminal.write(buffer, n);
				}

				FD_ZERO(&noop);
				FD_ZERO(&readfds);
				FD_SET(ssh_get_fd(_session), &readfds);

				nready = _select_handler.select(ssh_get_fd(_session)+1, readfds, noop, noop);
			}
		});
	}

	void _select_ready(int nready, fd_set const &readfds, fd_set const &writefds, fd_set const &exceptfds)
	{
		_handle_channel(nready);
	}

	static void _log_host_usage()
	{
		char buf[1024];
		Generator::generate({ buf, sizeof(buf) }, "host",
			[&] (Generator &g) {
				g.attribute("name", "...");
				g.attribute("port", 22);
				g.attribute("user", "...");
				g.attribute("pass", "...");
				g.attribute("known", "yes");
		}).with_result(
			[&] (size_t used) {
				log("host file format: ", Node(Const_byte_range_ptr(buf, used))); },
			[&] (Buffer_error) {
				warning("host info exceeds maximum buffer size");
		});
	}

	void _configure()
	{
		using namespace Genode;

		_env.with_config([&] (Node const &config) {
			int verbosity = config.attribute_value("verbose", false)
				? SSH_LOG_FUNCTIONS : SSH_LOG_NOLOG;
			ssh_options_set(_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
		});

		/* read all files from the root directory */
		ssh_options_set(_session, SSH_OPTIONS_SSH_DIR, "/");
		ssh_options_set(_session, SSH_OPTIONS_KNOWNHOSTS, "/known_hosts");

		/* read the XML formatted host file */
		char buf[4096];
		FILE *f = fopen("host", "r");
		if (f == NULL) {
			error("failed to open \"/host\" configuration file");
			_log_host_usage();
			_exit(~0);
		}
		size_t n = fread(buf,	1, sizeof(buf), f);
		fclose(f);

		Node const host_cfg(Const_byte_range_ptr(buf, n));
		if (!host_cfg.has_attribute("name")) {
			error("failed to parse host configuration");
			error(host_cfg);
			_log_host_usage();
			_exit(~0);
		}
		_hostname = host_cfg.attribute_value("name", String());
		ssh_options_set(_session, SSH_OPTIONS_HOST,
			_hostname.string());
		ssh_options_set(_session, SSH_OPTIONS_PORT_STR,
			string_attr(host_cfg, "port").string());
		ssh_options_set(_session, SSH_OPTIONS_USER,
			string_attr(host_cfg, "user").string());
		_password = host_cfg.attribute_value("pass", String());
		_host_known = host_cfg.attribute_value("known", true);
	}

	void _authenticate_public_key()
	{
		int rc = SSH_AUTH_AGAIN;
		while (rc == SSH_AUTH_AGAIN) {
			rc = ssh_userauth_publickey_auto(_session, NULL, NULL);
			switch (rc) {
			case SSH_AUTH_SUCCESS:
				Genode::log("public key authentication successful"); return;
			case SSH_AUTH_ERROR:
				Genode::error("public key authentication failed"); break;
			case SSH_AUTH_DENIED:
				Genode::error("public key authentication denied"); break;
			case SSH_AUTH_PARTIAL:
				Genode::error("additional authentication is required"); break;
			case SSH_AUTH_AGAIN:
			default:
				break;
			}
		}

		if (ssh_userauth_password(_session, NULL, _password.string()) == SSH_OK) {
			Genode::log("password authentication successful");
			return;
		}

		Genode::error("password authentication denied");
		if (ssh_userauth_none(_session, NULL) == SSH_OK) {
			Genode::log("anonymous authentication successful");
			return;
		}

		_die();
	}

	void _connect()
	{
		if (ssh_connect(_session) != SSH_OK) _die();

		{
			ssh_key hostkey = NULL;
			ssh_get_publickey(_session, &hostkey);
			{
				unsigned char *hash = NULL;
				size_t         hashlen = 0;
				ssh_get_publickey_hash(hostkey, SSH_PUBLICKEY_HASH_SHA1,
				                       &hash, &hashlen);
				{
					char *hexhost = ssh_get_hexa(hash, hashlen);
					log(_hostname, " ", (char const *)hexhost);
					ssh_string_free_char(hexhost);
				}
				ssh_clean_pubkey_hash(&hash);
			}
			ssh_key_free(hostkey);
		}

		if (!ssh_is_server_known(_session)) {
			if (_host_known) {
				error("unknown host");
				_exit(~0);
			} else {
				ssh_write_knownhost(_session);
			}
		}

		_authenticate_public_key();

		if (char *banner = ssh_get_issue_banner(_session)) {
			Genode::log((const char *)banner);
			free(banner);
		}

		_channel = ssh_channel_new(_session);
		if (_channel == NULL) _die();

		if (ssh_channel_open_session(_channel) != SSH_OK) _die();

		auto size = _terminal.size();
		if (ssh_channel_request_pty_size(_channel, "screen",
		                                 size.columns(),
		                                 size.lines())  != SSH_OK) _die();

		if (ssh_channel_request_shell(_channel) != SSH_OK) _die();
	}

	Main(Libc::Env &env) : _env(env)
	{
		if (!_session) {
			Genode::error("failed to initialize libssh session");
			_die();
		}

		_terminal.read_avail_sigh(_terminal_handler);
		_terminal.size_changed_sigh(_size_handler);

		_configure();
		_connect();

		_handle_channel(1);
		_handle_terminal();
	}

	~Main()
	{
		ssh_free(_session);
	}
};


static void log_callback(int priority,
                         const char *function,
                         const char *buffer,
                         void *userdata)
{
	(void)userdata;
	(void)function;
	Genode::log(buffer);
}


void Libc::Component::construct(Libc::Env &env)
{
	with_libc([&] () {
		Genode::log("libssh ", ssh_version(0));

		ssh_set_log_callback(log_callback);
		ssh_init();

		static Ssh_client::Main main(env);
	});
}
