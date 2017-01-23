/*
 * \brief  Dummy implementations
 * \author Reinier Millo Sánchez
 * \author Alexy Gallardo Segura
 * \date   2015-11-15
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <signal.h>

int sigpending(sigset_t *set)
{
	//*sig = SIGQUIT;
	return 0;
}


int sigwait(const sigset_t *set, int *sig)
{
	*sig = SIGQUIT;
	return 0;
}


