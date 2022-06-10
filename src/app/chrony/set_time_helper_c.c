/*
 * \brief  C helper implementation for set time
 * \author Roland Bär
 * \date   2022-06-06
 */

#include <time.h>
#include <stdio.h>
#include "set_time_helper.h"


void set_time_via_helper(set_time_helper* set_time_helper, time_t time)
{
	set_time_callback_function(set_time_helper, time);
}