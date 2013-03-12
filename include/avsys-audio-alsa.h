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

#ifndef __AVSYS_AUDIO_ALSA_H__
#define __AVSYS_AUDIO_ALSA_H__

#ifdef __cplusplus
	extern "C" {
#endif
#include <avsys-audio-handle.h>

typedef enum {
	AIF_CP_CAPTURE,
	AIF_CP_PLAYBACK,
	AIF_BT_CAPTURE,
	AIF_BT_PLAYBACK,
	AIF_RADIO_PLAYBACK,
	AIF_DEVICE_MAX,
} aif_device_type_t;

typedef enum  {
	AIF_CONF_RATE = 0,
	AIF_NB_RATE = 8000,
	AIF_WB_RATE = 16000,
} aif_rate_t;

typedef struct {
	void *alsa_handle;
	int type;
	int rate;
} avsys_audio_alsa_aif_handle_t;

int avsys_audio_alsa_open_AIF_device(aif_device_type_t aif_type, avsys_audio_alsa_aif_handle_t *handle);
int avsys_audio_alsa_close_AIF_device(avsys_audio_alsa_aif_handle_t* handle);
int avsys_audio_alsa_set_AIF_params(avsys_audio_alsa_aif_handle_t *handle, aif_rate_t rate);

#ifdef __cplusplus
	}
#endif

#endif /* __AVSYS_AUDIO_H__ */
