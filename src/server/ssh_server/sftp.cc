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

#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

extern "C" {
#include <libssh/buffer.h>
}
#include <libssh/server.h>

/* local includes */
#include "sftp.h"

template <class T>
class Free_guard {
	T& _ptr;
public:
	Free_guard(T& ptr) : _ptr(ptr) {}
	~Free_guard() { if (_ptr != nullptr) ::free(_ptr); }
};


void Genode::print(Output &out, Ssh::Sftp::State state)
{
  switch (state) {
	case Ssh::Sftp::UNINITIALIZED:   out.out_string("UNINITIALIZED");   break;
	case Ssh::Sftp::CREATED:         out.out_string("CREATED");         break;
	case Ssh::Sftp::INITIALIZED:     out.out_string("INITIALIZED");     break;
	case Ssh::Sftp::WORKER_CLOSING:  out.out_string("WORKER_CLOSING");  break;
	case Ssh::Sftp::WORKER_FINISHED: out.out_string("WORKER_FINISHED"); break;
	case Ssh::Sftp::CREATE_ERROR:    out.out_string("CREATE_ERROR");    break;
	case Ssh::Sftp::CLEAN:           out.out_string("CLEAN");           break;
  };
}

void * Ssh::Sftp::sftp_worker_loop(void *arg)
{
	Ssh::Sftp &server = *reinterpret_cast<Ssh::Sftp*>(arg);

	bool signal_sent = false;
	while (true) {
		sftp_client_message msg = server._client_requests.get();

		/* made some place in _client_requests so signal about it */
		if (!signal_sent) {
			signal_sent = true;
			server._wake_up_signaller.signal_wake_up();
		}

		/* exit loop if empty message found */
		if (msg == NULL) break;

		server.process_message(msg);
	}

	ssh_channel_request_send_exit_status(server._sftp_server->channel, 0);

	server.set_state(WORKER_FINISHED);

	server._wake_up_signaller.signal_wake_up();

	return 0;
}


Ssh::Sftp::~Sftp()
{
	cleanup();
}

void Ssh::Sftp::cleanup()
{
	switch (_state) {
	case INITIALIZED:
	case CREATED:
		/* simulate end of communcation */
		handle_eof();
		[[fallthrough]];
	case WORKER_FINISHED:
	case WORKER_CLOSING:
		{
			auto const ret = pthread_join(_worker_thread, nullptr);
			if (0 != ret) {
				Genode::warning("pthread_joined returned with ", ret,
				                " (errno=", errno, ")");
			}
		}
		[[fallthrough]];
	case CREATE_ERROR:
		sftp_server_free(_sftp_server);
		[[fallthrough]];
	case UNINITIALIZED:
	case CLEAN:
		break;
	}

	auto destroy_handle = [&] (Handle &handle) {
		Genode::destroy(&_heap, &handle);
	};
	_handles.for_each(destroy_handle);

	while (!_client_requests.empty()) {
		sftp_client_message_free(_client_requests.get());
	}

	while (!_pending_packets.empty()) {
		ssh_buffer_free(_pending_packets.get());
	}

	if (_output_payload != nullptr) {
		ssh_buffer_free(_output_payload);
		_output_payload = nullptr;
	}

	set_state(CLEAN);
}

int sftp_packet_write_ovr(sftp_session sftp, uint8_t type,
                          ssh_buffer payload, void *userdata)
{
	Ssh::Sftp &server = *reinterpret_cast<Ssh::Sftp*>(userdata);
	return server.enqueue_sftp_packet(payload);
}

sftp_packet sftp_packet_read_ovr(sftp_session sftp, void *userdata)
{
	Ssh::Sftp &server = *reinterpret_cast<Ssh::Sftp*>(userdata);
	sftp_packet packet = server.consume_sftp_packet();
	return packet;
}

void Ssh::Sftp::set_state(State state)
{
	/* warn on unexpected state changes */
	bool bad_target_state = false;
	bool bad_transition = false;
	switch (state) {
	case UNINITIALIZED:   bad_target_state = true;                      break;
	case CREATED:         bad_transition = (_state != UNINITIALIZED);   break;
	case INITIALIZED:     bad_transition = (_state != CREATED);         break;
	case WORKER_CLOSING:  bad_transition = (_state != INITIALIZED);     break;
	case WORKER_FINISHED: bad_transition = (_state != WORKER_CLOSING);  break;
	case CREATE_ERROR:    bad_transition = (_state != UNINITIALIZED);   break;
	case CLEAN:           bad_transition = false;                       break;
	};
	if (bad_target_state)
		Genode::warning("Sftp ", state, " state should never be set explicitely");
	if (bad_transition)
		Genode::warning("Sftp unexpected ", state, " state change from ", _state);

	_state = state;
}

