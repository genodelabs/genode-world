/*
 * \brief  Google Time-based One-time Password Algorithm filesystem
 * \author Emery Hemingway
 * \date   2016-09-07
 *
 * https://en.wikipedia.org/wiki/Google_Authenticator
 */

/*
 * Copyright (C) 2016-2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */


/* Genode includes */
#include <libc/component.h>
#include <os/reporter.h>
#include <rtc_session/connection.h>
#include <timer_session/connection.h>
#include <base/log.h>

/* Crypto++ includes */
#include <hmac.h>
#include <sha.h>


struct Main
{
	Genode::Env &env;

	Timer::Connection timer    { env };
	Genode::Reporter  reporter { env, "otp" };

	CryptoPP::HMAC<CryptoPP::SHA1> hmac;

	Timer::Periodic_timeout<Main> update_timeout {
		timer, *this, &Main::update, Genode::Microseconds(30*1000*1000) };

	static Genode::uint64_t rtc_nonce(Genode::Env &env)
	{
		using namespace Genode;
		typedef Genode::uint64_t uint64_t;

		Rtc::Timestamp const ts = Rtc::Connection(env).current_time();;

		enum {
			PERIOD = 30 /* seconds */,
			PERIODS_PER_MINUTE = 60 / PERIOD,
			PERIODS_PER_HOUR   = PERIODS_PER_MINUTE*60,
			PERIODS_PER_DAY    = PERIODS_PER_HOUR*24
		};

		unsigned const m = (ts.month + 9) % 12;
		unsigned const y = ts.year - m/10;
		unsigned const days =
			365*(y) + y/4 - y/100 + y/400 +
			(m*306 + 5)/10 + (ts.day - 1) - 719468;

		return
			(uint64_t)days*PERIODS_PER_DAY +
			(uint64_t)ts.hour*PERIODS_PER_HOUR +
			(uint64_t)ts.minute*PERIODS_PER_MINUTE +
			(uint64_t)ts.second/PERIOD;
	}

	uint64_t const initial_nonce = rtc_nonce(env);

	void update(Genode::Duration dur)
	{
		uint64_t const nonce =
			initial_nonce + dur.trunc_to_plain_ms().value / (30 * 1000);

		uint8_t nonce_digest[hmac.DigestSize()];
		uint8_t nonce_buf[8];

		nonce_buf[0] = nonce >> (7 * 8) & 0xff;
		nonce_buf[1] = nonce >> (6 * 8) & 0xff;
		nonce_buf[2] = nonce >> (5 * 8) & 0xff;
		nonce_buf[3] = nonce >> (4 * 8) & 0xff;
		nonce_buf[4] = nonce >> (3 * 8) & 0xff;
		nonce_buf[5] = nonce >> (2 * 8) & 0xff;
		nonce_buf[6] = nonce >> (1 * 8) & 0xff;
		nonce_buf[7] = nonce >> (0 * 8) & 0xff;

		hmac.Update(nonce_buf, sizeof(nonce_buf));
		hmac.Final(nonce_digest);
		hmac.Restart();

		unsigned const offset =
			nonce_digest[sizeof(nonce_digest)-1] & 0x0F;

		uint32_t code;
		code  = (nonce_digest[offset+0]&0x7F) << (3 * 8);
		code +=  nonce_digest[offset+1] << (2 * 8);
		code +=  nonce_digest[offset+2] << (1 * 8);
		code +=  nonce_digest[offset+3] << (0 * 8);

		code = code % 1000000;

		char buf[7] { 0 };
		buf[0] = 0x30 | (code/100000);
		buf[1] = 0x30 | ((code/10000) % 10);
		buf[2] = 0x30 | ((code/1000) % 10);
		buf[3] = 0x30 | ((code/100) % 10);
		buf[4] = 0x30 | ((code/10) % 10);
		buf[5] = 0x30 | (code%10);

		{
			using namespace Genode;

			Reporter::Xml_generator gen(reporter, [&] () {
				gen.attribute("value", (char const *)buf);
			});
		}
	}

	Main(Genode::Env &env, uint8_t *secret, size_t secret_len)
	: env(env), hmac(secret, secret_len)
	{
		reporter.enabled(true);
	}
};


void Libc::Component::construct(Libc::Env &env)
{
	using namespace Genode;

	enum {
		BASE32_FACTOR   = 256 / 32,
		MAX_SECRET_BIN_LEN = 20,
		MAX_SECRET_STR_LEN = MAX_SECRET_BIN_LEN * BASE32_FACTOR
	};

	size_t i = 0;
	uint8_t binsec[MAX_SECRET_BIN_LEN];

	{
		Genode::uint8_t base32_table[256];

		char const *upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
		char const *lower = "abcdefghijklmnopqrstuvwxyz";

		for (int i = 0; i < 256; ++i)
			base32_table[i] = 0xFF;

		for (int i = 0; i < 32; ++i)
			base32_table[(uint8_t)upper[i]] = i;

		for (int i = 0; i < 26; ++i)
			base32_table[(uint8_t)lower[i]] = i;

		typedef String<MAX_SECRET_STR_LEN+1> Secret_string;

		Secret_string secret_string;

		env.config([&] (Xml_node const &node) {
			try {
				node.attribute("secret").value(&secret_string); }
			catch (Xml_node::Nonexistent_attribute) {
				error("'secret' attribute missing from gtotp VFS node");
				throw;
			}
		});

		if ((secret_string.length()-1) % 8) {
			error("gtotp secret has a strange length ", secret_string.length()-1);
			throw Exception();
		}

		char const *b32str = secret_string.string();

		uint8_t buf[8];
		size_t j = 0;
		while (j < secret_string.length()-1) {
			for (size_t k = 0; k < 8; ++k)
				buf[k] = base32_table[(uint8_t)b32str[j+k]];

			binsec[i+0] = buf[0]<<3 | buf[1]>>2;
			binsec[i+1] = buf[1]<<6 | buf[2]<<1 | buf[3] >>4;
			binsec[i+2] = buf[3]<<4 | buf[4]>>1;
			binsec[i+3] = buf[4]<<7 | buf[5]<<2 | buf[6]>>3;
			binsec[i+4] = buf[6]<<5 | buf[7];

			i += 5;
			j += 8;
		}
	}

	static Main inst(env, binsec, i);
};
