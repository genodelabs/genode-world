/*
 * \brief  Suppress 'not implemented' messages
 * \author Christian Prochaska
 * \date   2012-04-01
 */

/*
 * Copyright (C) 2012-2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

typedef struct fenv fenv_t;

int feholdexcept(fenv_t *envp) { }
int feupdateenv(const fenv_t *envp) { }