void Ssh::Sftp::initialize_subsystem(ssh_session session, ssh_channel channel,
                                     Ssh::User const &user)
{
	_sftp_server = sftp_server_new(session, channel);
	if (_sftp_server == nullptr) {
		Genode::error("sftp: sftp_server_new failed");
		set_state(CREATE_ERROR);
		return;
	}

	_sftp_server->userdata = this;
	_sftp_server->sftp_packet_write_function = sftp_packet_write_ovr;
	_sftp_server->sftp_packet_read_function = sftp_packet_read_ovr;

	_user = user;

	int const ret = pthread_create(&_worker_thread, nullptr,
	                               Ssh::Sftp::sftp_worker_loop, (void*) this);
	if (ret == -1) {
		Genode::error("sftp: pthread_create failed");
		set_state(CREATE_ERROR);
		return;
	}

	set_state(CREATED);
}


int Ssh::Sftp::incoming_sftp_data(void *data, uint32_t len)
{
	uint8_t  *data_ptr = reinterpret_cast<uint8_t*>(data);
	uint32_t  avail    = len;

	int       rc;

	do {
		uint32_t processed = assemble_sftp_packet(data_ptr, avail);

		if (processed < 0) {
			return -1;
		}
		
		avail -= processed;
		data_ptr += processed;

		if (_packet_state == PACKET_ASSEMBLED) {

			if (_state == CREATED) {
				/* initialize sftp server */
				rc = sftp_server_init(_sftp_server);
				if (rc < 0) {
					Genode::error("sftp_server_init failed");
					/* consume packet if sftp_server_init did not do that already */
					if (_packet_state != INITIAL) sftp_packet_read(_sftp_server);
					/* satisfy handle_eof() */
					set_state(INITIALIZED);
					/* simulate end of communcation */
					handle_eof();
				} else {
					/* init success */
					set_state(INITIALIZED);
				}
			} else if (_state == CREATE_ERROR) {
				Genode::error("incoming_sftp_data: ",
				              "ignoring packet in CREATE_ERROR state");
				sftp_packet_read(_sftp_server);
			} else {
				sftp_client_message msg = sftp_get_client_message(_sftp_server);
				enqueue_sftp_client_message(msg);
			}

			assert(_packet_state == INITIAL);
		}

	} while (avail > 0 && _packet_state == INITIAL);

	return len - avail;
}

void Ssh::Sftp::handle_eof()
{
	set_state(WORKER_CLOSING);
	enqueue_sftp_client_message(0);
}

/* CHECK */
/* Buffer size maximum is 256M */
#define SFTP_PACKET_SIZE_MAX 0x10000000

static uint32_t sftp_get_u32(const void *vp)
{
    const uint8_t *p = (const uint8_t *)vp;
    uint32_t v;

    v  = (uint32_t)p[0] << 24;
    v |= (uint32_t)p[1] << 16;
    v |= (uint32_t)p[2] << 8;
    v |= (uint32_t)p[3];

    return v;
}

