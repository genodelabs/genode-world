/*
 * \brief  ROM Lzlip decompressor
 * \author Emery Hemingway
 * \date   2017-04-10
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/*
 * TODO: Multithreaded decompression.
 */

/* Genode includes */
#include <os/session_policy.h>
#include <rom_session/connection.h>
#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <base/heap.h>
#include <base/service.h>
#include <base/session_label.h>
#include <libc/component.h>
#include <base/log.h>

namespace {
using namespace Genode;

/* Lzlib includes */
#include <lzlib.h>
}

namespace Lz_rom {
	using namespace Genode;

	typedef Session_state::Args Args;
	typedef String<Session_label::capacity()> Lz_path;

	struct Session;
	struct Main;

	struct File_error { };
	struct Decompression_error { };

}


struct Lz_rom::Session :
	Genode::Rpc_object<Genode::Rom_session>,
	Genode::Parent::Server
{
	Parent::Client parent_client;

	Id_space<Parent::Server>::Element server_id;

	Session(Id_space<Parent::Server> &server_space,
	        Parent::Server::Id server_id,
	        Libc::Env &env, Genode::Allocator &alloc,
	        Lz_path const &path,
	        LZ_Decoder *decoder);

	Attached_ram_dataspace ram_ds;

	/***************************
	 ** ROM session interface **
	 ***************************/

	Rom_dataspace_capability dataspace() override
	{
		Genode::Dataspace_capability ds_cap = ram_ds.cap();
		return static_cap_cast<Rom_dataspace>(ds_cap);
	}

	void sigh(Signal_context_capability sigh) override { }
};


Lz_rom::Session::Session(Id_space<Parent::Server> &server_space,
                         Parent::Server::Id server_id,
                         Libc::Env &env, Genode::Allocator &alloc,
                         Lz_path const &path,
                         LZ_Decoder *decoder)
:
	server_id(*this, server_space, server_id),
	ram_ds(env.ram(), env.rm(), 0)
{
	using namespace Vfs;
	typedef Vfs::Directory_service::Stat_result Stat_result;
	typedef Vfs::Directory_service::Open_result Open_result;
	typedef Vfs::File_io_service::Read_result Read_result;

	/* Get file size */
	Vfs::Directory_service::Stat stat;
	if (env.vfs().stat(path.string(), stat) != Stat_result::STAT_OK)
		throw File_error();
	if (!stat.size)
		throw File_error();

	/* Open file */
	Vfs_handle *fh;
	Open_result res = env.vfs().open(
		path.string(), Vfs::Directory_service::OPEN_MODE_RDONLY, &fh, alloc);
	if (res != Open_result::OPEN_OK)
		throw File_error();
	Vfs_handle::Guard handle_guard(fh);

	size_t compressed_size = stat.size;
	size_t uncompressed_size;

	/* read the uncompressed file size from the end of the file */
	fh->seek(stat.size - 16);
	{
		uint64_t data_size = 0;

		file_size n = 0;
		Read_result res = fh->fs().read(fh, (char*)&data_size, sizeof(data_size), n);
		if (res != Read_result::READ_OK || n != sizeof(data_size))
			throw File_error();

		/* XXX: little-endian only */
		uncompressed_size = data_size;
	}
	if (uncompressed_size == 0)
		throw File_error();

	/*
	 * TODO: check the compressed member size
	 * to see if this is a concatenated file
	 */

	LZ_decompress_reset(decoder);

	/* Allocate the ROM buffer now that the size is known */
	ram_ds.realloc(&env.ram(), uncompressed_size);
	uint8_t *rom_buf = ram_ds.local_addr<uint8_t>();

	/* Page aligned size of ROM dataspace */
	size_t const rom_size = ram_ds.size();
	/* Offset of encoded data*/
	size_t enc_off = rom_size - compressed_size;
	/* Offset of decoded data */
	size_t dec_off = 0;

	/* Read the compressed data into the back of the ROM dataspace */
	{
		file_size read_off = enc_off;
		file_size read_len = compressed_size;
		fh->seek(0);
		while (read_off < rom_size) {
			file_size n = 0;
			Read_result res = fh->fs().read(
				fh, (char*)(rom_buf+read_off), read_len, n);
			if (res != Read_result::READ_OK)
				throw File_error();
			fh->advance_seek(n);
			read_len -= n;
			read_off += n;
		}
	}

	/* Decode from the back of the dataspace to the front */
	while (dec_off < uncompressed_size) {
		if (enc_off < rom_size) {
			int write_size = min(LZ_decompress_write_size(decoder),
			                     int(rom_size - enc_off));

			/* write to the decoder */
			write_size = LZ_decompress_write(decoder, rom_buf+enc_off, write_size);
			if (write_size < 0)
				throw Decompression_error();
			enc_off += write_size;
		}

		/* read from the decoder */
		int read_size = LZ_decompress_read(
			decoder, rom_buf+dec_off, uncompressed_size-dec_off);
		if (read_size < 0)
			throw Decompression_error();

		dec_off += read_size;
	}

	/* Sweep the crumbs out of the page boundry gap */
	memset(rom_buf+uncompressed_size, 0x00, rom_size - uncompressed_size);

	LZ_decompress_finish(decoder);
}


