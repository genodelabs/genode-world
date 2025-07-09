/*
 * \brief  helper for setting the time
 * \author Roland Bär
 * \date   2022-06-06
 */

#ifndef _SET_TIME_HELPER_H_
#define _SET_TIME_HELPER_H_

#ifdef __cplusplus

#include <rtc.h>
#include <rtc_session/rtc_session.h>
#include <os/reporter.h>
#include <util/reconstructible.h>

class set_time_helper
{
private:
	Genode::Constructible<Genode::Expanding_reporter> _set_time_reporter { };

	void _set_rtc(Genode::Expanding_reporter &reporter, Rtc::Timestamp const &ts)
	{
		Genode::log("_set_rtc");
		reporter.generate([&] (Genode::Generator &g) {
			g.attribute("year",   ts.year);
			g.attribute("month",  ts.month);
			g.attribute("day",    ts.day);
			g.attribute("hour",   ts.hour);
			g.attribute("minute", ts.minute);
			g.attribute("second", ts.second);
		});
	}

public:
	set_time_helper();
	~set_time_helper();
	void init(Libc::Env &env);
	void set_time(time_t &time);
};

#else
	typedef
		struct set_time_helper
			set_time_helper;
#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif
	extern set_time_helper _set_time_helper;
#if defined(__STDC__) || defined(__cplusplus)
	extern void set_time_via_helper(set_time_helper*, time_t);   /* ANSI C prototypes */
	extern set_time_helper* set_time_callback_function(set_time_helper*, time_t);
#else
	extern void set_time_via_helper(time_t);        /* K&R style */
	extern set_time_helper* set_time_callback_function(time_t);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SET_TIME_HELPER_H_