int Ssh::Sftp::assemble_sftp_packet(void *data, uint32_t len)
{
	uint8_t  *data_ptr = reinterpret_cast<uint8_t*>(data);
	uint32_t  avail    = len;
	uint32_t  data_len;

	int       rc;

	if (_sftp_server == nullptr) {
		Genode::error("assemble_sftp_packet: ",
		              "ignoring data if sftp server is null");
		return len;
	}

	sftp_packet packet = _sftp_server->read_packet;

	switch (_packet_state) {
	case INITIAL:
		/*
		 * If the packet has a payload, then just reinit the buffer, otherwise
		 * allocate a new one.
		 */
		if (packet->payload != NULL) {
			rc = ssh_buffer_reinit(packet->payload);
			if (rc != 0) {
				Genode::log("Ssh::Sftp::assemble_sftp_packet: buffer reinit failed");
				return -1;
			}
		} else {
			packet->payload = ssh_buffer_new();
			if (packet->payload == NULL) {
				Genode::log("Ssh::Sftp::assemble_sftp_packet: buffer new failed");
				return -1;
			}
		}

		size_filled = 0;

		_packet_state = PAYLOAD_INITED;
		[[fallthrough]];

	case PAYLOAD_INITED:
		/* read packet size */
		while (size_filled < SIZE_BUFFER_SIZE && avail > 0) {
			size_buffer[size_filled] = *data_ptr;
			++size_filled;
			++data_ptr;
			--avail;
		}
		if (size_filled == SIZE_BUFFER_SIZE) {
			packet_size_left = sftp_get_u32(size_buffer);
			if (packet_size_left == 0 || packet_size_left > SFTP_PACKET_SIZE_MAX) {
				Genode::log("Ssh::Sftp::assemble_sftp_packet: bad packet size: ",
				            packet_size_left);
				return -1;
			}
		} else {
			/* size buffer not filled yet */
			return len;
		}

		_packet_state = SIZE_READ;
		[[fallthrough]];

	case SIZE_READ:
		if (avail > 0) {
			packet->type = *data_ptr;
			++data_ptr;
			--avail;
		} else {
			return len;
		}

		/* Remove the packet type size */
		packet_size_left -= sizeof(uint8_t);

		_packet_state = TYPE_READ;
		[[fallthrough]];

	case TYPE_READ:
		rc = ssh_buffer_allocate_size(packet->payload, packet_size_left);
		if (rc < 0) {
			Genode::log("Ssh::Sftp::assemble_sftp_packet: buffer allocate failed: ",
			            packet_size_left);
			return -1;
		}

		_packet_state = PAYLOAD_ALLOCATED;
		[[fallthrough]];

	case PAYLOAD_ALLOCATED:
		data_len = (packet_size_left <= avail ? packet_size_left : avail);
		rc = ssh_buffer_add_data(packet->payload, data_ptr, data_len);
		if (rc != 0) {
			Genode::log("Ssh::Sftp::assemble_sftp_packet: buffer add data failed: ",
			            data_len);
			return -1;
		}

		packet_size_left -= data_len;
		data_ptr += data_len;
		avail -= data_len;

		if (packet_size_left != 0) {
			break;
		}

		_packet_state = PACKET_ASSEMBLED;

		[[fallthrough]];

	case PACKET_ASSEMBLED:
		break;
	}

	return len - avail;
}


sftp_packet Ssh::Sftp::consume_sftp_packet()
{
	assert(_packet_state == PACKET_ASSEMBLED);
	_packet_state = INITIAL;
	return _sftp_server->read_packet;
}


int Ssh::Sftp::enqueue_sftp_packet(ssh_buffer payload)
{
	ssh_buffer buf = ssh_buffer_new();
	if (buf == nullptr) {
		return SSH_ERROR;
	}

	/* cannot just take buffer as it is released by caller */
	ssh_buffer_swap(payload, buf);

	_pending_packets.add(buf);

	_wake_up_signaller.signal_wake_up();

	return SSH_OK;
}

void Ssh::Sftp::send_queued_packets(ssh_channel channel)
{
	/* ignore send request */
	if (!channel || !ssh_channel_is_open(channel)) { return; }

	bool signal_sent = false;
	while (_output_payload != nullptr || !_pending_packets.empty()) {
		ssh_buffer payload = nullptr;
		uint32_t pos = 0;
		if (_output_payload != nullptr) {
			payload = _output_payload;
			pos = _output_pos;
			_output_payload = nullptr;
		} else {
			payload = _pending_packets.get();

			/* made some place in _pending_packets so signal about it */
			if (!signal_sent) {
				signal_sent = true;
				_wake_up_signaller.signal_wake_up();
			}
		}
		void const *data   = ssh_buffer_get(payload);
		uint32_t const len = ssh_buffer_get_len(payload);

		int const num_bytes = ssh_channel_write(channel, (char*) data + pos,
		                                        len - pos);

		if (num_bytes < 0) {
			Genode::error("ERROR sending sftp data");
			ssh_buffer_free(payload);
			if (_state == CREATED || _state == INITIALIZED) {
				/* simulate end of communcation */
				handle_eof();
			}
			return;
		}

		if ((size_t)num_bytes < len - pos) {
			_output_payload = payload;
			_output_pos = pos + num_bytes;
			return;
		}

		ssh_buffer_free(payload);
	}
}

void Ssh::Sftp::enqueue_sftp_client_message(sftp_client_message msg)
{
	_client_requests.add(msg);
}


int Ssh::Sftp::Handle::close_dir()
{
	if (_dir == nullptr) return -1;

	int result = closedir(_dir);
	if (result != 0) {
		Genode::error("close_dir(): failed to close directory");
	}
	_dir = nullptr;

	return result;
}

int Ssh::Sftp::Handle::close_file()
{
	if (_file == nullptr) return -1;

	int result = fclose(_file);
	if (result != 0) {
		Genode::error("close_file(): failed to close file");
	}
	_file = nullptr;

	return result;
}


