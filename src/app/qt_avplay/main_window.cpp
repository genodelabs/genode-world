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

/* qt_avplay includes */
#include "main_window.h"


using namespace Genode;


QGenodeViewWidget *Main_window::_create_avplay_widget()
{
	QPluginLoader plugin_loader("/qt/plugins/qgenodeviewwidget/libqgenodeviewwidget.lib.so");

	QObject *plugin = plugin_loader.instance();

	if (!plugin)
		qFatal("Error: Could not load QGenodeViewWidget Qt plugin");

	QGenodeViewWidgetInterface *genode_view_widget_interface =
		qobject_cast<QGenodeViewWidgetInterface*>(plugin);

	QGenodeViewWidget *avplay_widget = static_cast<QGenodeViewWidget*>(
		genode_view_widget_interface->createWidget()
	);

	return avplay_widget;
}


Main_window::Main_window(Genode::Env &env)
:
	_env(env),
	_mediafile_name(env),
	_control_bar(_gui_session_component.input_component())
{
	/* add widgets to layout */

	_layout->addWidget(_avplay_widget);
	_layout->addWidget(_control_bar);

	/*
	 * The main window must be visible before avplay requests the GUI
	 * session, because the parent view of the new GUI view is part of
	 * the QGenodePlatformWindow object, which is created when the main
	 * window becomes visible.
	 */

	show();

	/* start avplay */

	Avplay_slave *avplay_slave = new Avplay_slave(_env,
	                                              _gui_service,
	                                              _mediafile_name.name.string());

	connect(_control_bar, SIGNAL(volume_changed(int)), avplay_slave, SLOT(volume_changed(int)));
}
