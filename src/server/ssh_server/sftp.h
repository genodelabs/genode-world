/*
 * \brief  Sftp support for ssh_server component
 * \author Tomasz Gajewski
 * \date   2021-07-24
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 * Copyright (C) 2021 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SSH_TERMINAL_SFTP_H_
#define _SSH_TERMINAL_SFTP_H_

/* Genode includes */
#include <base/capability.h>
#include <base/log.h>
#include <base/mutex.h>
#include <base/signal.h>
#include <base/thread.h>
#include <os/ring_buffer.h>
#include <session/session.h>

#define WITH_SERVER
#include <libssh/sftp.h>

#include <assert.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

/* local includes */
#include "login.h"
#include "wake_up_signaller.h"


namespace Ssh
{
	using namespace Genode;

	class Sftp;
}


#define SFTP_WORKER_STACK_SIZE 16*1024

class Ssh::Sftp
{
	private:
		Genode::Heap       &_heap;
	  Wake_up_signaller  &_wake_up_signaller;
		Ssh::User           _user               { };
		sftp_session        _sftp_server        { nullptr };
		pthread_t           _worker_thread      { 0 };

		/* currently assembled packet state */
		enum Packet_state { INITIAL,
		                    PAYLOAD_INITED,
		                    SIZE_READ,
		                    TYPE_READ,
		                    PAYLOAD_ALLOCATED,
		                    PACKET_ASSEMBLED } _packet_state = INITIAL;

		static constexpr int SIZE_BUFFER_SIZE = 4;
		uint8_t  size_buffer[SIZE_BUFFER_SIZE];
		uint32_t size_filled;

		uint32_t packet_size_left;

		static constexpr int CLIENT_REQUESTS_MAX = 128;
		Ring_buffer<sftp_client_message, CLIENT_REQUESTS_MAX> _client_requests;

		static constexpr int PENDING_PACKETS_MAX = 128;
		Ring_buffer<ssh_buffer, PENDING_PACKETS_MAX> _pending_packets;

		ssh_buffer _output_payload;
		uint32_t   _output_pos;

		static constexpr const char* valid_path_prefix = "/sftp";
		static constexpr int valid_path_length = 5;

	public:

		enum State { UNINITIALIZED,
		             CREATED,
		             INITIALIZED,
		             WORKER_CLOSING,
		             WORKER_FINISHED,
		             CREATE_ERROR,
		             CLEAN } _state = UNINITIALIZED;

		Sftp(Genode::Heap &heap, Wake_up_signaller &wake_up_signaller)
			: _heap(heap), _wake_up_signaller(wake_up_signaller),
			  _output_payload(nullptr), _output_pos(0) {}
		~Sftp();

		void cleanup();

		void set_state(State state);

		bool uninitialized() const { return _state == UNINITIALIZED; }

		void initialize_subsystem(ssh_session session, ssh_channel channel,
		                          Ssh::User const &user);

		Ssh::User const &user() const { return _user; }


		int incoming_sftp_data(void *data, uint32_t len);
		void handle_eof();

		int assemble_sftp_packet(void *data, uint32_t len);
		sftp_packet consume_sftp_packet();
		void enqueue_sftp_client_message(sftp_client_message msg);

		int enqueue_sftp_packet(ssh_buffer payload);
		void send_queued_packets(ssh_channel channel);


		struct Handle	: Genode::Registry<Handle>::Element
		{
			enum Type { HDIR, HFILE } _type;
			char*                     _name = nullptr;
			DIR*                      _dir  = nullptr;
			FILE*                     _file = nullptr;
			bool                      _eof  = false;
			bool                      _root = false;

			Handle(DIR* dir, const char* name, bool root,
			       Genode::Registry<Handle> &reg)
				: Element(reg, *this), _type(HDIR), _dir(dir), _root(root)
			{
				_name = strdup(name);
			}

			Handle(FILE* file, const char* name, Genode::Registry<Handle> &reg)
				: Element(reg, *this), _type(HFILE), _file(file)
			{
				_name = strdup(name);
			}

			~Handle() {
				if (_name != nullptr) ::free(_name);
				close_dir();
				close_file();
			}

			int close_dir();
			int close_file();
		};

		using Handle_registry = Genode::Registry<Handle>;
		Handle_registry _handles;

		int reply_errno_status(sftp_client_message msg);

		enum Stat_mode { STAT, LSTAT };
		int get_fs_entry_info(const char* path, Stat_mode mode,
		                      sftp_attributes attrs,
		                      const char* name = nullptr,
		                      char* longname = nullptr);

		void process_message(sftp_client_message msg);

		enum Realpath_mode { VALIDATE, VALIDATE_DIR, PROCESS };
		bool process_realpath(sftp_client_message msg, Realpath_mode mode,
		                      bool* is_root = nullptr);
		void process_opendir(sftp_client_message msg);
		void process_open(sftp_client_message msg);
		void process_readdir(sftp_client_message msg);
		void process_read(sftp_client_message msg);
		void process_write(sftp_client_message msg);
		void process_close(sftp_client_message msg);
		void process_stat(sftp_client_message msg, Stat_mode mode);
		void process_remove(sftp_client_message msg);
		void process_mkdir(sftp_client_message msg);
		void process_rmdir(sftp_client_message msg);

		static void * sftp_worker_loop(void *arg);
};

namespace Genode {
	void print(Output&, Ssh::Sftp::State);
}

#endif  /* _SSH_TERMINAL_SFTP_H_ */
