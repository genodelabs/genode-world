/*
 * \brief  MBIM connection bindings
 * \author Sebastian Sumpf
 * \date   2020-10-27
 */

/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <libc/component.h>
#include <os/reporter.h>
#include <net/ipv4.h>

/* libc includes */
#include <libc/args.h>
#include <stdlib.h> /* 'exit'   */

#include <libmbim-glib.h>
extern "C" {
#include <mbimcli.h>
}

class Mbim
{
	enum { TRACE = FALSE };

	enum State { NONE, PIN, QUERY, ATTACH, CONNECT };

	using String = Genode::String<32>;

	struct Connection
	{
		Net::Ipv4_address ip;
		Genode::uint32_t  mask;
		Net::Ipv4_address gateway;
		Net::Ipv4_address dns[2];
	};

	struct Network
	{
		String apn;
		String user;
		String password;
		String pin;
	};

	private:

		Genode::Env     &_env;
		Genode::Reporter _reporter { _env, "config", "nic_router.config" };

		State       _state { NONE };
		GMainLoop  *_loop { nullptr };
		MbimDevice *_device { nullptr };
		unsigned    _retry { 0 };
		guint32     _session_id { 0 };
		Connection  _connection { };

		Genode::Attached_rom_dataspace _config_rom { _env, "config" };
		Network                        _network { };

		static Mbim *_mbim(gpointer user_data)
		{
			return reinterpret_cast<Mbim *>(user_data);
		}

		MbimMessage *_command_response(GAsyncResult *res, bool may_fail = false)
		{
			GError *error         = nullptr;
			MbimMessage *response = mbim_device_command_finish (_device, res, &error);

			if (!response ||
			    !mbim_message_response_get_result(response, MBIM_MESSAGE_TYPE_COMMAND_DONE, &error)) {
				if (may_fail) return nullptr;
				Genode::error("operation failed: ", (char const*)error->message);
				_shutdown(FALSE);
				return nullptr;
			}

			return response;
		}

		gchar const *_pin()
		{
			//XXX: hide
			return _network.pin.string();
		}

		static void _device_close_ready (MbimDevice   *dev,
		                                 GAsyncResult *res, gpointer user_data)
		{
			Mbim *mbim = reinterpret_cast<Mbim *>(user_data);
			g_autoptr(GError) error = nullptr;
			if (!mbim_device_close_finish(mbim->_device, res, &error)) {
				Genode::error("couldn't close device: ", (char const*)error->message);
				g_error_free(error);
			}

			g_main_loop_quit(mbim->_loop);
		}

		void _shutdown(gboolean operation_status)
		{
			/* Set the in-session setup */
			g_object_set(_device,
			             MBIM_DEVICE_IN_SESSION, FALSE,
			             nullptr);

			/* Close the device */
			mbim_device_close(_device,
			                  15,
			                  nullptr,
			                  (GAsyncReadyCallback) _device_close_ready,
			                  this);
		}

