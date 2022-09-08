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
#include <qgenodeviewwidget/qgenodeviewwidget.h>

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
#include "gui_session_component.h"


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
				config.with_optional_sub_node(node_type, [&] (Xml_node const &mediafile) {
					result = mediafile.attribute_value("name", result); });

				return result;
			}

			Mediafile_name(Genode::Env &env) : name(_name_from_config(env)) { }
		};

		Genode::Env                &_env;

		Mediafile_name              _mediafile_name;

		QGenodeViewWidget          *_avplay_widget { _create_avplay_widget() };
		QMember<Control_bar>        _control_bar;

		Gui::Session_component _gui_session_component { _env, _avplay_widget };

		Gui_service::Single_session_factory _gui_factory { _gui_session_component };

		Gui_service _gui_service { _gui_factory };

		QGenodeViewWidget *_create_avplay_widget();

	public:

		Main_window(Genode::Env &env);
};

#endif /* _MAIN_WINDOW_H_ */
