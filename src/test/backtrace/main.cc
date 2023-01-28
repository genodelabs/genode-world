/*
 * \brief  libbacktrace test
 * \author Christian Prochaska
 * \date   2023-01-28
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <backtrace.h>

int main(int argc, char **argv)
{
	struct backtrace_state *state =
		backtrace_create_state(NULL, 0, NULL, NULL);

	backtrace_print(state, 0, stdout);

	return 0;
}