		void _send_request()
		{
			g_autoptr(MbimMessage) request = nullptr;
			g_autoptr(GError)      error   = nullptr;

			switch (_state) {

				case NONE:
					request = (mbim_message_pin_set_new(MBIM_PIN_TYPE_PIN1,
					                                    MBIM_PIN_OPERATION_ENTER,
					                                    _pin(),
					                                    nullptr,
					                                    &error));
					if (!request) {
						Genode::error("couldn't create request: ", (char const*)error->message);
						_shutdown (FALSE);
						return;
					}

					mbim_device_command (_device,
					                     request,
					                     10,
					                     nullptr,
					                     (GAsyncReadyCallback)_pin_ready,
					                     this);
					break;

				case PIN:
					request = mbim_message_register_state_query_new(nullptr);
					mbim_device_command(_device,
					                    request,
					                    10,
					                    nullptr,
					                    (GAsyncReadyCallback)_register_state,
					                    this);
					break;

				case QUERY:
					request = mbim_message_packet_service_set_new(MBIM_PACKET_SERVICE_ACTION_ATTACH
					                                              , &error);
					if (!request) {
						Genode::error("couldn't create request: ", (char const *)error->message);
						_shutdown(FALSE);
						return;
					}

					mbim_device_command(_device,
					                    request,
					                    120,
					                    nullptr,
					                    (GAsyncReadyCallback)_packet_service_ready,
					                    this);
					break;

				case ATTACH: {
					guint32            session_id { 0 };
					g_autofree gchar  *apn { (gchar *)_network.apn.string() };
					MbimAuthProtocol   auth_protocol { MBIM_AUTH_PROTOCOL_PAP };
					g_autofree gchar  *username { (char *)_network.user.string() };
					g_autofree gchar  *password { (char *)_network.password.string() };
					MbimContextIpType  ip_type { MBIM_CONTEXT_IP_TYPE_DEFAULT };

					request = mbim_message_connect_set_new(session_id,
					                                       MBIM_ACTIVATION_COMMAND_ACTIVATE,
					                                       apn,
					                                       username,
					                                       password,
					                                       MBIM_COMPRESSION_NONE,
					                                       auth_protocol,
					                                       ip_type,
					                                       mbim_uuid_from_context_type (MBIM_CONTEXT_TYPE_INTERNET),
					                                       &error);

					if (!request) {
						Genode::error("couldn't create request: ", (char const *)error->message);
						_shutdown (FALSE);
						return;
					}

					mbim_device_command (_device,
					                     request,
					                     120,
					                     nullptr,
					                     (GAsyncReadyCallback)_connect_ready,
					                     this);
					break;
				}

				case CONNECT:
					request = (mbim_message_ip_configuration_query_new (
					             _session_id,
					             MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_NONE, /* ipv4configurationavailable */
					             MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_NONE, /* ipv6configurationavailable */
					             0, /* ipv4addresscount */
					             nullptr, /* ipv4address */
					             0, /* ipv6addresscount */
					             nullptr, /* ipv6address */
					             nullptr, /* ipv4gateway */
					             nullptr, /* ipv6gateway */
					             0, /* ipv4dnsservercount */
					             nullptr, /* ipv4dnsserver */
					             0, /* ipv6dnsservercount */
					             nullptr, /* ipv6dnsserver */
					             0, /* ipv4mtu */
					             0, /* ipv6mtu */
					             &error));
					if (!request) {
						Genode::error("couldn't create IP config request: ", (char const *)error->message);
						_shutdown (FALSE);
						return;
					}

					mbim_device_command (_device,
					                     request,
					                     60,
					                     nullptr,
					                     (GAsyncReadyCallback)_ip_configuration_query_ready,
					                     this);
			}
		}

		static void _pin_ready(MbimDevice   *dev,
		                       GAsyncResult *res, gpointer user_data)
		{
			Mbim *mbim = _mbim(user_data);
			g_autoptr(GError)      error    = nullptr;
			g_autoptr(MbimMessage) response = mbim->_command_response(res, true);

			if (!response) {
				Genode::log("PIN might be entered already");
				mbim->_state = Mbim::PIN;
				mbim->_send_request();
				return;
			}

			MbimPinType  pin_type;
			MbimPinState pin_state;
			guint32      remaining_attempts;
			if (!mbim_message_pin_response_parse(response,
			                                     &pin_type,
			                                     &pin_state,
			                                     &remaining_attempts,
			                                     &error)) {
				Genode::error("couldn't parse response message: ", (char const *)error->message);
				mbim->_shutdown (FALSE);
				return;
			}

			if (TRACE)
				Genode::log("PIN: state: ", mbim_pin_state_get_string(pin_state),
				            " remaining attempts: ", remaining_attempts);
			mbim->_state = Mbim::PIN;
			mbim->_send_request();
		}

