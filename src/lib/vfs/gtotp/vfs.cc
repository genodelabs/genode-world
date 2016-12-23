/*
 * \brief  Google Time-based One-time Password Algorithm filesystem
 * \author Emery Hemingway
 * \date   2016-09-07
 *
 * https://en.wikipedia.org/wiki/Google_Authenticator
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */


/* Genode includes */
#include <vfs/file_system_factory.h>
#include <vfs/single_file_system.h>
#include <vfs/vfs_handle.h>
#include <rtc_session/connection.h>
#include <base/log.h>

/* Crypto++ includes */
#include <hmac.h>
#include <sha.h>


namespace Vfs { class Gtotp_file_system; };

class Vfs::Gtotp_file_system : public Single_file_system
{
	private:

		CryptoPP::HMAC<CryptoPP::SHA1> _hmac;

		Rtc::Connection &_rtc;

	public:

		Gtotp_file_system(Genode::Xml_node &node,
		                  Rtc::Connection  &rtc,
		                  Genode::uint8_t  *secret,
		                  Genode::size_t    secret_len)
		:
			Vfs::Single_file_system(NODE_TYPE_CHAR_DEVICE, "gtotp", node),
			_hmac(secret, secret_len), _rtc(rtc) { }

		Stat_result stat(char const *path, Stat &out) override
		{
			Stat_result result = Single_file_system::stat(path, out);
			out.size = 7; /* six decimal digits and newline */

			return result;
		}

		Read_result read(Vfs_handle *vfs_handle,
		                 char *buf, file_size buf_size,
		                 file_size &out) override
		{
			if ((vfs_handle->seek() != 0) || (buf_size < 7)) {
				out = 0;
				return READ_OK;
			}

			using namespace Genode;

			enum {
				PERIOD = 30 /* seconds */, 
				PERIODS_PER_MINUTE = 60 / PERIOD,
				PERIODS_PER_HOUR   = PERIODS_PER_MINUTE*60,
				PERIODS_PER_DAY    = PERIODS_PER_HOUR*24
			};

			Rtc::Timestamp const ts = _rtc.current_time();

			unsigned const m = (ts.month + 9) % 12;
			unsigned const y = ts.year - m/10;
			unsigned const days =
				365*(y) + y/4 - y/100 + y/400 +
				(m*306 + 5)/10 + (ts.day - 1) - 719468;

			Genode::uint64_t const nonce =
				days*PERIODS_PER_DAY +
				ts.hour*PERIODS_PER_HOUR +
				ts.minute*PERIODS_PER_MINUTE +
				ts.second/PERIOD;

			uint8_t nonce_digest[_hmac.DigestSize()];
			uint8_t nonce_buf[8];

			nonce_buf[0] = nonce >> (7 * 8) & 0xff;
			nonce_buf[1] = nonce >> (6 * 8) & 0xff;
			nonce_buf[2] = nonce >> (5 * 8) & 0xff;
			nonce_buf[3] = nonce >> (4 * 8) & 0xff;
			nonce_buf[4] = nonce >> (3 * 8) & 0xff;
			nonce_buf[5] = nonce >> (2 * 8) & 0xff;
			nonce_buf[6] = nonce >> (1 * 8) & 0xff;
			nonce_buf[7] = nonce >> (0 * 8) & 0xff;

			_hmac.Update(nonce_buf, sizeof(nonce_buf));
			_hmac.Final(nonce_digest);
			_hmac.Restart();

			unsigned const offset =
				nonce_digest[sizeof(nonce_digest)-1] & 0x0F;

			uint32_t code;
			code  = (nonce_digest[offset+0]&0x7F) << (3 * 8);
			code +=  nonce_digest[offset+1] << (2 * 8);
			code +=  nonce_digest[offset+2] << (1 * 8);
			code +=  nonce_digest[offset+3] << (0 * 8);

			code = code % 1000000;

			buf[0] = 0x30 | (code/100000);
			buf[1] = 0x30 | ((code/10000) % 10);
			buf[2] = 0x30 | ((code/1000) % 10);
			buf[3] = 0x30 | ((code/100) % 10);
			buf[4] = 0x30 | ((code/10) % 10);
			buf[5] = 0x30 | (code%10);
			buf[6] = '\n';
			out = 7;

			return Read_result::READ_OK;
		}

		Write_result write(Vfs_handle *vfs_handle,
		                   char const *buf, file_size buf_size,
		                   file_size &out_count) override {
			return WRITE_ERR_INVALID; }
};


struct Gtotp_file_system_factory : Vfs::File_system_factory
{
	Genode::uint8_t base32_table[256];

	Gtotp_file_system_factory()
	{
		char const *upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
		char const *lower = "abcdefghijklmnopqrstuvwxyz";

		for (int i = 0; i < 256; ++i)
			base32_table[i] = 0xFF;

		for (int i = 0; i < 32; ++i)
			base32_table[(uint8_t)upper[i]] = i;

		for (int i = 0; i < 26; ++i)
			base32_table[(uint8_t)lower[i]] = i;
	}

	Vfs::File_system *create(Genode::Env &env,
		                     Genode::Allocator &alloc,
		                     Genode::Xml_node node) override
	{
		static Rtc::Connection rtc(env);

		using namespace Genode;
		typedef Genode::size_t size_t;

		enum {
			BASE32_FACTOR   = 256 / 32,
			MAX_SECRET_BIN_LEN = 20,
			MAX_SECRET_STR_LEN = MAX_SECRET_BIN_LEN * BASE32_FACTOR
		};

		typedef String<MAX_SECRET_STR_LEN+1> Secret_string;

		Secret_string secret_string;
		uint8_t binsec[MAX_SECRET_BIN_LEN];

		try { node.attribute("secret").value(&secret_string); }
		catch (Xml_node::Nonexistent_attribute) {
			error("'secret' attribute missing from gtotp VFS node");
			throw;
		}

		if ((secret_string.length()-1) % 8) {
			error("gtotp secret has a strange length ", secret_string.length()-1);
			throw Exception();
		}

		char const *b32str = secret_string.string();

		uint8_t buf[8];
		size_t i = 0;
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

		return new (Genode::env()->heap())
			Vfs::Gtotp_file_system(node, rtc, binsec, i);
	}
};


extern "C" Vfs::File_system_factory *vfs_file_system_factory(void)
{
	static Gtotp_file_system_factory factory;
	return &factory;
}
