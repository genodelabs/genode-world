/*
 * \brief Packet format we use for inter-system communication
 * \author Johannes Schlatow
 * \date   2018-11-05
 */

#include <util/construct_at.h>
#include <util/string.h>
#include <net/size_guard.h>

#ifndef __INCLUDE__REMOTE_ROM__PACKET_H_
#define __INCLUDE__REMOTE_ROM__PACKET_H_

namespace Remote_rom {
	using Genode::size_t;
	using Genode::uint16_t;
	using Genode::uint32_t;

	struct Packet;
	struct DataPacket;
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
		} Type;

	private:
		char         _module_name[MAX_NAME_LEN];   /* the ROM module name */
		Type         _type;                        /* packet type */

		/*****************************************************
		 ** 'payload' must be the last member of this class **
		 *****************************************************/

		char payload[0];

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

		void module_name(const char *module)
		{
			Genode::strncpy(_module_name, module, MAX_NAME_LEN);
		}

		template <typename T>
		T const &data(Net::Size_guard &size_guard) const
		{
			size_guard.consume_head(sizeof(T));
			return *(T const *)(payload);
		}

		template <typename T>
		T &construct_at_data(Net::Size_guard &size_guard)
		{
			size_guard.consume_head(sizeof(T));
			return *Genode::construct_at<T>(payload);
		}

} __attribute__((packed));


class Remote_rom::DataPacket
{
	public:
		static const size_t MAX_PAYLOAD_SIZE = 1024;

	private:
		uint32_t     _content_size;                /* ROM content size in bytes */
		uint32_t     _offset;                      /* offset in bytes */
		uint16_t     _payload_size;                /* payload size in bytes */

		char payload[0];

	public:
		/**
		 * Return size of the packet
		 */
		size_t size() const { return _payload_size + sizeof(*this); }

		/**
		 * Return content_size of the packet
		 */
		size_t content_size() const { return _content_size; }

		/**
		 * Return offset of the packet
		 */
		size_t offset() const { return _offset; }

		void content_size(size_t size) { _content_size = size; }
		void offset(size_t offset) { _offset = offset; }

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
		void *addr() { return payload; }
		const void *addr() const { return payload; }

		/**
		 * Return packet size for given payload
		 */
		static size_t packet_size(size_t payload) {
			return sizeof(DataPacket) + Genode::min(payload, MAX_PAYLOAD_SIZE);
		}

} __attribute__((packed));

#endif