		static void _register_state(MbimDevice   *dev,
		                            GAsyncResult *res, gpointer user_data)
		{
			Mbim *mbim = _mbim(user_data);
			g_autoptr(GError)      error    = nullptr;
			g_autoptr(MbimMessage) response = mbim->_command_response(res);

			if (!response) return;

			MbimNwError          nw_error;
			MbimRegisterState    register_state;
			MbimRegisterMode     register_mode;
			MbimDataClass        available_data_classes;
			MbimCellularClass    cellular_class;
			g_autofree gchar    *provider_id = nullptr;
			g_autofree gchar    *provider_name = nullptr;
			g_autofree gchar    *roaming_text = nullptr;
			MbimRegistrationFlag registration_flag;
			if (!mbim_message_register_state_response_parse(response,
			                                                &nw_error,
			                                                &register_state,
			                                                &register_mode,
			                                                &available_data_classes,
			                                                &cellular_class,
			                                                &provider_id,
			                                                &provider_name,
			                                                &roaming_text,
			                                                &registration_flag,
			                                                &error)) {
				Genode::error("couldn't parse response message: ", (char const *)error->message);
				mbim->_shutdown (FALSE);
				return;
			}

			if (register_state == MBIM_REGISTER_STATE_HOME ||
			    register_state == MBIM_REGISTER_STATE_ROAMING ||
			    register_state == MBIM_REGISTER_STATE_PARTNER)
				mbim->_state = Mbim::QUERY;

			if (mbim->_retry++ >= 100) {
				Genode::error("Device not registered after ", mbim->_retry, " tries");
				mbim->_shutdown(FALSE);
			}

			mbim->_send_request();
		}

		static void _packet_service_ready(MbimDevice   *dev,
		                                  GAsyncResult *res, gpointer user_data)
		{
			Mbim *mbim = _mbim(user_data);
			GError *error                   = nullptr;
			g_autoptr(MbimMessage) response = mbim->_command_response(res);

			if (!response) return;

			guint32                 nw_error;
			MbimPacketServiceState  packet_service_state;
			MbimDataClass           highest_available_data_class;
			g_autofree gchar       *highest_available_data_class_str = nullptr;
			guint64                 uplink_speed;
			guint64                 downlink_speed;
			if (!mbim_message_packet_service_response_parse(response,
			                                                &nw_error,
			                                                &packet_service_state,
			                                                &highest_available_data_class,
			                                                &uplink_speed,
			                                                &downlink_speed,
			                                                &error)) {
				Genode::error("couldn't parse response message: ", (char const *)error->message);
				mbim->_shutdown (FALSE);
				return;
			}

			mbim->_state = Mbim::ATTACH;

			highest_available_data_class_str = mbim_data_class_build_string_from_mask (highest_available_data_class);
			Genode::log("Successfully attached packet service");

			if (TRACE)
				Genode::log("Packet service status:\n",
				            "\tAvailable data classes: '", (char const *)highest_available_data_class_str, "'\n",
				            "\t          Uplink speed: '", uplink_speed, "'\n",
				            "\t        Downlink speed: '", downlink_speed, "'");

			mbim->_send_request();
		}

		static void _connect_ready(MbimDevice   *dev,
		                           GAsyncResult *res, gpointer user_data)
		{
			Mbim *mbim = _mbim(user_data);
			GError *error                   = nullptr;
			g_autoptr(MbimMessage) response = mbim->_command_response(res);

			if (!response) return;

			guint32              session_id;
			MbimActivationState  activation_state;
			MbimVoiceCallState   voice_call_state;
			MbimContextIpType    ip_type;
			const MbimUuid      *context_type;
			guint32              nw_error;
			if (!mbim_message_connect_response_parse (
			        response,
			        &session_id,
			        &activation_state,
			        &voice_call_state,
			        &ip_type,
			        &context_type,
			        &nw_error,
			        &error)) {
				Genode::error("couldn't parse response message: ", (char const *)error->message);
				mbim->_shutdown(FALSE);
				return;
			}

			mbim->_session_id = session_id;
			mbim->_state      = Mbim::CONNECT;
			mbim->_send_request();
		}