struct Lz_rom::Main
{
	Id_space<Parent::Server> server_id_space;

	Libc::Env &env;

	Attached_rom_dataspace config_rom { env, "config" };

	Attached_rom_dataspace session_requests { env, "session_requests" };

	Sliced_heap session_alloc { env.ram(), env.rm() };
	Heap        vfs_alloc { env.ram(), env.rm() };

	bool config_stale = false;

	void handle_config() {
		config_stale = true; }

	void handle_session_request(Xml_node request);

	void handle_session_requests()
	{
		if (config_stale) {
			config_rom.update();
			config_stale = false;
		}

		session_requests.update();

		Xml_node const requests = session_requests.xml();

		requests.for_each_sub_node([&] (Xml_node request) {
			handle_session_request(request);
		});
	}

	Signal_handler<Main> config_handler {
		env.ep(), *this, &Main::handle_config };

	Signal_handler<Main> session_request_handler {
		env.ep(), *this, &Main::handle_session_requests };

	LZ_Decoder *decoder = LZ_decompress_open();

	Main(Libc::Env &env) : env(env)
	{
		config_rom.sigh(config_handler);
		session_requests.sigh(session_request_handler);

		/* handle requests that have queued before or during construction */
		handle_session_requests();
	}

	~Main()
	{
		LZ_decompress_close(decoder);
	}

};


void Lz_rom::Main::handle_session_request(Xml_node request)
{
	if (!request.has_attribute("id"))
		return;

	Id_space<Parent::Server>::Id const server_id {
		request.attribute_value("id", 0UL) };

	if (request.has_type("create")) {
		if (!request.has_sub_node("args"))
			return;

		typedef Session_state::Args Args;
		Args const args = request.sub_node("args").decoded_content<Args>();
		Session_label const request_label =
			label_from_args(args.string()).last_element();
		Lz_path const lz_path("/", request_label.string(), ".lz");

		try {
			Session *session = new (session_alloc)
				Session(server_id_space, server_id, env, vfs_alloc, lz_path, decoder);
			env.parent().deliver_session_cap(
				server_id, env.ep().manage(*session));
			return;
		} catch (File_error) {
			log("failed to open or read file '", lz_path, "'");
		} catch (Decompression_error) {
			char const *msg = "";

			switch (LZ_decompress_errno(decoder)) {
			case LZ_ok:
				error("no error"); break;

			case LZ_bad_argument:
				msg = "at least one of the arguments passed to the library function was invalid"; break;

			case LZ_mem_error:
				msg = "no memory available"; break;

			case LZ_sequence_error:
				msg = "a library function was called in the wrong order"; break;

			case LZ_header_error:
				msg = "an invalid member header was read"; break;

			case LZ_unexpected_eof:
				msg = "the end of the data stream was reached in the middle of a member"; break;

			case LZ_data_error:
				msg = "the data stream is corrupt"; break;

			case LZ_library_error:
				msg = "a bug was detected in the library"; break;
			}
			error("failed to decompress '", lz_path, "', ", msg);
		} catch (...) { }
		env.parent().session_response(server_id, Parent::INVALID_ARGS);
	}

	if (request.has_type("close")) {
		server_id_space.apply<Session>(server_id, [&] (Session &session) {
			destroy(session_alloc, &session);
			env.parent().session_response(server_id, Parent::SESSION_CLOSED);
		});
	}
}


/***************
 ** Component **
 ***************/

void Libc::Component::construct(Libc::Env &env)
{
	Libc::with_libc([&] () {
		static Lz_rom::Main inst(env);
		env.parent().announce("ROM");
	});
}
