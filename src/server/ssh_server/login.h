/*
 * \brief  Component providing a Terminal session via SSH
 * \author Josef Soentgen
 * \author Pirmin Duss
 * \author Tomasz Gajewski
 * \date   2019-05-29
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 * Copyright (C) 2019-2021 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SSH_TERMINAL_LOGIN_H_
#define _SSH_TERMINAL_LOGIN_H_

/* Genode includes */
#include <util/string.h>
#include <base/heap.h>
#include <base/registry.h>

/* libssh includes */
#include <libssh/libssh.h>

/* local includes */
#include "util.h"


namespace Ssh {

	using namespace Genode;
	using namespace Util;

	using User     = String<32>;
	using Password = String<64>;
	using Hash     = String<65>;

	using Terminal_name = String<64>;

	struct Login;
	struct Login_registry;
}


struct Ssh::Login : Genode::Registry<Ssh::Login>::Element
{
	Ssh::User     user         { };
	Ssh::Password password     { };
	Ssh::Hash     pub_key_hash { };
	ssh_key       pub_key      { nullptr };
	bool allow_terminal        { false };
	bool allow_sftp            { false };

	Ssh::Terminal_name terminal_name;
	bool multi_login           { false };
	bool request_terminal      { false };

	/**
	 * Constructor
	 */
	Login(Genode::Registry<Login> &reg,
	      Ssh::User      const &user,
	      Ssh::Password  const &pw,
	      Filename       const &pk_file,
	      bool           const allow_terminal,
	      bool           const allow_sftp,
	      Ssh::Terminal_name const &term_name,
	      bool           const multi_login,
	      bool           const request_terminal)
	:
		Element(reg, *this),
		user(user), password(pw),
		allow_terminal(allow_terminal), allow_sftp(allow_sftp),
		terminal_name(term_name),
		multi_login(multi_login), request_terminal(request_terminal)
	{
		Libc::with_libc([&] {

			if (pk_file.valid() &&
			    ssh_pki_import_pubkey_file(pk_file.string(), &pub_key)) {
				Genode::error("could not import public key for user '",
				              user, "'");
			}

			if (pub_key) {
				unsigned char *h    = nullptr;
				size_t         hlen = 0;
				/* pray and assume both calls never fail */
				ssh_get_publickey_hash(pub_key, SSH_PUBLICKEY_HASH_SHA256,
				                       &h, &hlen);
				char const *p = ssh_get_fingerprint_hash(SSH_PUBLICKEY_HASH_SHA256,
				                                         h, hlen);
				if (p) {
					pub_key_hash = Ssh::Hash(p);
				}

				ssh_clean_pubkey_hash(&h);

				/* abuse function to free fingerprint */
				ssh_clean_pubkey_hash((unsigned char**)&p);
			}
		}); /* Libc::with_libc */
	}

	virtual ~Login() { ssh_key_free(pub_key); }

	bool auth_password()  const { return password.valid(); }
	bool auth_publickey() const { return pub_key != nullptr; }

	void print(Genode::Output &out) const
	{
		Genode::print(out, "user ", user, ": ");
		if (auth_password())  { Genode::print(out, "password "); }
		if (auth_publickey()) { Genode::print(out, "public-key"); }
	}
};


struct Ssh::Login_registry : Genode::Registry<Ssh::Login>
{
	Genode::Allocator   &_alloc;
	Util::Pthread_mutex  _mutex { };

	/**
	 * Import one login from node
	 */
	bool _import_single(Genode::Xml_node const &node,
	                    Genode::Xml_node const &main)
	{
		User     user        = node.attribute_value("user", User());
		Password pw          = node.attribute_value("password", Password());
		Filename pub         = node.attribute_value("pub_key", Filename());

		Ssh::Terminal_name const terminal
			= node.attribute_value("terminal", Ssh::Terminal_name());
		bool     allow_term  = terminal.valid();

		bool     allow_sftp  = node.attribute_value("sftp", false);

		int      policies_found = 0;
		bool     multi_login    = false;
		bool     req_term       = false;

		if (!user.valid() || (!pw.valid() && !pub.valid())) {
			Genode::warning("ignoring invalid login: '", user, "'");
			return false;
		}

		if (lookup(user.string())) {
			Genode::warning("ignoring already imported login ", user.string());
			return false;
		}

		if (allow_term) {
			main.for_each_sub_node("policy",
				[&] (Genode::Xml_node const &policy) {
					Ssh::Terminal_name terminal_name
						= policy.attribute_value("terminal_name", Ssh::Terminal_name());
					if (terminal_name == terminal) {
						++policies_found;
						multi_login = policy.attribute_value("multi_login", false);
						req_term = policy.attribute_value("request_terminal", false);
					}
				});
			if (policies_found == 0) {
				Genode::warning("ignoring login: '", user,
				                "' due to policy with terminal name: '", terminal,
				                "' not found");
				return false;
			}
			if (policies_found > 1) {
				Genode::warning("ignoring login: '", user,
				                "' due to multiple policies with terminal name: '",
				                terminal, "' found");
				return false;
			}
		}

		try {
			new (&_alloc) Login(*this, user, pw, pub,
			                    allow_term, allow_sftp,
			                    terminal,
			                    multi_login, req_term);
			return true;
		} catch (...) { return false; }
	}

	void _remove_all()
	{
		for_each([&] (Login &login) {
			Genode::destroy(&_alloc, &login);
		});
	}

	/**
	 * Constructor
	 *
	 * \param  alloc   allocator for registry elements
	 */
	Login_registry(Genode::Allocator &alloc) : _alloc(alloc) { }

	/**
	 * Return registry mutex 
	 */
	Util::Pthread_mutex &mutex() { return _mutex; }

	/**
	 * Import all login information from config
	 */
	void import(Genode::Xml_node const &node)
	{
		_remove_all();

		try {
			node.for_each_sub_node("login",
			[&] (Genode::Xml_node const &login) {
				_import_single(login, node);
			});
		} catch (...) { }
	}

	/**
	 * Look up login information by user name
	 */
	Ssh::Login const *lookup(char const *user) const
	{
		Login const *p = nullptr;
		auto lookup_user = [&] (Login const &login) {
			if (login.user == user) { p = &login; }
		};
		for_each(lookup_user);
		return p;
	}
};

#endif /* _SSH_TERMINAL_LOGIN_H_ */