int Ssh::Sftp::reply_errno_status(sftp_client_message msg)
{
	const int MAX_STRERROR = 1024;
	char error_string[MAX_STRERROR];
	if (strerror_r(errno, error_string, MAX_STRERROR) != 0) {
		sprintf(error_string, "Unknown errno %d", errno);
	}

	uint32_t sftp_error_code = SSH_FX_FAILURE;
	switch (errno) {
	case EACCES:  sftp_error_code = SSH_FX_PERMISSION_DENIED; break;
	case EPERM:   sftp_error_code = SSH_FX_PERMISSION_DENIED; break;
	case ENOENT:  sftp_error_code = SSH_FX_NO_SUCH_PATH;      break;
	case ENOTDIR: sftp_error_code = SSH_FX_NO_SUCH_PATH;      break;
	}
	return sftp_reply_status(msg, sftp_error_code, error_string);
}

int Ssh::Sftp::get_fs_entry_info(const char* path, Stat_mode mode,
                                 sftp_attributes attrs,
                                 const char* name, char* longname)
{
	struct stat stat_buf;
	int stat_result = -1;
	switch (mode) {
	case STAT:  stat_result = stat(path, &stat_buf);  break;
	case LSTAT: stat_result = lstat(path, &stat_buf); break;
	};
	if (stat_result != 0) {
		return stat_result; /* errno stay set */
	}

	/* handle sftp attributes */

	int stmode = stat_buf.st_mode;

	switch (stmode & S_IFMT) {
	case S_IFREG:  attrs->type = SSH_FILEXFER_TYPE_REGULAR;   break;
	case S_IFDIR:  attrs->type = SSH_FILEXFER_TYPE_DIRECTORY; break;
	case S_IFLNK:  attrs->type = SSH_FILEXFER_TYPE_SYMLINK;   break;
	case S_IFSOCK: attrs->type = SSH_FILEXFER_TYPE_SPECIAL;   break;
	case S_IFBLK:  attrs->type = SSH_FILEXFER_TYPE_SPECIAL;   break;
	case S_IFCHR:  attrs->type = SSH_FILEXFER_TYPE_SPECIAL;   break;
	case S_IFIFO:  attrs->type = SSH_FILEXFER_TYPE_SPECIAL;   break;
	default:       attrs->type = SSH_FILEXFER_TYPE_UNKNOWN;   break;
	};

	attrs->size  = stat_buf.st_size;
	attrs->permissions = stat_buf.st_mode;
	attrs->atime = stat_buf.st_atime;
	attrs->mtime = stat_buf.st_mtime;
	attrs->flags = (SSH_FILEXFER_ATTR_SIZE
	                | SSH_FILEXFER_ATTR_PERMISSIONS
	                | SSH_FILEXFER_ATTR_ACMODTIME);

	/* handle longname */
	if (name == nullptr || longname == nullptr) return 0;

	char t = '-';
	switch (stmode & S_IFMT) {
	case S_IFDIR: t = 'd'; break;
	case S_IFLNK: t = 'l'; break;
	};

	char p[10];
	p[0] = (stmode & S_IRUSR ? 'r' : '-');
	p[1] = (stmode & S_IWUSR ? 'w' : '-');
	p[2] = (stmode & S_IXUSR ? (stmode & S_ISUID ? 's' : 'x') : '-');
	p[3] = (stmode & S_IRGRP ? 'r' : '-');
	p[4] = (stmode & S_IWGRP ? 'w' : '-');
	p[5] = (stmode & S_IXGRP ? (stmode & S_ISGID ? 's' : 'x') : '-');
	p[6] = (stmode & S_IROTH ? 'r' : '-');
	p[7] = (stmode & S_IWOTH ? 'w' : '-');
	p[8] = (stmode & S_IXOTH ? (stmode & S_ISVTX ? 't' : 'x') : '-');
	p[9] = 0;

	char time[26]; /* from man ctime_r */
	ctime_r(&stat_buf.st_mtime, time);
	char* eol = strchr(time, '\n');
	if (eol != nullptr) *eol = 0;

	::snprintf(longname, PATH_MAX, "%c%s %3d %d %d %ld %s %s",
	           t, p, (int) stat_buf.st_nlink, (int) stat_buf.st_uid,
	           (int) stat_buf.st_gid, (long int) stat_buf.st_size,
	           time, name);

	return 0;
}


