/*
 * \brief  Interface for waking up server event loop
 * \author Tomasz Gajewski
 * \date   2021-06-06
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 * Copyright (C) 2021 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SSH_WAKE_UP_SIGNALLER_H_
#define _SSH_WAKE_UP_SIGNALLER_H_


namespace Ssh {
	struct Wake_up_signaller;
}

struct Ssh::Wake_up_signaller
{
	virtual void signal_wake_up() = 0;
};

#endif /* _SSH_WAKE_UP_SIGNALLER_H_ */
