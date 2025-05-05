/*
 * \brief   Avplay slave
 * \author  Christian Prochaska
 * \date    2012-04-05
 */

/*
 * Copyright (C) 2012-2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _AVPLAY_SLAVE_H_
#define _AVPLAY_SLAVE_H_

/* Qt includes */
#include <QDebug>
#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>

/* Genode includes */
#include <input/component.h>
#include <timer_session/timer_session.h>
#include <audio_out_session/audio_out_session.h>
#include <os/static_parent_services.h>
#include <slave.h>

/* local includes */
#include "gui_session_component.h"

typedef Genode::Local_service<Gui::Session_component> Gui_service;

class Avplay_slave : public QObject
{
	Q_OBJECT

	private:

		struct Policy_base
		{
			Genode::Static_parent_services<Genode::Cpu_session,
			                               Genode::Log_session,
			                               Genode::Pd_session,
			                               Genode::Rom_session,
			                               Timer::Session,
			                               Audio_out::Session>
				_parent_services;

			Policy_base(Genode::Env &env) : _parent_services(env) { }
		};

		class Policy : Policy_base, public Genode::Slave::Policy
		{
			private:

				Gui_service &_gui_service;

				const char *_mediafile;
				int _sdl_audio_volume;
				QByteArray _config_byte_array;


				const char *_config()
				{
					QDomDocument config_doc;

					QDomElement config_node = config_doc.createElement("config");
					config_doc.appendChild(config_node);

					QDomElement arg0_node = config_doc.createElement("arg");
					arg0_node.setAttribute("value", "avplay");
					config_node.appendChild(arg0_node);

					QDomElement arg1_node = config_doc.createElement("arg");
					arg1_node.setAttribute("value", _mediafile);
					config_node.appendChild(arg1_node);

					/*
			 	 	 * Configure libc of avplay to direct output to LOG and to obtain
			 	 	 * the mediafile from ROM.
			 	 	 */

					QDomElement vfs_node = config_doc.createElement("vfs");
					QDomElement vfs_dev_node = config_doc.createElement("dir");
					vfs_dev_node.setAttribute("name", "dev");
					QDomElement vfs_dev_log_node = config_doc.createElement("log");
					vfs_dev_node.appendChild(vfs_dev_log_node);
					QDomElement vfs_dev_rtc_node = config_doc.createElement("inline");
					vfs_dev_rtc_node.setAttribute("name", "rtc");
					QDomText vfs_dev_rtc_node_text = config_doc.createTextNode("2018-01-01 00:01");
					vfs_dev_rtc_node.appendChild(vfs_dev_rtc_node_text);
					vfs_dev_node.appendChild(vfs_dev_rtc_node);
					vfs_node.appendChild(vfs_dev_node);
					QDomElement vfs_mediafile_node = config_doc.createElement("rom");
					vfs_mediafile_node.setAttribute("name", "mediafile");
					vfs_node.appendChild(vfs_mediafile_node);
					config_node.appendChild(vfs_node);

					QDomElement libc_node = config_doc.createElement("libc");
					libc_node.setAttribute("stdout", "/dev/log");
					libc_node.setAttribute("stderr", "/dev/log");
					libc_node.setAttribute("rtc", "/dev/rtc");
					config_node.appendChild(libc_node);

					QDomElement sdl_audio_volume_node = config_doc.createElement("sdl_audio_volume");
					sdl_audio_volume_node.setAttribute("value", QString::number(_sdl_audio_volume));
					config_node.appendChild(sdl_audio_volume_node);

					_config_byte_array = config_doc.toByteArray(4);

					return _config_byte_array.constData();
				}

				static Genode::Cap_quota _caps()      { return { 150 }; }
				static Genode::Ram_quota _ram_quota() { return { 64*1024*1024 }; }
				static Name              _name()      { return "avplay"; }

				void _with_matching_service(Genode::Service::Name const &name,
				                            auto const &fn, auto const &denied_fn)
				{
					if (name == "Gui") {
						fn(_gui_service);
						return;
					}
					denied_fn();
				}

			public:

				Policy(Genode::Env        &env,
				       Gui_service        &gui_service,
				       char const         *mediafile)
				:
					Policy_base(env),
					Genode::Slave::Policy(env, _name(), _name(),
					                      Policy_base::_parent_services,
					                      env.ep().rpc_ep(), _caps(),
					                      _ram_quota()),
					_gui_service(gui_service),
					_mediafile(mediafile),
					_sdl_audio_volume(100)
				{
					configure(_config());
				}

				void _with_route(Genode::Service::Name     const &name,
				                 Genode::Session_label     const &label,
				                 Genode::Session::Diag     const  diag,
				                 With_route::Ft    const &fn,
				                 With_no_route::Ft const &denied_fn) override
				{
					using namespace Genode;

					_with_matching_service(name,
						[&] (Service &service) { fn(Route { .service = service,
						                                    .label   = label,
						                                    .diag    = diag }); },
						[&] /* denied */ {
							Slave::Policy::_with_route(name, label, diag,
							                           fn, denied_fn); }
					);
				}

				void volume_changed(int value)
				{
					_sdl_audio_volume = value;
					configure(_config());
				}

		};

		Policy              _policy;
		Genode::Child       _child;

	public:

		/**
		 * Constructor
		 */
		Avplay_slave(Genode::Env        &env,
		             Gui_service        &gui_service,
		             char const         *mediafile)
		:
			_policy(env, gui_service, mediafile),
			_child(env.rm(), env.ep().rpc_ep(), _policy)
		{ }

	public Q_SLOTS:

		void volume_changed(int value)
		{
			_policy.volume_changed(value);
		}
};

#endif /* _AVPLAY_SLAVE_H_ */
