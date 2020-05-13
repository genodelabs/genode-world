/*
 * \brief   Main window of the media player
 * \author  Christian Prochaska
 * \date    2012-03-29
 */

/*
 * Copyright (C) 2012-2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

/* Qt includes */
#include <QVBoxLayout>
#include <QWidget>
#include <qnitpickerviewwidget/qnitpickerviewwidget.h>

/* Qoost includes */
#include <qoost/compound_widget.h>
#include <qoost/qmember.h>

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/service.h>
#include <input/root.h>
#include <rom_session/connection.h>

/* local includes */
#include "avplay_slave.h"
#include "control_bar.h"
#include "nitpicker_session_component.h"


class Main_window : public Compound_widget<QWidget, QVBoxLayout>
{
	Q_OBJECT

	private:

		struct Mediafile_name
		{
			typedef Genode::String<256> Name;
			Name const name;

			Name _name_from_config(Genode::Env &env)
			{
				using namespace Genode;

				Attached_rom_dataspace config_ds(env, "config");
				Xml_node const config = config_ds.xml();
				char const * const node_type = "mediafile";

				if (!config.has_sub_node(node_type))
					warning("no <", node_type, " name=\"...\"> config node found, "
					        "using \"mediafile\"");

				Name result { "mediafile" };
				config.with_sub_node(node_type, [&] (Xml_node const &mediafile) {
					result = mediafile.attribute_value("name", result); });

				return result;
			}

			Mediafile_name(Genode::Env &env) : name(_name_from_config(env)) { }
		};

		Genode::Env                   &_env;

		Genode::size_t const           _ep_stack_size { 16 * 1024 };
		Genode::Entrypoint             _ep { _env, _ep_stack_size,
		                                     "avplay_ep",
		                                     Genode::Affinity::Location() };

		Mediafile_name                 _mediafile_name;

		QMember<QNitpickerViewWidget>  _avplay_widget;
		QMember<Control_bar>           _control_bar;

		Nitpicker::Session_component   _nitpicker_session_component {
			_env, _ep, *_avplay_widget };

		Nitpicker_service::Single_session_factory _nitpicker_factory {
			_nitpicker_session_component };

		Nitpicker_service              _nitpicker_service { _nitpicker_factory };

	public:

		Main_window(Genode::Env &env);
};

#endif /* _MAIN_WINDOW_H_ */
