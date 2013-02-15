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

#include <alsa/asoundlib.h>

#include "avsys-audio-alsa.h"
#include "avsys-types.h"
#include "avsys-error.h"
#include "avsys-debug.h"

#if defined(_MMFW_I386_ALL_SIMULATOR)
#define AIF_CP_DEVICE_NAME "default"
#define AIF_BT_DEVICE_NAME "default"
#define AIF_RADIO_DEVICE_NAME "default"
#else
#define AIF_CP_DEVICE_NAME "AIF2"
#define AIF_BT_DEVICE_NAME "AIF3"
#define AIF_RADIO_DEVICE_NAME "AIF4"
#endif

int avsys_audio_alsa_open_AIF_device(aif_device_type_t aif_type, avsys_audio_alsa_aif_handle_t *handle)
{
	snd_pcm_t *ahandle = NULL;
	int err = -1;
	char dev_name[16] = { 0, };
	snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;

	avsys_info(AVAUDIO, "%s\n", __func__);
	if (!handle)
		return AVSYS_STATE_ERR_NULL_POINTER;

	memset(dev_name, '\0', sizeof(dev_name));
	switch (aif_type) {
	case AIF_CP_CAPTURE:
		strncpy(dev_name, AIF_CP_DEVICE_NAME, sizeof(dev_name) - 1);
		stream = SND_PCM_STREAM_CAPTURE;
		break;
	case AIF_CP_PLAYBACK:
		strncpy(dev_name, AIF_CP_DEVICE_NAME, sizeof(dev_name) - 1);
		stream = SND_PCM_STREAM_PLAYBACK;
		break;
	case AIF_BT_CAPTURE:
		strncpy(dev_name, AIF_BT_DEVICE_NAME, sizeof(dev_name) - 1);
		stream = SND_PCM_STREAM_CAPTURE;
		break;
	case AIF_BT_PLAYBACK:
		strncpy(dev_name, AIF_BT_DEVICE_NAME, sizeof(dev_name) - 1);
		stream = SND_PCM_STREAM_PLAYBACK;
		break;
	case AIF_RADIO_PLAYBACK:
		strncpy(dev_name, AIF_RADIO_DEVICE_NAME, sizeof(dev_name) - 1);
		stream = SND_PCM_STREAM_PLAYBACK;
		break;
	default:
		avsys_critical_r(AVAUDIO, "Invalid AIF device %d\n", aif_type);
		return AVSYS_STATE_ERR_INVALID_MODE;
		break;
	}
#if !defined(_MMFW_I386_ALL_SIMULATOR)
	err = snd_pcm_open(&ahandle, dev_name, stream, 0);
#else
	avsys_warning_r(AVAUDIO, "Skip real device open in SDK\n");
	err = 0;		/* set fake return value */
#endif

	if (err < 0) {
		avsys_error_r(AVAUDIO, "unable to open AIF device: %s\n", snd_strerror(err));
		return AVSYS_STATE_ERR_INTERNAL;
	}

	handle->alsa_handle = (void *)ahandle;
	handle->type = aif_type;

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_alsa_close_AIF_device(avsys_audio_alsa_aif_handle_t* handle)
{
	int err = 0;
	snd_pcm_t *ahandle = NULL;

	avsys_info(AVAUDIO, "%s\n", __func__);
#if defined(_MMFW_I386_ALL_SIMULATOR)
	avsys_warning(AVAUDIO, "Skip close call device in SDK");
	return AVSYS_STATE_SUCCESS;
#endif
	if (!handle)
		return AVSYS_STATE_ERR_NULL_POINTER;

	ahandle = (snd_pcm_t *)handle->alsa_handle;
	if (!ahandle) {
		avsys_error_r(AVAUDIO, "[%s] alsa handle is null\n", __func__);
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	err = snd_pcm_close(ahandle);
	if (err < 0) {
		avsys_critical_r(AVAUDIO, "unable to close pcm device: %s\n", snd_strerror(err));
		return AVSYS_STATE_ERR_INTERNAL;
	}

	handle->alsa_handle = NULL;

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_alsa_set_AIF_params(avsys_audio_alsa_aif_handle_t *handle, aif_rate_t rate)
{
#if defined(_MMFW_I386_ALL_SIMULATOR)
	return AVSYS_STATE_SUCCESS;
#else
	int err = 0;
	snd_pcm_t *ahandle = NULL;
	snd_pcm_hw_params_t *params;
	unsigned int val = 0;
	int dir = 0;

	avsys_info(AVAUDIO, "%s\n", __func__);
	if (!handle)
		return AVSYS_STATE_ERR_NULL_POINTER;

	ahandle = (snd_pcm_t *)handle->alsa_handle;
	if (!ahandle)
		return AVSYS_STATE_ERR_NULL_POINTER;

	/* Skip parameter setting to null device. */
	if (snd_pcm_type(ahandle) == SND_PCM_TYPE_NULL)
		return AVSYS_STATE_SUCCESS;

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);
	/* Fill it in with default values. */
	err = snd_pcm_hw_params_any(ahandle, params);
	if (err < 0) {
		avsys_error_r(AVAUDIO, "snd_pcm_hw_params_any() : failed! - %s\n", snd_strerror(err));
		return AVSYS_STATE_ERR_INTERNAL;
	}

	/* Set the desired hardware parameters. */
	/* Interleaved mode */
	err = snd_pcm_hw_params_set_access(ahandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		avsys_error_r(AVAUDIO, "snd_pcm_hw_params_set_access() : failed! - %s\n", snd_strerror(err));
		return AVSYS_STATE_ERR_INTERNAL;
	}

	if (rate == AIF_CONF_RATE) {
		/* Auto rate by conf */
		avsys_info(AVAUDIO, "Let rate by configuration\n");
	} else {
		/* Force input rate */
		avsys_info(AVAUDIO, "Force rate (%d)\n", rate);
		err = snd_pcm_hw_params_set_rate(ahandle, params, rate, 0);
		if (err < 0) {
			avsys_error_r(AVAUDIO, "snd_pcm_hw_params_set_rate() : failed! - %s\n", snd_strerror(err));
			return AVSYS_STATE_ERR_INTERNAL;
		}
	}

	err = snd_pcm_hw_params(ahandle, params);
	if (err < 0) {
		avsys_error_r(AVAUDIO, "snd_pcm_hw_params() : failed! - %s\n", snd_strerror(err));
		return AVSYS_STATE_ERR_INTERNAL;
	}

	/* Dump current param */
	snd_pcm_hw_params_get_access(params, (snd_pcm_access_t *) &val);
	avsys_info(AVAUDIO, "access type = %s\n", snd_pcm_access_name((snd_pcm_access_t)val));

	snd_pcm_hw_params_get_format(params, &val);
	avsys_info(AVAUDIO, "format = '%s' (%s)\n",
					snd_pcm_format_name((snd_pcm_format_t)val),
					snd_pcm_format_description((snd_pcm_format_t)val));

	snd_pcm_hw_params_get_subformat(params, (snd_pcm_subformat_t *)&val);
	avsys_info(AVAUDIO, "subformat = '%s' (%s)\n",
					snd_pcm_subformat_name((snd_pcm_subformat_t)val),
					snd_pcm_subformat_description((snd_pcm_subformat_t)val));

	snd_pcm_hw_params_get_channels(params, &val);
	avsys_info(AVAUDIO, "channels = %d\n", val);

	snd_pcm_hw_params_get_rate(params, &val, &dir);
	avsys_info(AVAUDIO, "rate = %d(%d) Hz\n", val, dir);

	/* Save current rate for later check */
	handle->rate = val;

	return AVSYS_STATE_SUCCESS;
#endif /* _MMFW_I386_ALL_SIMULATOR */
}