void Ssh::Sftp::process_message(sftp_client_message msg)
{
	switch(msg->type){
	case SFTP_REALPATH:
		Genode::log("received realpath: ", (const char*) msg->filename);
		process_realpath(msg, PROCESS);
		break;
	case SFTP_STAT:
		Genode::log("received stat: ", (const char*) msg->filename);
		process_stat(msg, STAT);
		break;
	case SFTP_LSTAT:
		Genode::log("received lstat: ", (const char*) msg->filename);
		process_stat(msg, LSTAT);
		break;
	case SFTP_OPENDIR:
		Genode::log("received opendir: ", (const char*) msg->filename);
		process_opendir(msg);
		break;
	case SFTP_OPEN:
		Genode::log("received open: ", (const char*) msg->filename);
		process_open(msg);
		break;
	case SFTP_READDIR:
		Genode::log("received readdir");
		process_readdir(msg);
		break;
	case SFTP_READ:
		Genode::log("received read: offset=", msg->offset, ", len=", msg->len);
		process_read(msg);
		break;
	case SFTP_WRITE:
		Genode::log("received write: offset=", msg->offset, ", len=",
		            ssh_string_len(msg->data));
		process_write(msg);
		break;
	case SFTP_CLOSE:
		Genode::log("received close");
		process_close(msg);
		break;
	case SFTP_REMOVE:
		Genode::log("received remove: ", (const char*) msg->filename);
		process_remove(msg);
		break;
	case SFTP_MKDIR:
		Genode::log("received mkdir: ", (const char*) msg->filename);
		process_mkdir(msg);
		break;
	case SFTP_RMDIR:
		Genode::log("received rmdir: ", (const char*) msg->filename);
		process_rmdir(msg);
		break;

	case SFTP_SETSTAT:
	case SFTP_FSETSTAT:
	case SFTP_FSTAT:
	case SFTP_RENAME:
	case SFTP_READLINK:
	case SFTP_SYMLINK:
	default:
		Genode::log("Received unsupported message %d\n", msg->type);
		sftp_reply_status(msg, SSH_FX_OP_UNSUPPORTED, "Unsupported message");
	}
	sftp_client_message_free(msg);
}

bool Ssh::Sftp::process_realpath(sftp_client_message msg, Realpath_mode mode,
                                 bool* is_root)
{
	char* realpath_buf = msg->filename;
	char dirbuf[PATH_MAX];
	if (mode == VALIDATE_DIR) {
		size_t realpath_len = ::strlen(realpath_buf);
		if (realpath_len >= PATH_MAX) {
			if (!sftp_reply_status(msg, SSH_FX_NO_SUCH_PATH, "Path too long")) {
				Genode::error("process_realpath(): ",
				              "failed to reply path too long status");
			}
			return false;
		}

		::strcpy(dirbuf, realpath_buf);
		char* last_slash = ::strrchr(dirbuf, '/');
		if (last_slash != nullptr) {
			*last_slash = '\0';
		} else {
			dirbuf[0] = '.';
			dirbuf[1] = '\0';
		}

		realpath_buf = dirbuf;
	}

	char buffer[PATH_MAX];
	if (realpath(realpath_buf, buffer) == nullptr) {
		if (reply_errno_status(msg) != 0) {
			Genode::error("process_realpath(): failed to reply errno status");
		}
		return false;
	}

	bool root_path = ::strcmp(buffer, "/") == 0;
	if (is_root != nullptr) {
		*is_root = root_path;
	}
	bool valid_path = root_path ||
		::strncmp(buffer, valid_path_prefix, valid_path_length) == 0;

	if (!valid_path) {
		Genode::warning("Path validation failed for: ",
		                (const char*) msg->filename);
		if (!sftp_reply_status(msg, SSH_FX_NO_SUCH_PATH, "No such path")) {
			Genode::error("process_realpath(): failed to reply no such path status");
		}
		return false;
	}

	if (mode == VALIDATE || mode == VALIDATE_DIR) {
		return true;
	}

	if (sftp_reply_name(msg, buffer, NULL) != 0) {
		Genode::error("process_realpath(): failed to reply realpath");
	}

	/* result important only in validation mode */
	return true;
}