		static void _ip_configuration_query_ready(MbimDevice   *dev,
		                                          GAsyncResult *res,
		                                          gpointer user_data)
		{
			Mbim *mbim = _mbim(user_data);
			GError *error                   = nullptr;
			g_autoptr(MbimMessage) response = mbim->_command_response(res);

			if (!response) return;

			MbimIPConfigurationAvailableFlag  ipv4configurationavailable;
			g_autofree gchar                 *ipv4configurationavailable_str = nullptr;
			MbimIPConfigurationAvailableFlag  ipv6configurationavailable;
			g_autofree gchar                 *ipv6configurationavailable_str = nullptr;
			guint32                           ipv4addresscount;
			g_autoptr(MbimIPv4ElementArray)   ipv4address = nullptr;
			guint32                           ipv6addresscount;
			g_autoptr(MbimIPv6ElementArray)   ipv6address = nullptr;
			const MbimIPv4                   *ipv4gateway;
			const MbimIPv6                   *ipv6gateway;
			guint32                           ipv4dnsservercount;
			g_autofree MbimIPv4              *ipv4dnsserver = nullptr;
			guint32                           ipv6dnsservercount;
			g_autofree MbimIPv6              *ipv6dnsserver = nullptr;
			guint32                           ipv4mtu;
			guint32                           ipv6mtu;

			if (!mbim_message_ip_configuration_response_parse(
			        response,
			        nullptr, /* sessionid */
			        &ipv4configurationavailable,
			        &ipv6configurationavailable,
			        &ipv4addresscount,
			        &ipv4address,
			        &ipv6addresscount,
			        &ipv6address,
			        &ipv4gateway,
			        &ipv6gateway,
			        &ipv4dnsservercount,
			        &ipv4dnsserver,
			        &ipv6dnsservercount,
			        &ipv6dnsserver,
			        &ipv4mtu,
			        &ipv6mtu,
			        &error))
			return;

			if (!ipv4configurationavailable) {
				Genode::error("No ipv4 configuration available");
				return;
			}

			Net::Ipv4_address address { ipv4address[0]->ipv4_address.addr };

			Genode::uint32_t netmask_lower = 32 - ipv4address[0]->on_link_prefix_length;
			Genode::uint32_t netmask = ~0u;
			for (Genode::uint32_t i = 0; i < netmask_lower; i++) {
				netmask ^= (1 << i);
			}

			Net::Ipv4_address mask { Net::Ipv4_address::from_uint32_little_endian(netmask) };
			Net::Ipv4_address gateway { (void *)ipv4gateway->addr };
			Genode::log("ip     : ", address);
			Genode::log("mask   : ", mask);
			Genode::log("gateway: ", gateway);

			Net::Ipv4_address dns[2];
			for (Genode::uint32_t i = 0; i < ipv4dnsservercount && i < 2; i++) {
				dns[i] = Net::Ipv4_address { (void *)ipv4dnsserver[i].addr };
				Genode::log("dns", i, "   : ", dns[i]);
			}

			mbim->_connection.ip      = address;
			mbim->_connection.mask    = ipv4address[0]->on_link_prefix_length;
			mbim->_connection.gateway = gateway;
			mbim->_connection.dns[0]  = dns[0];
			mbim->_connection.dns[1]  = dns[1];
			mbim->_report();
		}

		static void _device_open_ready(MbimDevice   *dev,
		                               GAsyncResult *res, gpointer user_data)
		{
			GError *error = nullptr;
			Mbim *mbim = reinterpret_cast<Mbim *>(user_data);
			if (!mbim_device_open_finish(dev, res, &error)) {
				Genode::error("couldn't open the MbimDevice: ",
				              (char const *)error->message);
				exit (EXIT_FAILURE);
			}

			mbim->_retry = 0;
			mbim->_send_request();
		}


		static void _device_new_ready(GObject *unsused, GAsyncResult *res, gpointer user_data)
		{
			Mbim *mbim = reinterpret_cast<Mbim *>(user_data);
			GError *error = nullptr;
			MbimDeviceOpenFlags open_flags = MBIM_DEVICE_OPEN_FLAGS_NONE;

			mbim->_device = mbim_device_new_finish (res, &error);
			if (!mbim->_device) {
				Genode::error("couldn't create MbimDevice: ",
				              (char const*)error->message);
				exit (EXIT_FAILURE);
			}

			mbim_device_open_full(mbim->_device,
			                      open_flags,
			                      30,
			                      nullptr,
			                      (GAsyncReadyCallback) _device_open_ready,
			                      mbim);
		}

