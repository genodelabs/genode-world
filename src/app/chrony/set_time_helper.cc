/*
 * \brief  helper implementation for set time
 * \author Roland Bär
 * \date   2022-06-06
 */

#include <base/env.h>
#include <libc/args.h>
#include <libc/component.h>
#include <rtc.h>
#include <os/reporter.h>
#include <util/reconstructible.h>
#include "set_time_helper.h"

set_time_helper::set_time_helper()
{
	Genode::log("set_time_helper::ctor");
}


set_time_helper::~set_time_helper()
{
}


void set_time_helper::init(Libc::Env &env)
{
	_set_time_reporter.construct(env, "set_rtc");
	_set_time_reporter->enabled(true);
}


void set_time_helper::set_time(time_t &time)
{
	Rtc::Timestamp _ts { };

	struct tm *utc = gmtime(&time);
	if (utc) {
		_ts.second = utc->tm_sec;
		_ts.minute = utc->tm_min;
		_ts.hour   = utc->tm_hour;
		_ts.day    = utc->tm_mday;
		_ts.month  = utc->tm_mon + 1;
		_ts.year   = utc->tm_year + 1900;
	} else {
		Genode::error("time is not in UTC!");
	}

	_set_rtc(*_set_time_reporter, _ts);
}


set_time_helper* set_time_callback_function(set_time_helper* set_time_helper, time_t time)
{
	set_time_helper->set_time(time);
	return set_time_helper;
}


/*
 * 'main' will be called by component initialization
 */
extern "C" int main(int argc, char *argv[]);


static void construct_component(Libc::Env &env)
{
	int argc    = 0;
	char **argv = nullptr;
	char **envp = nullptr;

	populate_args_and_env(env, argc, argv, envp);

	exit(main(argc, argv));
}


void Libc::Component::construct(Libc::Env &env)
{
	_set_time_helper.init(env);
	Libc::with_libc([&] () { construct_component(env); });
}


set_time_helper _set_time_helper;