void Ssh::Sftp::process_opendir(sftp_client_message msg)
{
	bool is_root = false;
	if (!process_realpath(msg, VALIDATE, &is_root)) return;

	DIR* dir = opendir(msg->filename);
	if (dir == nullptr) {
		if (reply_errno_status(msg) != 0) {
			Genode::error("process_opendir(): failed to reply errno status");
		}
		return;
	}

	Handle* handle = new (&_heap) Handle(dir, msg->filename, is_root, _handles);
	ssh_string sftp_handle = sftp_handle_alloc(_sftp_server, handle);
	if (sftp_handle == nullptr) {
		Genode::error("process_opendir(): failed to allocate handle");
		return;
	}
	if (sftp_reply_handle(msg, sftp_handle) != 0) {
		Genode::error("process_opendir(): failed to reply handle");
	}
	ssh_string_free(sftp_handle);
}

void Ssh::Sftp::process_open(sftp_client_message msg)
{
	if (!process_realpath(msg, VALIDATE_DIR)) return;

	int flags = 0;
	bool readwrite = (msg->flags & SSH_FXF_READ && msg->flags & SSH_FXF_WRITE);
	if (readwrite)                       flags |= O_RDWR;
	else if (msg->flags & SSH_FXF_READ)  flags |= O_RDONLY;
	else if (msg->flags & SSH_FXF_WRITE) flags |= O_WRONLY;

	if (msg->flags & SSH_FXF_APPEND) flags |= O_APPEND;
	if (msg->flags & SSH_FXF_CREAT)  flags |= O_CREAT;
	if (msg->flags & SSH_FXF_TRUNC)  flags |= O_TRUNC;
	if (msg->flags & SSH_FXF_EXCL)   flags |= O_EXCL;

	mode_t mode = (msg->flags & SSH_FILEXFER_ATTR_PERMISSIONS
	               ? msg->attr->permissions : 0);

	int fd = open(msg->filename, flags, mode);
	if (fd < 0) {
		if (reply_errno_status(msg) != 0) {
			Genode::error("process_open(): failed to open file");
		}
		return;
	}

	const char* fd_mode;
	if (msg->flags & SSH_FXF_CREAT && msg->flags & SSH_FXF_APPEND)
		fd_mode = (readwrite ? "a+" : "a");
	else if (msg->flags & SSH_FXF_CREAT && msg->flags & SSH_FXF_TRUNC)
		fd_mode = (readwrite ? "w+" : "w");
	else
		fd_mode = (readwrite ? "r+" : "r");

	FILE* file = fdopen(fd, fd_mode);
	if (file == nullptr) {
		if (reply_errno_status(msg) != 0) {
			Genode::error("process_open(): failed to reply errno status");
		}
		return;
	}

	Handle* handle = new (&_heap) Handle(file, msg->filename, _handles);
	ssh_string sftp_handle = sftp_handle_alloc(_sftp_server, handle);
	if (sftp_handle == nullptr) {
		Genode::error("process_open(): failed to allocate handle");
		return;
	}
	if (sftp_reply_handle(msg, sftp_handle) != 0) {
		Genode::error("process_open(): failed to reply handle");
	}
	ssh_string_free(sftp_handle);
}

void Ssh::Sftp::process_readdir(sftp_client_message msg)
{
	Handle* handle = reinterpret_cast<Handle*>(sftp_handle(_sftp_server,
	                                                       msg->handle));
	if (handle == nullptr) {
		Genode::error("process_readdir(): received invalid handle");
		if (sftp_reply_status(msg, SSH_FX_INVALID_HANDLE, "invalid handle") != 0) {
			Genode::error("process_readdir(): ",
			              "failed to reply invalid handle status");
		}
		return;
	}
	if (handle->_type != Handle::HDIR) {
		Genode::error("process_readdir(): wrong handle type");
		if (sftp_reply_status(msg, SSH_FX_BAD_MESSAGE, "wrong handle type") != 0) {
			Genode::error("process_readdir(): ",
			              "failed to reply wrong handle type status");
		}
		return;
	}

	/* process directory entries */
	bool entry_found = false;

	/* NOTE: returning up to 10 entries in a single response is very     */
	/* safe (openssh returns up to 100 entries) but in case of very long */
	/* names maximum packet size could be exceeded; possibly maximum     */
	/* number of entries could be calculated on the fly depending on     */
	/* size of packet and lengths of file names in directory             */
	for (int i = 0; !handle->_eof && i < 10; i++) {
		struct dirent *dir = readdir(handle->_dir);
		if (dir == nullptr) {
			handle->_eof = true;
			break;
		}

		entry_found = true;

		sftp_attributes attrs = reinterpret_cast<sftp_attributes>
			(malloc(sizeof(struct sftp_attributes_struct)));
		Free_guard attr_guard(attrs);
		::memset(attrs, 0, sizeof(struct sftp_attributes_struct));

		char path[PATH_MAX];
		int reallen = ::snprintf(path, PATH_MAX, "%s/%s",
		                         handle->_name, dir->d_name);
		if (reallen >= PATH_MAX) {
			Genode::error("process_readdir(): path length too long: ",
			              (const char*) handle->_name, "/",
			              (const char*) dir->d_name);
			if (sftp_reply_status(msg, SSH_FX_FAILURE, "path too long") != 0) {
				Genode::error("process_readdir(): failed to reply path too long");
			}
			return;
		}

		char longname[PATH_MAX];
		if (get_fs_entry_info(path, LSTAT, attrs, dir->d_name, longname) != 0) {
			if (reply_errno_status(msg) != 0) {
				Genode::error("process_readdir(): failed to reply errno status");
			}
			return;
		}

		/* show only valid_path_prefix in root */
		if (handle->_root && ::strcmp(dir->d_name, valid_path_prefix + 1) != 0) {
			i--;
			continue;
		}

		if (sftp_reply_names_add(msg, dir->d_name, longname, attrs) != 0) {
			if (sftp_reply_status(msg, SSH_FX_FAILURE, "adding name failed") != 0) {
				Genode::error("process_readdir(): failed to reply adding name failed");
			}
			return;
		}
	}

	if (!entry_found && handle->_eof) {
		if (sftp_reply_status(msg, SSH_FX_EOF, nullptr) != 0) {
			Genode::error("process_readdir(): failed to reply eof");
		}
		return;
	}

	if (sftp_reply_names(msg) != 0) {
		Genode::error("process_readdir(): failed to reply names");
	}
}

