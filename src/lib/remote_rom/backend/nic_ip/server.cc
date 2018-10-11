/*
 * \brief Server implementation
 * \author Johannes Schlatow
 * \date   2018-11-06
 */

#include <backend_base.h>
#include <base.h>

namespace Remote_rom {
	using  Genode::Cstring;

	struct Backend_server;
};

class Remote_rom::Backend_server :
  public Backend_server_base,
  public Backend_base
{
	private:

		Rom_forwarder_base         *_forwarder;

		Backend_server(Backend_server &);
		Backend_server &operator= (Backend_server &);

		void send_data()
		{
			if (!_forwarder) return;

			size_t offset = 0;
			size_t size = _forwarder->content_size();
			while (offset < size)
			{
				/* create and transmit packet via NIC session */
				size_t max_size = _upper_layer_size(sizeof(Packet)
				                                    + DataPacket::packet_size(size));
				Nic::Packet_descriptor pd = alloc_tx_packet(max_size);
				Size_guard size_guard(pd.size());

				char *content = _nic.tx()->packet_content(pd);
				Ipv4_packet &ip = _prepare_upper_layers(content, size_guard);
				Packet &pak = ip.construct_at_data<Packet>(size_guard);
				pak.type(Packet::DATA);
				pak.module_name(_forwarder->module_name());

				DataPacket &data = pak.construct_at_data<DataPacket>(size_guard);
				data.offset(offset);
				data.content_size(size);

				const size_t max = DataPacket::MAX_PAYLOAD_SIZE;
				data.payload_size(_forwarder->transfer_content((char*)data.addr(),
				                                               max,
				                                               offset));
				_finish_ipv4(ip, sizeof(Packet) + data.size());

				submit_tx_packet(pd);

				offset += data.payload_size();
			}
		}


		void receive(Packet &packet, Size_guard &)
		{
			switch (packet.type())
			{
				case Packet::UPDATE:
					if (_verbose)
						Genode::log("receiving UPDATE (",
						            Cstring(packet.module_name()),
						            ") packet");

					if (!_forwarder)
						return;

					/* check module name */
					if (Genode::strcmp(packet.module_name(), _forwarder->module_name()))
						return;

					send_data();

					break;
				default:
					break;
			}
		}

	public:

		Backend_server(Genode::Env &env, Genode::Allocator &alloc) :
		  Backend_base(env, alloc),
		  _forwarder(nullptr)
		{	}


		void register_forwarder(Rom_forwarder_base *forwarder)
		{
			_forwarder = forwarder;
		}


		void send_update()
		{
			if (!_forwarder) return;
			_transmit_notification_packet(Packet::SIGNAL, _forwarder);
		}
};

namespace Remote_rom {
	using Genode::Env;
	using Genode::Allocator;

	Backend_server_base &backend_init_server(Env &env, Allocator &alloc)
	{
		static Backend_server backend(env, alloc);
		return backend;
	}
};
