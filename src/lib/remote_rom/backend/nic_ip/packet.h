/*
 * \brief Packet format we use for inter-system communication
 * \author Johannes Schlatow
 * \author Edgard Schmidt
 * \date   2018-11-05
 */

#include <util/construct_at.h>
#include <util/string.h>
#include <net/size_guard.h>

#ifndef __INCLUDE__REMOTE_ROM__PACKET_H_
#define __INCLUDE__REMOTE_ROM__PACKET_H_

namespace Remote_rom {
	using Genode::size_t;
	using Genode::uint8_t;
	using Genode::uint16_t;
	using Genode::uint32_t;

	class Window_state;

	class Packet;
	class NotificationPacket;
	class AckPacket;
	class DataPacket;
}


class Remote_rom::Packet
{
	public:
		enum {
			MAX_NAME_LEN = 64        /* maximum length of the module name */
		};

		typedef enum {
			SIGNAL    = 1,           /* signal that ROM content has changed     */
			UPDATE    = 2,           /* request transmission of updated content */
			DATA      = 3,           /* data packet                             */
			ACK       = 4,           /* acknowledge data packets                */
		} Type;

	private:
		char         _module_name[MAX_NAME_LEN];   /* the ROM module name */
		uint32_t     _content_hash;                /* serves as version */
		Type         _type;                        /* packet type */

		char         _data[0];

	public:
		/**
		 * Return type of the packet
		 */
		Type type() const { return _type; }

		/**
		 * Return module_name of the packet
		 */
		const char *module_name() { return _module_name; }

		void type(Type type)
		{
			_type = type;
		}

		void     content_hash(uint32_t hash) { _content_hash = hash; }
		uint32_t content_hash() const        { return _content_hash; }

		void module_name(const char *module)
		{
			Genode::strncpy(_module_name, module, MAX_NAME_LEN);
		}

		template <typename T>
		T const &data(Net::Size_guard &size_guard) const
		{
			size_guard.consume_head(sizeof(T));
			return *(T const *)(_data);
		}

		template <typename T>
		T &construct_at_data(Net::Size_guard &size_guard)
		{
			size_guard.consume_head(sizeof(T));
			return *Genode::construct_at<T>(_data);
		}

} __attribute__((packed));


class Remote_rom::NotificationPacket
{
	private:
		uint32_t     _content_size;   /* ROM content size in bytes */

	public:

		void   content_size(size_t size) { _content_size = size; }
		size_t content_size() const      { return _content_size; }

} __attribute__((packed));

class Remote_rom::AckPacket
{
	private:
		uint16_t     _window_id;   /* refers to this window id */
		uint16_t     _ack_until;   /* acknowledge until this packet id - 1 */

		/**
		 * TODO Implement selective ARQ by using a Bit_array to save
		 *      the ACK state in the window.
		 *      We should, however, not use the Bit_array directly as a
		 *      Packet member.
		 */

	public:

		void window_id(size_t window_id) { _window_id = window_id; }
		void ack_until(size_t packet_id) { _ack_until = packet_id; }

		size_t window_id() const { return _window_id; }
		size_t ack_until() const { return _ack_until; }

} __attribute__((packed));

class Remote_rom::DataPacket
{
	public:
		static const size_t MAX_PAYLOAD_SIZE = 1350;

	private:
		uint16_t     _payload_size;    /* payload size in bytes */
		uint16_t     _window_id;       /* window id */
		uint16_t     _packet_id;       /* packet number within window */
		uint16_t     _window_length;   /* 0: no ARQ, >0: ARQ window length */

		char _data[0];

	public:
		/**
		 * Return size of the packet
		 */
		size_t size() const { return _payload_size + sizeof(*this); }

		void   window_length(size_t len) { _window_length = len; }
		void   window_id(size_t id)      { _window_id = id; }
		void   packet_id(size_t id)      { _packet_id = id; }

		size_t window_length() const { return _window_length; }
		size_t window_id()     const { return _window_id; }
		size_t packet_id()     const { return _packet_id; }

		/**
		 * Set payload size of the packet
		 */
		void payload_size(Genode::size_t payload_size)
		{
			_payload_size = payload_size;
		}

		/**
		 * Return payload size of the packet
		 */
		size_t payload_size() const { return _payload_size; }

		/**
		* Return address of the payload
		*/
		void *addr() { return _data; }
		const void *addr() const { return _data; }

} __attribute__((packed));

#endif
