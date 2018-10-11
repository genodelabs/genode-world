/*
 * \brief Client implementation
 * \author Johannes Schlatow
 * \date   2018-11-06
 */

#include <backend_base.h>
#include <base.h>

namespace Remote_rom {
	using  Genode::Cstring;

	struct Backend_client;
};

class Remote_rom::Backend_client :
  public Backend_client_base,
  public Backend_base
{
	private:

		Rom_receiver_base          *_receiver;
		char                       *_write_ptr;
		size_t                     _buf_size;

		Backend_client(Backend_client &);
		Backend_client &operator= (Backend_client &);

		void write(const void *data, size_t offset, size_t size)
		{
			if (!_write_ptr) return;


			size_t const len = Genode::min(size, _buf_size-offset);
			Genode::memcpy(_write_ptr+offset, data, len);

			if (offset + len >= _buf_size)
				_receiver->commit_new_content();
		}


		void update(const char* module_name)
		{
			if (!_receiver) return;

			/* check module name */
			if (Genode::strcmp(module_name, _receiver->module_name()))
				return;

			_transmit_notification_packet(Packet::UPDATE, _receiver);
		}


		void receive(Packet &packet, Size_guard &size_guard)
		{
			switch (packet.type())
			{
				case Packet::SIGNAL:
					if (_verbose)
						Genode::log("receiving SIGNAL(",
						            Cstring(packet.module_name()),
						            ") packet");

					/* send update request */
					update(packet.module_name());

					break;
				case Packet::DATA:
					{
						if (_verbose)
							Genode::log("receiving DATA(",
							            Cstring(packet.module_name()),
							            ") packet");

						/* write into buffer */
						if (!_receiver) return;

						/* check module name */
						if (Genode::strcmp(packet.module_name(), _receiver->module_name()))
							return;

						const DataPacket &data = packet.data<DataPacket>(size_guard);
						size_guard.consume_head(data.payload_size());

						if (!data.offset()) {
							_write_ptr = _receiver->start_new_content(data.content_size());
							_buf_size  = (_write_ptr) ? data.content_size() : 0;
						}

						write(data.addr(), data.offset(), data.payload_size());

						break;
					}
				default:
					break;
			}
		}

	public:

		Backend_client(Genode::Env &env, Genode::Allocator &alloc) :
		  Backend_base(env, alloc),
		  _receiver(nullptr), _write_ptr(nullptr),
		  _buf_size(0)
		{ }


		void register_receiver(Rom_receiver_base *receiver)
		{
			/* TODO support multiple receivers (ROM names) */
			_receiver = receiver;

			/*
			 * FIXME request update on startup
			 * (occasionally triggers invalid signal-context capability)
			 * */
//			if (_receiver)
//				update(_receiver->module_name());
		}
};

namespace Remote_rom {
	using Genode::Env;
	using Genode::Allocator;

	Backend_client_base &backend_init_client(Env &env, Allocator &alloc)
	{
		static Backend_client backend(env, alloc);
		return backend;
	}
};
