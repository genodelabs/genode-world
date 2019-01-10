/*
 * A simple DHT boostrap node for tox.
 */

/*
 * Copyright © 2018 Genode Labs GmbH
 *
 * This file is part of Tox, the free peer to peer instant messenger.
 *
 * Tox is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Tox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tox.  If not, see <http://www.gnu.org/licenses/>.
 */

extern "C" {
#include "toxcore/tox.h"
#include "toxcore/DHT.h"
#include "toxcore/LAN_discovery.h"
#include "toxcore/friend_requests.h"
#include "toxcore/logger.h"
#include "toxcore/mono_time.h"
#include "toxcore/tox.h"
#include "toxcore/util.h"
}
#include "toxcore/genode_logger.h"

/* Genode includes */
#include <os/reporter.h>
#include <timer_session/connection.h>
#include <libc/component.h>


namespace Tox_dht_bootstrap {

struct Main;

enum {
	IPV6_ENABLE = false,
	PORT  = 33445
};


typedef Genode::String<65> Key_string;


static bool non_zero(Client_data const &peer)
{
	for (int i = 0; i < CRYPTO_PUBLIC_KEY_SIZE; ++i)
		if (peer.public_key[i]) return true;
	return false;
}


static Key_string encode_key(uint8_t const *bin)
{
	static const char alphabet[] = {
		'0',  '1',  '2',  '3',  '4',  '5',  '6',  '7', 
		'8',  '9',  'A',  'B',  'C',  'D',  'E',  'F',
	};

	char buf[CRYPTO_PUBLIC_KEY_SIZE*2];
	for (int i = 0; i < CRYPTO_PUBLIC_KEY_SIZE; ++i) {
		buf[(i<<1)] = alphabet[bin[i]>>4];
		buf[(i<<1)|1] = alphabet[bin[i]&0xf];
	}

	return Key_string(Genode::Cstring(buf, CRYPTO_PUBLIC_KEY_SIZE*2));
}

static uint8_t *hex_string_to_bin(const char *hex_string)
{
    if (strlen(hex_string) % 2 != 0) {
        return nullptr;
    }

    size_t len = strlen(hex_string) / 2;
    uint8_t *ret = (uint8_t *)malloc(len);

    const char *pos = hex_string;
    size_t i;

    for (i = 0; i < len; ++i, pos += 2) {
        unsigned int val;
        sscanf(pos, "%02x", &val);
        ret[i] = val;
    }

    return ret;
}


void bootstrap_from_config(DHT *dht, Genode::Xml_node const &config)
{
	typedef Genode::String<TOX_ADDRESS_SIZE*2> Key;
	typedef Genode::String<TOX_MAX_HOSTNAME_LENGTH> Host;

	auto const bs_fn = [&] (Genode::Xml_node const &node) {
		Key key;
		Host host;
		uint16_t port = PORT;

		try {
			node.attribute("ip").value(&host);
			node.attribute("key").value(&key);
			port = node.attribute_value("port", port);
		}
		catch (...) {
			Genode::error("Invalid bootstrap node ", node);
			return;
		}

		uint8_t *bootstrap_key = hex_string_to_bin(key.string());
		int res = dht_bootstrap_from_address(
			dht, host.string(), IPV6_ENABLE, net_htons(port), bootstrap_key);
		free(bootstrap_key);
		if (!res)
			Genode::error("Failed to connect to ", node);
	};

	config.for_each_sub_node("bootstrap", bs_fn);
}

}


struct Tox_dht_bootstrap::Main
{
	Main(const Main&);
	Main &operator=(const Main&);

	Libc::Env &_env;

	Timer::Connection _timer { _env, "periodic-timeout" };

	Genode::Expanding_reporter _reporter { _env, "dht_state", "dht_state" };

	Logger *logger = Toxcore::new_genode_logger();

	Mono_Time *_mono_time = mono_time_new();

	uint64_t last_LANdiscovery = 0;

	int is_waiting_for_dht_connection = 1;

