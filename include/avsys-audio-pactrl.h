/*
 * avsystem
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Jonghyuk Choi <jhchoi.choi@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __AVSYS_AUDIO_PACTRL_H__
#define __AVSYS_AUDIO_PACTRL_H__

#ifdef __cplusplus
	extern "C" {
#endif
#include <avsys-audio-handle.h>
#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>
#include <pulse/proplist.h>

/*
 * Enums
 */
enum {
	AVSYS_AUDIO_PA_CTL_SINK_UNKNOWN	= 0x0000,
	AVSYS_AUDIO_PA_CTL_SINK_ALSA	= 0x0001,
	AVSYS_AUDIO_PA_CTL_SINK_BLUEZ	= 0x0002,
};
/*
 * Defines
 */
#define ALSA_SINK			0
#define BLUEZ_SINK			16

#define _EXIST				1
#define _DEACTIVE			2
#define _ACTIVE				3

#define ALSA_SINK_EXIST		((1 << (_EXIST + ALSA_SINK)))
#define ALSA_SINK_DEACTIVE	((1 << (_DEACTIVE + ALSA_SINK)))
#define ALSA_SINK_ACTIVE	((1 << (_ACTIVE + ALSA_SINK)))

#define BULEZ_SINK_EXIST	((1 << (_EXIST + BLUEZ_SINK)))
#define BULEZ_SINK_DEACTIVE	((1 << (_DEACTIVE + BLUEZ_SINK)))
#define BULEZ_SINK_ACTIVE	((1 << (_ACTIVE + BLUEZ_SINK)))

/*
 * Function Prorotypes
 */
int avsys_audio_pa_ctrl_volume_by_index(unsigned int idx, int volume, int ch);
int avsys_audio_pa_ctrl_mute_by_index(unsigned int idx, int mute);
int avsys_audio_pa_ctrl_get_default_sink(int *sink);

#ifdef __cplusplus
	}
#endif

#endif /* __AVSYS_AUDIO_PACTRL_H__ */
