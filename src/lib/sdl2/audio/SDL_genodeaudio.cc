/*
 * \brief  Genode-specific audio backend
 * \author Alexander Boettcher 
 * \date   2020-05-20
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

extern "C" {

#include "SDL_internal.h"
#include "SDL_audio.h"
#include "SDL_audio_c.h"
#include "SDL_audiodev_c.h"

static int GenodeAudio_Init(SDL_AudioDriverImpl *)
{
	printf("Audio not supported\n");
	return 0;
}

AudioBootStrap GenodeAudio_bootstrap = {
	"genode", "Genode audio driver", GenodeAudio_Init, 0
};

} /* extern "C" */