	DHT *init_dht()
	{
		/* Initialize networking -
		   Bind to ip 0.0.0.0 / [::] : PORT */
		IP ip;
		ip_init(&ip, IPV6_ENABLE);

		DHT *dht = new_dht(logger, _mono_time, new_networking(logger, ip, PORT), true);
		Onion *onion = new_onion(_mono_time, dht);
		Onion_Announce *onion_a = new_onion_announce(_mono_time, dht);

		if (!(onion && onion_a)) {
			Genode::error("Something failed to initialize");
			exit(1);
		}

		Genode::log("Listening on UDP port ", net_ntohs(net_port(dht_get_net(dht))));

		_env.config([&] (Genode::Xml_node const &config) {
			bootstrap_from_config(dht, config); });

		lan_discovery_init(dht);

		_env.config([&] (Genode::Xml_node const &config) {
			config.for_each_sub_node("report", [&] (Genode::Xml_node const &node) {
				if (node.attribute_value("dht", false)) {
					_report_timeout.construct(_timer, *this, &Main::report, Genode::Microseconds(4*1000*1000));
				}
			});
		});

		return dht;
	}

	DHT *_dht = init_dht();

	Timer::Periodic_timeout<Main> _periodic_timeout {
		_timer, *this, &Main::run, Genode::Microseconds(1000*1000) };

	Genode::Constructible<Timer::Periodic_timeout<Main>> _report_timeout { };

	Main(Libc::Env &env) : _env(env) { }


	void report(Genode::Duration);
	void run(Genode::Duration);

	template<typename FUNC>
	void for_each_close_peer(FUNC const &func)
	{
		for (uint32_t i = 0; i < LCLIENT_LIST; i +=  LCLIENT_NODES) {
			for (uint32_t j = 0; j < LCLIENT_NODES; ++j) {
				Client_data const *peer = dht_get_close_client(_dht, i+j);
				if (!non_zero(*peer)) break;
				func((Client_data const &)*peer);
			}
		}
	}
};


void Tox_dht_bootstrap::Main::report(Genode::Duration)
{
	mono_time_update(_mono_time);

	Libc::with_libc([&] () {
		_reporter.generate([&] (Genode::Xml_generator &gen) {
			uint64_t now = mono_time_get(_mono_time);
			gen.attribute("timestamp", now);
			gen.node("base", [&] () {
				gen.attribute("public", encode_key(dht_get_self_public_key(_dht)));
				//gen.attribute("secret", encode_key(dht_get_self_secret_key(_dht)));
			});

			for_each_close_peer([&] (Client_data const &peer) {
				gen.node("close", [&] () {
					gen.attribute("public", encode_key(peer.public_key));

					char ip_buf[IP_NTOA_LEN];
					ip_ntoa(&peer.assoc4.ip_port.ip, ip_buf, sizeof(ip_buf));
					gen.attribute("ip", (char const *)ip_buf);
					gen.attribute("port", peer.assoc4.ip_port.port);
					gen.attribute("timestamp", now - peer.assoc4.timestamp);
					gen.attribute("last_pinged", now - peer.assoc4.last_pinged);
				});
			});
		});
	});
}


void Tox_dht_bootstrap::Main::run(Genode::Duration)
{
	Libc::with_libc([&] () {
		mono_time_update(_mono_time);

		if (is_waiting_for_dht_connection && dht_isconnected(_dht)) {
			Genode::log("Connected to other bootstrap node successfully.\n");
			is_waiting_for_dht_connection = 0;
		}

		do_dht(_dht);

		if (mono_time_is_timeout(_mono_time, last_LANdiscovery, is_waiting_for_dht_connection ? 5 : LAN_DISCOVERY_INTERVAL)) {
			lan_discovery_send(net_htons(PORT), _dht);
			last_LANdiscovery = mono_time_get(_mono_time);
		}

		networking_poll(dht_get_net(_dht), nullptr);
	});
}


void Libc::Component::construct(Libc::Env &env)
{
	Libc::with_libc([&] () {
		static Tox_dht_bootstrap::Main inst(env);
	});
}
