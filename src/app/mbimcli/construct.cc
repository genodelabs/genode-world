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

	enum State { NONE, UNLOCK, PIN, QUERY, ATTACH, CONNECT, READY };

	enum Backoff {
		BACKOFF_START = 1000,    /* first retry after one second */
		BACKOFF_LIMIT = 64000,   /* increase retry timeout up to 64 seconds */
		RSSI_DISCONNECT = 31,
	};

	using String  = Genode::String<32>;
	using Cstring = Genode::Cstring;

	struct Connection
	{
		Net::Ipv4_address ip;
		Genode::uint32_t  mask;
		Net::Ipv4_address gateway;
		Net::Ipv4_address dns[2];
		bool              connected;
	};

	struct Network
	{
		String apn;
		String user;
		String password;
		String pin;
	};

	struct State_report
	{
		String sim        { };
		String error      { };
		String network    { };
		String provider   { };
		String data_class { };
		String roaming    { };
		Genode::uint32_t rssi       { 99 };
		Genode::uint32_t error_rate { 99 };
	};

	private:

		Genode::Env      &_env;
		Genode::Reporter  _config_reporter { _env, "config", "nic_router.config" };
		Genode::Reporter  _state_reporter  { _env, "state",  "state" };

		State       _state      { NONE };
		GMainLoop  *_loop       { nullptr };
		MbimDevice *_device     { nullptr };
		unsigned    _retry      { 0 };
		unsigned    _backoff    { Mbim::BACKOFF_START };
		guint32     _session_id { 0 };
		Connection  _connection { };

		Genode::Attached_rom_dataspace _config_rom   { _env, "config" };
		Network                        _network      { };
		State_report                   _state_report { };

		Genode::Signal_handler<Mbim> _config_handler {
			_env.ep(), *this, &Mbim::_report_config };

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
					request = mbim_message_subscriber_ready_status_query_new(nullptr);
					if (!request) {
						Genode::error("couldn't create request: ", (char const*)error->message);
						_shutdown (FALSE);
						return;
					}
					
					mbim_device_command(_device,
					                    request,
					                    10,
					                    nullptr,
					                    (GAsyncReadyCallback)_subscriber_state,
					                    this);
					break;

				case UNLOCK:
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
					if (!request) {
						Genode::error("couldn't create request: ", (char const*)error->message);
						_shutdown (FALSE);
						return;
					}

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
					gchar             *apn { (gchar *)_network.apn.string() };
					MbimAuthProtocol   auth_protocol { MBIM_AUTH_PROTOCOL_PAP };
					gchar             *username { (char *)_network.user.string() };
					gchar             *password { (char *)_network.password.string() };
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
					break;
				case READY:
					break;
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

			mbim->_state_report.sim = mbim_pin_state_get_string(pin_state);
			mbim->_report_state();

			if (pin_state != MBIM_PIN_STATE_UNLOCKED) {
				Genode::error("Unable to unlock SIM card. Wrong PIN?"
				              " Remaining attempts: ", remaining_attempts);
				mbim->_shutdown(FALSE);
				return;
			}

			if (TRACE)
				Genode::log("PIN: state: ", mbim_pin_state_get_string(pin_state),
				            " remaining attempts: ", remaining_attempts);

			mbim->_state = Mbim::PIN;
			mbim->_send_request();
		}

		static void _subscriber_state(MbimDevice   *dev,
		                              GAsyncResult *res, gpointer user_data)
		{
			Mbim *mbim = _mbim(user_data);
			g_autoptr(GError)      error    = nullptr;
			g_autoptr(MbimMessage) response = mbim->_command_response(res);

			if (!response) return;

			MbimSubscriberReadyState ready_state;

			if (!mbim_message_subscriber_ready_status_response_parse(response,
			                                                         &ready_state,
			                                                         nullptr,
			                                                         nullptr,
			                                                         nullptr,
			                                                         nullptr,
			                                                         nullptr,
			                                                         &error)) {
				Genode::error("couldn't parse response message: ", (char const *)error->message);
				mbim->_shutdown (FALSE);
				return;
			}

			mbim->_state_report.sim = mbim_subscriber_ready_state_get_string(ready_state);
			mbim->_report_state();

			if (ready_state == MBIM_SUBSCRIBER_READY_STATE_DEVICE_LOCKED) {
				mbim->_state = Mbim::UNLOCK;
				mbim->_send_request();
				return;
			}

			if (ready_state != MBIM_SUBSCRIBER_READY_STATE_INITIALIZED) {
				Genode::error("subscriber not initialized: ",
				              mbim_subscriber_ready_state_get_string(ready_state));
				mbim->_shutdown (FALSE);
				return;
			}

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

			/* store info for state report */
			mbim->_state_report.error      = mbim_nw_error_get_string(nw_error);
			mbim->_state_report.network    = mbim_register_state_get_string(register_state);
			mbim->_state_report.provider   = Cstring(provider_name);
			mbim->_state_report.data_class = Cstring(mbim_data_class_build_string_from_mask(available_data_classes));
			mbim->_state_report.roaming    = Cstring(roaming_text);
			mbim->_report_state();

			if (register_state == MBIM_REGISTER_STATE_HOME ||
			    register_state == MBIM_REGISTER_STATE_ROAMING ||
			    register_state == MBIM_REGISTER_STATE_PARTNER) {
				/* check our state to allow polling registered state periodically 
 				 * even if we're already connected */
				if (mbim->_state == Mbim::PIN) {
					mbim->_state = Mbim::QUERY;
					mbim->_retry = 0;
					mbim->_send_request();
					return;
				}
			}
			else {
				/* reset state and wait+retry until registered */
				mbim->_state = Mbim::PIN;
			}

			if ((++mbim->_retry) % 10 == 0)
				Genode::warning("Device not registered after ", mbim->_retry, " tries");

			/*
			 * We delay request retries to leave device time for network
			 * registration. The delay is based on exponential backoff with
			 * upper bound.
			 */
			guint const delay = mbim->_backoff < Mbim::BACKOFF_LIMIT
			                  ? mbim->_backoff *= 2
			                  : Mbim::BACKOFF_LIMIT;
			g_timeout_add(delay, _handle_timeout, mbim);
		}

		static gboolean _handle_timeout(gpointer user_data)
		{
			Mbim *mbim = _mbim(user_data);
			mbim->_send_request();

			/* discard timer */
			return FALSE;
		}

		static void _packet_service_ready(MbimDevice   *dev,
		                                  GAsyncResult *res, gpointer user_data)
		{
			Mbim *mbim = _mbim(user_data);
			GError *error                   = nullptr;
			g_autoptr(MbimMessage) response = mbim->_command_response(res);

			/* clear backoff upon successful connection */
			mbim->_backoff = Mbim::BACKOFF_START;

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

			mbim->_connection.ip        = address;
			mbim->_connection.mask      = ipv4address[0]->on_link_prefix_length;
			mbim->_connection.gateway   = gateway;
			mbim->_connection.dns[0]    = dns[0];
			mbim->_connection.dns[1]    = dns[1];
			mbim->_connection.connected = true;
			mbim->_state = Mbim::READY;
			mbim->_report_config();
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

			/* register handler for status messages */
			g_signal_connect(mbim->_device,
			                 MBIM_DEVICE_SIGNAL_INDICATE_STATUS,
			                 G_CALLBACK(_handle_indicate_status),
			                 mbim);

			/* register handler for hangup messages */
			g_signal_connect(mbim->_device,
			                 MBIM_DEVICE_SIGNAL_REMOVED,
			                 G_CALLBACK(_handle_hangup),
			                 mbim);

			if (!mbim_device_is_open(mbim->_device)) {
				if (TRACE)
					Genode::log("opening device");
				mbim_device_open_full(mbim->_device,
				                      open_flags,
				                      45,
				                      nullptr,
				                      (GAsyncReadyCallback) _device_open_ready,
				                      mbim);
			}
			else {
				mbim->_retry = 0;
				mbim->_send_request();
			}
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

		static void _handle_indicate_status(MbimDevice* dev,
		                             MbimMessage* msg,
		                             gpointer user_data)
		{
			Mbim *mbim = _mbim(user_data);

			GError     *error   = nullptr;
			MbimService service = mbim_message_indicate_status_get_service(msg);
			guint32     cid     = mbim_message_indicate_status_get_cid(msg);

			if (service == MBIM_SERVICE_BASIC_CONNECT) {
				switch(cid) {

					case MBIM_CID_BASIC_CONNECT_SIGNAL_STATE:
						if (!mbim_message_signal_state_notification_parse(msg,
						                                                  &mbim->_state_report.rssi,
						                                                  &mbim->_state_report.error_rate,
						                                                  nullptr,
						                                                  nullptr,
						                                                  nullptr,
						                                                  &error)) {
							Genode::error("couldn't parse signal state notification message: ",
							              (char const *)error->message);
							return;
						}

						/* handle RSSI connection-state change */
						if (mbim->_state_report.rssi > RSSI_DISCONNECT) {
							if (mbim->_connection.connected) {
								mbim->_connection.connected = false;
								mbim->_report_config();
								mbim->_state = NONE;
							}
						} else {
							if (!mbim->_connection.connected) {
								mbim->_connection.connected = true;
								mbim->_report_config();
								mbim->_send_request();
							}
						}

						mbim->_report_state();
						break;

					case MBIM_CID_BASIC_CONNECT_REGISTER_STATE:
					{
						MbimNwError          nw_error;
						MbimRegisterState    register_state;
						MbimDataClass        available_data_classes;
						g_autofree gchar    *provider_name = nullptr;
						g_autofree gchar    *roaming_text = nullptr;

						if (!mbim_message_register_state_notification_parse(msg,
						                                                    &nw_error,
						                                                    &register_state,
						                                                    nullptr,
						                                                    &available_data_classes,
						                                                    nullptr,
						                                                    nullptr,
						                                                    &provider_name,
						                                                    &roaming_text,
						                                                    nullptr,
						                                                    &error)) {
							Genode::error("couldn't parse register state notification message: ",
							              (char const *)error->message);
							return;
						}

						/* store info for state report */
						mbim->_state_report.error      = mbim_nw_error_get_string(nw_error);
						mbim->_state_report.network    = mbim_register_state_get_string(register_state);
						mbim->_state_report.provider   = Cstring(provider_name);
						mbim->_state_report.data_class = Cstring(mbim_data_class_build_string_from_mask(available_data_classes));
						mbim->_state_report.roaming    = Cstring(roaming_text);
						mbim->_report_state();

						if (register_state != MBIM_REGISTER_STATE_HOME &&
						    register_state != MBIM_REGISTER_STATE_ROAMING &&
						    register_state != MBIM_REGISTER_STATE_PARTNER) {
							Genode::warning("Lost network registration");
						}

						break;
					}
					case MBIM_CID_BASIC_CONNECT_PACKET_SERVICE:
						/* ignore */
						break;
					case MBIM_CID_BASIC_CONNECT_SUBSCRIBER_READY_STATUS:
					{
						MbimSubscriberReadyState ready_state;

						if (!mbim_message_subscriber_ready_status_notification_parse(msg,
						                                                             &ready_state,
						                                                             nullptr,
						                                                             nullptr,
						                                                             nullptr,
						                                                             nullptr,
						                                                             nullptr,
						                                                             &error)) {
							Genode::error("couldn't parse notification message: ", (char const *)error->message);
							mbim->_shutdown (FALSE);
							return;
						}

						mbim->_state_report.sim = mbim_subscriber_ready_state_get_string(ready_state);
						mbim->_report_state();

						if (ready_state == MBIM_SUBSCRIBER_READY_STATE_DEVICE_LOCKED) {
							/* unlock with PIN */
							mbim->_state = Mbim::UNLOCK;
							mbim->_send_request();
						}
						else if (ready_state == MBIM_SUBSCRIBER_READY_STATE_INITIALIZED) {
							if (mbim->_state == Mbim::NONE) {
								/* jump ahead and wait for network registration */
								mbim->_state = Mbim::PIN;
								mbim->_send_request();
							}
						}
						else {
							/* reset */
							mbim->_state = Mbim::NONE;
							mbim->_send_request();
						}

						break;
					}
					default:
						const gchar *cid_printable = mbim_cid_get_printable(mbim_message_indicate_status_get_service (msg),
						                                                    mbim_message_indicate_status_get_cid (msg));
						Genode::warning("Received unknown status message with cid: ", cid_printable);
						Genode::warning(Genode::Cstring(mbim_message_get_printable(msg, "  ", FALSE)));
				}
			}
		}

		static void _handle_hangup(MbimDevice* dev,
		                    gpointer user_data)
		{
			Mbim *mbim = _mbim(user_data);

			Genode::warning("Device hung-up. Reconnecting...");
			mbim->_state = Mbim::PIN;
			mbim->_send_request();
		}

		void _connect()
		{
			g_main_loop_run(_loop);
			g_object_unref(_device);
			g_main_loop_unref(_loop);
		}

		void _report_state()
		{
			_state_reporter.enabled(true);
			try {
				Genode::Reporter::Xml_generator xml(_state_reporter, [&]() {
					xml.node("device", [&] () {
						xml.attribute("sim", _state_report.sim);
					});

					xml.node("network", [&] () {
						xml.attribute("error",      _state_report.error);
						xml.attribute("registered", _state_report.network);
						xml.attribute("provider",   _state_report.provider);
						xml.attribute("data_class", _state_report.data_class);
						xml.attribute("roaming",    _state_report.roaming);
					});

					xml.node("signal", [&] () {
						if (_state_report.rssi > RSSI_DISCONNECT)
							xml.attribute("rssi_dbm", "unknown");
						else
							xml.attribute("rssi_dbm", String("-", 113-2*_state_report.rssi));

						xml.attribute("error_rate", _state_report.error_rate);
						
					});
				});
			}
			catch (...) { Genode::warning("Could not report state."); }
		}

		void _report_config()
		{
			if (_state != Mbim::READY)
				return;

			/* handle intermediate disconnect */
			if (!_connection.connected) {
				_config_reporter.enabled(true);
				Genode::Reporter::Xml_generator xml(_config_reporter, [&]() {
					xml.attribute("verbose", "no");
					xml.attribute("verbose_packets", "no");
					xml.attribute("verbose_domain_state", "yes");
				});
				return;
			}

			String interface = "10.0.1.1/24";
			String ip_first  = "10.0.1.2";
			String ip_last   = "10.0.1.200";

			try {
				Genode::Xml_node const &net = _config_rom.xml().sub_node("default-domain");

				interface = net.attribute_value("interface", interface);
				ip_first  = net.attribute_value("ip_first",  ip_first);
				ip_last   = net.attribute_value("ip_first",  ip_last);

			} catch (...) { }

			_config_reporter.enabled(true);
			try {
				Genode::Reporter::Xml_generator xml(_config_reporter, [&]() {
				xml.attribute("verbose", "no");
				xml.attribute("verbose_packets", "no");
				xml.attribute("verbose_domain_state", "yes");

					xml.node("default-policy", [&] () {
						xml.attribute("domain", "default");
					});

					xml.node("policy", [&] () {
						xml.attribute("label_prefix", "usb_modem_drv");
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
							xml.attribute("tcp-ports", "1000");
							xml.attribute("udp-ports", "1000");
							xml.attribute("icmp-ids", "1000");
						});
						if (_config_rom.xml().attribute_value("nic_client_enable", false)) {
							xml.node("nat", [&] () {
								xml.attribute("domain", "downlink");
								xml.attribute("tcp-ports", "1000");
								xml.attribute("udp-ports", "1000");
								xml.attribute("icmp-ids", "1000");
							});
						}
					});

					/* link to another nic_router */
					if (_config_rom.xml().attribute_value("nic_client_enable", false)) {
						xml.node("nic-client", [&] () {
							xml.attribute("domain", "downlink");
						});
						xml.node("domain", [&] () {
							xml.attribute("name", "downlink");

							xml.attribute("interface", "10.0.2.1/24");

							xml.node("dhcp-server", [&] () {
								xml.attribute("ip_first", "10.0.2.2");
								xml.attribute("ip_last",  "10.0.2.3");

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
					}

					/* default */
					xml.node("domain", [&] () {
						xml.attribute("name", "default");

						xml.attribute("interface", interface);

						xml.node("dhcp-server", [&] () {
							xml.attribute("ip_first", ip_first);
							xml.attribute("ip_last",  ip_last);

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