		static void _log_handler(const gchar *log_domain,
		                         GLogLevelFlags log_level,
		                         const gchar *message,
		                         gpointer user_data)
		{
			Genode::String<32> level;
			switch (log_level) {
				case G_LOG_LEVEL_WARNING:
					level = "[Warning]";
					break;
				case G_LOG_LEVEL_CRITICAL:
				case G_LOG_LEVEL_ERROR:
					level = "[Error]";
					break;
				case G_LOG_LEVEL_DEBUG:
					level = "[Debug]";
					break;
				case G_LOG_LEVEL_MESSAGE:
				case G_LOG_LEVEL_INFO:
					level = "[Info]";
				break;
				case G_LOG_FLAG_FATAL:
				case G_LOG_LEVEL_MASK:
				case G_LOG_FLAG_RECURSION:
				default:
					g_assert_not_reached ();
			}

				Genode::log(level, " ", message);
		}

		void _init()
		{
			if (TRACE) {
				g_log_set_handler(nullptr, G_LOG_LEVEL_MASK, _log_handler, nullptr);
				g_log_set_handler("Mbim", G_LOG_LEVEL_MASK, _log_handler, nullptr);
			}
			mbim_utils_set_traces_enabled(TRACE);

			g_autoptr(GFile) file = nullptr;
			file = g_file_new_for_commandline_arg ("/dev/cdc-wdm0");
			_loop = g_main_loop_new(nullptr, FALSE);
			mbim_device_new(file, nullptr, (GAsyncReadyCallback)_device_new_ready, this);
		}

		void _connect()
		{
			g_main_loop_run(_loop);
			g_object_unref(_device);
			g_main_loop_unref(_loop);
		}

		void _report()
		{
			_reporter.enabled(true);
			try {
				Genode::Reporter::Xml_generator xml(_reporter, [&]() {
				xml.attribute("verbose", "no");
				xml.attribute("verbose_packets", "no");

					xml.node("default-policy", [&] () {
						xml.attribute("domain", "default");
					});
					xml.node("uplink", [&] () {
						xml.attribute("domain", "uplink");
					});
					/* uplink */
					xml.node("domain", [&] () {
						xml.attribute("name", "uplink");
						Genode::String<18> ip { _connection.ip, "/", _connection.mask };
						xml.attribute("interface", ip);
						Genode::String<15> gw { _connection.gateway };
						xml.attribute("gateway", gw);
						/* no ARP */
						xml.attribute("use_arp", "no");

						xml.node("nat", [&] () {
							xml.attribute("domain", "default");
							xml.attribute("tcp-ports", "100");
							xml.attribute("udp-ports", "100");
							xml.attribute("icmp-ids", "100");
						});
					});

					/* default */
					xml.node("domain", [&] () {
						xml.attribute("name", "default");
						xml.attribute("interface", "10.0.3.1/24");

						xml.node("dhcp-server", [&] () {
							xml.attribute("ip_first", "10.0.3.2");
							xml.attribute("ip_last", "10.0.3.200");

							xml.node("dns-server", [&] () {
								xml.attribute("ip", Genode::String<15>(_connection.dns[0]));
							});

							xml.node("dns-server", [&] () {
								xml.attribute("ip", Genode::String<15>(_connection.dns[1]));
							});
						});

						xml.node("tcp", [&] () {
							xml.attribute("dst", "0.0.0.0/0");
							xml.node("permit-any", [&] () {
								xml.attribute("domain", "uplink");
							});
						});
						xml.node("udp", [&] () {
							xml.attribute("dst", "0.0.0.0/0");
							xml.node("permit-any", [&] () {
								xml.attribute("domain", "uplink");
							});
						});
						xml.node("icmp", [&] () {
							xml.attribute("dst", "0.0.0.0/0");
							xml.attribute("domain", "uplink");
						});
					});
				}); /* reporter */
			}
			catch (...) { Genode::warning("Could not report NIC router configuration"); }
		}

	public:

		Mbim(Libc::Env &env) : _env(env)
		{
			try {
				Genode::Xml_node const &xml = _config_rom.xml();
				Genode::Xml_node const &net = xml.sub_node("network");

				_network.apn      = net.attribute_value("apn", String());
				_network.user     = net.attribute_value("user", String());
				_network.password = net.attribute_value("password", String());
				_network.pin      = net.attribute_value("pin", String());
			} catch (...) {
				Genode::error("No valid <network> configuration found");
				exit(1);
			}

			_init();
			_connect();
			exit(0);
		}
};


void Libc::Component::construct(Libc::Env &env)
{
	Libc::with_libc([&] () {
		static Mbim main { env };
	});
}