void Ssh::Sftp::process_read(sftp_client_message msg)
{
	Handle* handle = reinterpret_cast<Handle*>(sftp_handle(_sftp_server,
	                                                       msg->handle));
	if (handle == nullptr) {
		Genode::error("process_read(): received invalid handle");
		if (sftp_reply_status(msg, SSH_FX_INVALID_HANDLE, "invalid handle") != 0) {
			Genode::error("process_read(): failed to reply invalid handle status");
		}
		return;
	}
	if (handle->_type != Handle::HFILE) {
		Genode::error("process_read(): wrong handle type");
		if (sftp_reply_status(msg, SSH_FX_BAD_MESSAGE, "wrong handle type") != 0) {
			Genode::error("process_read(): ",
			              "failed to reply wrong handle type status");
		}
		return;
	}

	if (fseeko(handle->_file, msg->offset, SEEK_SET) != 0) {
		if (reply_errno_status(msg) != 0) {
			Genode::error("process_read(): failed to reply errno status");
		}
		return;
	}

	constexpr int max_len = 2<<15; /* 32k */
	uint32_t len = (msg->len > max_len ? max_len : msg->len);
	void* data = malloc(len);
	if (data == nullptr) {
		if (sftp_reply_status(msg, SSH_FX_FAILURE,
		                      "memory allocation failed") != 0) {
			Genode::error("process_read(): ",
			              "failed to reply memory allocation failed");
		}
		return;
	}
	Free_guard data_guard(data);

	size_t read_len = fread(data, 1, len, handle->_file);
	Genode::log("process_read(): read ", read_len, " bytes");
	if (read_len == 0) {
		if (feof(handle->_file)) {
			if (sftp_reply_status(msg, SSH_FX_EOF, nullptr) != 0) {
				Genode::error("process_read(): failed to reply eof");
			}
			return;
		}
		if (reply_errno_status(msg) != 0) {
			Genode::error("process_read(): failed to reply errno status");
		}
		return;
	}

	if (sftp_reply_data(msg, data, read_len) != 0) {
		Genode::error("process_read(): failed to reply data");
	}
}

void Ssh::Sftp::process_write(sftp_client_message msg)
{
	Handle* handle = reinterpret_cast<Handle*>(sftp_handle(_sftp_server,
	                                                       msg->handle));
	if (handle == nullptr) {
		Genode::error("process_write(): received invalid handle");
		if (sftp_reply_status(msg, SSH_FX_INVALID_HANDLE, "invalid handle") != 0) {
			Genode::error("process_write(): failed to reply invalid handle status");
		}
		return;
	}
	if (handle->_type != Handle::HFILE) {
		Genode::error("process_write(): wrong handle type");
		if (sftp_reply_status(msg, SSH_FX_BAD_MESSAGE, "wrong handle type") != 0) {
			Genode::error("process_write(): ",
			              "failed to reply wrong handle type status");
		}
		return;
	}

	if (fseeko(handle->_file, msg->offset, SEEK_SET) != 0) {
		if (reply_errno_status(msg) != 0) {
			Genode::error("process_write(): failed to reply errno status");
		}
		return;
	}

	size_t data_len = ssh_string_len(msg->data);
	size_t write_len = fwrite(ssh_string_data(msg->data), 1, data_len,
	                          handle->_file);
	if (write_len != data_len) {
		if (reply_errno_status(msg) != 0) {
			Genode::error("process_write(): failed to reply errno status");
		}
		return;
	}

	if (sftp_reply_status(msg, SSH_FX_OK, nullptr) != 0) {
		Genode::error("process_write(): failed to reply ok status");
	}
}

