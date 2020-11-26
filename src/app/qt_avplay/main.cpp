/*
 * \brief   Simple Qt interface for 'avplay' media player
 * \author  Christian Prochaska
 * \date    2012-03-21
 */

/*
 * Copyright (C) 2012-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Qt includes */
#include <QApplication>

/* qt5_component includes */
#include <qt5_component/qpa_init.h>

/* qt_avplay includes */
#include "main_window.h"

/* Genode includes */
#include <libc/component.h>


static inline void load_stylesheet()
{
        QFile file(":style.qss");
        if (!file.open(QFile::ReadOnly)) {
                qWarning() << "Warning:" << file.errorString()
                           << "opening file" << file.fileName();
                return;
        }

        qApp->setStyleSheet(QLatin1String(file.readAll()));
}

extern void initialize_qt_gui(Genode::Env &);

/*
 * The Qt main code is executed in a pthread to keep the entrypoint
 * ready to handle RPC from 'avplay'
 */
void *main_func(void *arg)
{
	Libc::with_libc([&] {

		Libc::Env *env = reinterpret_cast<Libc::Env*>(arg);

		qpa_init(*env);

		int argc = 1;
		char const *argv[] = { "qt_avplay", 0 };

		QApplication app(argc, (char**)argv);

		load_stylesheet();

		QMember<Main_window> main_window(*env);

		main_window->show();

		app.exec();
	});

	return nullptr;
}

void Libc::Component::construct(Libc::Env &env)
{
	Libc::with_libc([&] {
		pthread_t main_thread;
		pthread_create(&main_thread, nullptr, main_func, &env);
	});
}
