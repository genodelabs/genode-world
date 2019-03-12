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


Main_window::Main_window(Genode::Env &env)
:
	_env(env),
	_mediafile_name(env),
	_control_bar(_nitpicker_session_component.input_component())
{
	/* add widgets to layout */

	_layout->addWidget(_avplay_widget);
	_layout->addWidget(_control_bar);

	/*
	 * The main window must be visible before avplay requests the Nitpicker
	 * session, because the parent view of the new Nitpicker view is part of
	 * the QNitpickerPlatformWindow object, which is created when the main
	 * window becomes visible.
	 */

	show();

	/* start avplay */

	Avplay_slave *avplay_slave = new Avplay_slave(_env, _ep,
	                                              _nitpicker_service,
	                                              _mediafile_name.buf);

	connect(_control_bar, SIGNAL(volume_changed(int)), avplay_slave, SLOT(volume_changed(int)));
}