template <typename T, typename DEALLOC>
class Destroyer {
	private:
		DEALLOC  dealloc;
		T       *obj;
	public:
		Destroyer(DEALLOC &&dealloc, T *obj)
			: dealloc(dealloc), obj(obj) {}
		~Destroyer() { Genode::destroy(dealloc, obj); }
};

void Ssh::Sftp::process_close(sftp_client_message msg)
{
	Handle* handle = reinterpret_cast<Handle*>(sftp_handle(_sftp_server,
	                                                       msg->handle));
	if (handle == nullptr) {
		Genode::error("process_close(): received invalid handle");
		if (sftp_reply_status(msg, SSH_FX_INVALID_HANDLE, "invalid handle") != 0) {
			Genode::error("process_close(): failed to reply invalid handle status");
		}
		return;
	}

	sftp_handle_remove(_sftp_server, msg->handle);
	Destroyer handle_destroyer(&_heap, handle);

	if (handle->_type == Handle::HDIR
	    && handle->close_dir() != 0) {
		if (reply_errno_status(msg) != 0) {
			Genode::error("process_close(): failed to reply errno status");
		}
		return;
	}

	if (handle->_type == Handle::HFILE
	    && handle->close_file() != 0) {
		if (reply_errno_status(msg) != 0) {
			Genode::error("process_close(): failed to reply errno status");
		}
		return;
	}

	if (sftp_reply_status(msg, SSH_FX_OK, nullptr) != 0) {
		Genode::error("process_close(): failed to reply ok status");
	}
}

void Ssh::Sftp::process_stat(sftp_client_message msg, Stat_mode mode)
{
	if (!process_realpath(msg, VALIDATE)) return;

	sftp_attributes attrs = reinterpret_cast<sftp_attributes>
		(malloc(sizeof(struct sftp_attributes_struct)));
	::memset(attrs, 0, sizeof(struct sftp_attributes_struct));
	Free_guard attr_guard(attrs);

	if (get_fs_entry_info(msg->filename, mode, attrs) != 0) {
		if (reply_errno_status(msg) != 0) {
			Genode::error("process_stat(): failed to reply errno status");
		}
		return;
	}

	if (sftp_reply_attr(msg, attrs) != 0) {
		Genode::error("process_stat(): failed to reply attr");
	}
}

void Ssh::Sftp::process_remove(sftp_client_message msg)
{
	if (!process_realpath(msg, VALIDATE)) return;

	if (unlink(msg->filename) != 0) {
		if (reply_errno_status(msg) != 0) {
			Genode::error("process_remove(): failed to reply errno status");
		}
		return;
	}

	if (sftp_reply_status(msg, SSH_FX_OK, nullptr) != 0) {
		Genode::error("process_remove(): failed to reply ok status");
	}
}

void Ssh::Sftp::process_mkdir(sftp_client_message msg)
{
	if (!process_realpath(msg, VALIDATE_DIR)) return;

	mode_t mode = (msg->flags & SSH_FILEXFER_ATTR_PERMISSIONS
	               ? msg->attr->permissions : 0);

	if (mkdir(msg->filename, mode) != 0) {
		if (reply_errno_status(msg) != 0) {
			Genode::error("process_mkdir(): failed to reply errno status");
		}
		return;
	}

	if (sftp_reply_status(msg, SSH_FX_OK, nullptr) != 0) {
		Genode::error("process_mkdir(): failed to reply ok status");
	}
}

void Ssh::Sftp::process_rmdir(sftp_client_message msg)
{
	if (!process_realpath(msg, VALIDATE)) return;

	if (rmdir(msg->filename) != 0) {
		if (reply_errno_status(msg) != 0) {
			Genode::error("process_rmdir(): failed to reply errno status");
		}
		return;
	}

	if (sftp_reply_status(msg, SSH_FX_OK, nullptr) != 0) {
		Genode::error("process_rmdir(): failed to reply ok status");
	}
}
