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

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

#include "avsys-types.h"
#include "avsys-error.h"
#include "avsys-debug.h"
#include "avsys-audio.h"
#include "avsys-audio-logical-volume.h"
#include "avsys-common.h"

#include "avsys-audio-path.h"
#include "avsys-audio-alsa.h"

#include "avsys-audio-pasimple.h"
#include "avsys-audio-pactrl.h"

/**
 * Internal functions definition
 */
#define FADEUP_CALC_BIAS	(1)

static int __avsys_audio_set_info(avsys_audio_handle_t *p, avsys_audio_param_t *param);

void __init_module(void);
void __fini_module(void);

#define AVSYS_GET_HANDLE_PTR(MODE) do {	\
	err = avsys_audio_handle_get_ptr((int)handle, &p, MODE);	\
	if (AVSYS_FAIL(err)) {	\
		return err;	\
	}	\
} while (0)

#define AVSYS_RELEASE_HANDLE_PTR(MODE) do {	\
	if (AVSYS_FAIL(avsys_audio_handle_release_ptr((int)handle, MODE))) {	\
		avsys_error(AVAUDIO, "audio handle release failed\n");	\
		return AVSYS_STATE_ERR_INTERNAL;	\
	}	\
} while (0)

#define AVSYS_STREAM_LOCK() do {	\
	pthread_mutex_lock(&gmutex);\
	avsys_info(AVAUDIO, "(+) LOCKED\n");	\
} while (0)

#define AVSYS_STREAM_UNLOCK() do {	\
	pthread_mutex_unlock(&gmutex);\
	avsys_info(AVAUDIO, "(-) UNLOCKED\n");	\
} while (0)

/**
 * Internal global variable
 */
static pthread_mutex_t gmutex = PTHREAD_MUTEX_INITIALIZER;


/****************************************************************************
 *
 *  Interface
 *
 ***************************************************************************/
EXPORT_API
int avsys_audio_open(avsys_audio_param_t *param, avsys_handle_t *phandle, int *size)
{
	int handle = -1;
	avsys_audio_handle_t *p = NULL;
	int err = AVSYS_STATE_ERR_UNKNOWN;

	avsys_info(AVAUDIO, "%s\n", __func__);

	if (param == NULL || phandle == NULL) {
		avsys_error(AVAUDIO, "param or phandle is null\n");
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	if (param->channels > 2 || param->channels < 1) {
		return AVSYS_STATE_ERR_INVALID_CHANNEL;
	}

	if (param->mode < AVSYS_AUDIO_MODE_OUTPUT || param->mode >= AVSYS_AUDIO_MODE_NUM) {
		return AVSYS_STATE_ERR_INVALID_MODE;
	}

	if (param->format < AVSYS_AUDIO_FORMAT_MIN || param->format > AVSYS_AUDIO_FORMAT_MAX) {
		return AVSYS_STATE_ERR_INVALID_FORMAT;
	}

	AVSYS_STREAM_LOCK();

	err = avsys_audio_handle_rejuvenation();
	if (AVSYS_FAIL(err)) {
		avsys_error(AVAUDIO, "Unused handle cleanup before handle allocation failed in %s\n", __func__);
		goto error;
	}
	err = avsys_audio_handle_alloc(&handle);
	if (AVSYS_STATE_ERR_RANGE_OVER == err) {
		avsys_error(AVAUDIO, "audio handle is fully allocated..try cleanup\n");
		err = avsys_audio_handle_rejuvenation();
		if (AVSYS_FAIL(err)) {
			avsys_error(AVAUDIO, "Unused handle cleanup failed in %s\n", __func__);
			goto error;
		}
		avsys_error(AVAUDIO, "one more try...to allocate audio handle\n");
		err = avsys_audio_handle_alloc(&handle);
		if (AVSYS_FAIL(err)) {
			avsys_error(AVAUDIO, "handle alloc failed 1 in %s\n", __func__);
			goto error;
		}
	} else if ((AVSYS_FAIL(err)) && (err != AVSYS_STATE_ERR_RANGE_OVER)) {
		avsys_error(AVAUDIO, "handle alloc failed 2 in %s\n", __func__);
		goto error;
	}

	err = avsys_audio_handle_get_ptr(handle, &p, HANDLE_PTR_MODE_NORMAL);
	if (AVSYS_FAIL(err)) {
		goto error;
	}

	/* set information to handle */
	err = __avsys_audio_set_info(p, param);
	if (AVSYS_FAIL(err)) {
		goto error;
	}

	if (p->mode == AVSYS_AUDIO_MODE_OUTPUT || p->mode == AVSYS_AUDIO_MODE_OUTPUT_CLOCK ||
		p->mode == AVSYS_AUDIO_MODE_OUTPUT_LOW_LATENCY || p->mode == AVSYS_AUDIO_MODE_OUTPUT_AP_CALL || p->mode == AVSYS_AUDIO_MODE_OUTPUT_VIDEO) {
		/* set volume table */
		err = avsys_audio_path_set_volume(handle);
		if (AVSYS_FAIL(err)) {
			goto error;
		}

		/* update volume by type */
		err = avsys_audio_handle_update_volume(p, p->gain_setting.vol_type);
		if (AVSYS_FAIL(err)) {
			goto error;
		}
		err = avsys_audio_handle_update_priority(handle, param->priority, param->handle_route, AVSYS_AUDIO_SET_PRIORITY);
		if (AVSYS_FAIL(err)) {
			goto error;
		}
	}

	/* open device */
	err = avsys_audio_pasimple_open_device(p->mode, p->format, p->channels, p->samplerate, p, param->handle_route);
	if (AVSYS_FAIL(err)) {
		goto error;
	}
	switch (p->mode) {
	case AVSYS_AUDIO_MODE_OUTPUT:
	case AVSYS_AUDIO_MODE_OUTPUT_CLOCK:
	case AVSYS_AUDIO_MODE_OUTPUT_LOW_LATENCY:
	case AVSYS_AUDIO_MODE_OUTPUT_VIDEO:
	case AVSYS_AUDIO_MODE_OUTPUT_AP_CALL:
		if (AVSYS_FAIL(avsys_audio_pasimple_set_volume(p, p->working_vol.level[AVSYS_AUDIO_CHANNEL_LEFT]))) {
			avsys_error(AVAUDIO, "can not set volume in %s\n", __func__);
		}
		break;
	default:
		break;
	}

	*phandle = (avsys_handle_t) handle;
	/* set recommended buffer size */
	if (size != NULL)
		*size = p->period;
	else
		avsys_warning(AVAUDIO, "Size is null\n");

	err = avsys_audio_handle_release_ptr(handle, HANDLE_PTR_MODE_NORMAL);
	if (AVSYS_FAIL(err)) {
		goto error;
	}
	AVSYS_STREAM_UNLOCK();
	return AVSYS_STATE_SUCCESS;

error:
	if (p) {
		avsys_audio_handle_release_ptr(handle, HANDLE_PTR_MODE_NORMAL);
	}

	if (handle != -1) {
		if (AVSYS_FAIL(avsys_audio_handle_free(handle))) {
			avsys_error(AVAUDIO, "Can not free handle %d\n", handle);
		}
	}

	avsys_error(AVAUDIO, "failed to open : RESION %x\n", err);

	*phandle = (avsys_handle_t)-1;
	AVSYS_STREAM_UNLOCK();
	return err;
}

EXPORT_API
int avsys_audio_close(avsys_handle_t handle)
{
	avsys_audio_handle_t *p = NULL;
	int err = AVSYS_STATE_ERR_UNKNOWN;

	AVSYS_STREAM_LOCK();
	avsys_info(AVAUDIO, "%s, handle=[%d]\n", __func__, (int)handle);

	err = avsys_audio_handle_get_ptr((int)handle, &p, HANDLE_PTR_MODE_NORMAL);
	if (AVSYS_FAIL(err)) {
		AVSYS_STREAM_UNLOCK();
		return err;
	}

	if (AVSYS_FAIL(avsys_audio_handle_update_priority((int)handle, p->priority, AVSYS_AUDIO_HANDLE_ROUTE_FOLLOWING_POLICY, AVSYS_AUDIO_UNSET_PRIORITY))) {
		avsys_error(AVAUDIO, "unset priority of handle %d error: %x\n", handle, err);
	}

	err = avsys_audio_pasimple_close_device(p);
	if (AVSYS_FAIL(err)) {
		avsys_error_r(AVAUDIO, "audio device close error : %x\n", err);
	}

	if (AVSYS_FAIL(avsys_audio_handle_release_ptr((int)handle, HANDLE_PTR_MODE_NORMAL))) {
		avsys_error(AVAUDIO, "audio handle release failed\n");
		AVSYS_STREAM_UNLOCK();
		return AVSYS_STATE_ERR_INTERNAL;
	}

	avsys_audio_handle_free((int)handle);

	AVSYS_STREAM_UNLOCK();

	return err;
}

EXPORT_API
int avsys_audio_ampon(void)
{
	avsys_info(AVAUDIO, "%s\n", __func__);
	return avsys_audio_path_ex_set_amp(1);
}

EXPORT_API
int avsys_audio_ampoff(void)
{
	avsys_info(AVAUDIO, "%s\n", __func__);
	return avsys_audio_path_ex_set_amp(0);
}


EXPORT_API
int avsys_audio_ext_device_ampon(avsysaudio_ext_device_t device_type)
{
	avsys_info(AVAUDIO, "%s\n", __func__);
	return avsys_audio_handle_ext_dev_set_mute(device_type, AVSYS_AUDIO_UNMUTE);
}

EXPORT_API
int avsys_audio_ext_device_ampoff(avsysaudio_ext_device_t device_type)
{
	avsys_info(AVAUDIO, "%s\n", __func__);
	return avsys_audio_handle_ext_dev_set_mute(device_type, AVSYS_AUDIO_MUTE);
}

EXPORT_API
int avsys_audio_set_ext_device_status(avsysaudio_ext_device_t device_type, int onoff)
{
	avsys_info(AVAUDIO, "%s\n", __func__);
	return avsys_audio_handle_ext_dev_status_update(device_type, onoff);
}

EXPORT_API
int avsys_audio_get_ext_device_status(avsysaudio_ext_device_t device_type, int *onoff)
{
	avsys_info(AVAUDIO, "%s\n", __func__);
	return avsys_audio_handle_ext_dev_status(device_type, onoff);
}

EXPORT_API
int avsys_audio_flush(avsys_handle_t handle)
{
	int err = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_t *p = NULL;
	avsys_info(AVAUDIO, "%s\n", __func__);

	AVSYS_GET_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	err = avsys_audio_pasimple_reset(p);

	AVSYS_RELEASE_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	return err;
}

EXPORT_API
int avsys_audio_drain(avsys_handle_t handle)
{
	int err = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_t *p = NULL;
	avsys_info(AVAUDIO, "%s\n", __func__);

	AVSYS_GET_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	err = avsys_audio_pasimple_drain(p);

	AVSYS_RELEASE_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	return err;
}

EXPORT_API
int avsys_audio_read(avsys_handle_t handle, void *buf, int size)
{
	int err = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_t *p = NULL;

	avsys_info(AVAUDIO, "%s\n", __func__);

	if (buf == NULL) {
		avsys_error(AVAUDIO, "input buffer pointer is null\n");
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	AVSYS_GET_HANDLE_PTR(HANDLE_PTR_MODE_FAST);

	if (p->mode != AVSYS_AUDIO_MODE_INPUT && p->mode != AVSYS_AUDIO_MODE_INPUT_LOW_LATENCY &&
		p->mode != AVSYS_AUDIO_MODE_INPUT_HIGH_LATENCY && p->mode != AVSYS_AUDIO_MODE_INPUT_AP_CALL) {
		avsys_error(AVAUDIO, "opened output mode\n");
		return AVSYS_STATE_ERR_INVALID_MODE;
	}

	err = avsys_audio_pasimple_read(p, buf, size);

	AVSYS_RELEASE_HANDLE_PTR(HANDLE_PTR_MODE_FAST);

	return err;
}

EXPORT_API
int avsys_audio_write(avsys_handle_t handle, void *buf, int size)
{
	int err = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_t *p = NULL;

	avsys_info(AVAUDIO, "%s\n", __func__);

	if (buf == NULL) {
		avsys_error(AVAUDIO, "buf is null\n");
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	AVSYS_GET_HANDLE_PTR(HANDLE_PTR_MODE_FAST);

	if (p->mode != AVSYS_AUDIO_MODE_OUTPUT && p->mode != AVSYS_AUDIO_MODE_OUTPUT_CLOCK &&
		p->mode != AVSYS_AUDIO_MODE_OUTPUT_LOW_LATENCY && p->mode != AVSYS_AUDIO_MODE_OUTPUT_AP_CALL && p->mode != AVSYS_AUDIO_MODE_OUTPUT_VIDEO) {
		avsys_error(AVAUDIO, "opened input mode\n");
		avsys_audio_handle_release_ptr((int)handle, HANDLE_PTR_MODE_FAST);
		return AVSYS_STATE_ERR_INVALID_MODE;
	}

	if (p->fadeup_vol > 1) {
		if (p->fadeup_multiplier == 0) {
			avsys_audio_volume_t fade_volume;
			fade_volume.level[AVSYS_AUDIO_CHANNEL_LEFT] = p->setting_vol.level[AVSYS_AUDIO_CHANNEL_LEFT] - (p->fadeup_vol - FADEUP_CALC_BIAS);
			fade_volume.level[AVSYS_AUDIO_CHANNEL_RIGHT] = p->setting_vol.level[AVSYS_AUDIO_CHANNEL_LEFT] - (p->fadeup_vol - FADEUP_CALC_BIAS);

			if (fade_volume.level[AVSYS_AUDIO_CHANNEL_LEFT] < 0) {
				fade_volume.level[AVSYS_AUDIO_CHANNEL_LEFT] = 0;
			}
			if (fade_volume.level[AVSYS_AUDIO_CHANNEL_RIGHT] < 0) {
				fade_volume.level[AVSYS_AUDIO_CHANNEL_RIGHT] = 0;
			}

			avsys_info(AVAUDIO, "fade_volume : %d (%d-(%d)+%d) p->fadeup_m = %d, p->msec_per_period = %d\n",
						fade_volume.level[AVSYS_AUDIO_CHANNEL_LEFT],p->setting_vol.level[AVSYS_AUDIO_CHANNEL_LEFT],p->fadeup_vol,FADEUP_CALC_BIAS,
						p->fadeup_multiplier, p->msec_per_period
						);

			avsys_audio_logical_volume_convert(&fade_volume, &p->working_vol, &p->gain_setting);
			avsys_audio_pasimple_set_volume(p, p->working_vol.level[AVSYS_AUDIO_CHANNEL_LEFT]);
			p->fadeup_vol--;
			if (p->msec_per_period > 50)
				p->fadeup_multiplier = 0;
			else
				p->fadeup_multiplier = FADE_UP_MULTIPLIER;
		} else {
			p->fadeup_multiplier--;
		}
	} else if (p->fadeup_vol <= -1) {
		if (p->fadeup_multiplier == 0) {
			int fadedown_vol = 0;
			avsys_audio_volume_t fade_volume;

			fadedown_vol = (-1) * (p->fadeup_vol) - FADEUP_CALC_BIAS;
			fade_volume.level[AVSYS_AUDIO_CHANNEL_LEFT] = fadedown_vol;
			fade_volume.level[AVSYS_AUDIO_CHANNEL_RIGHT] = fadedown_vol;

			if (fade_volume.level[AVSYS_AUDIO_CHANNEL_LEFT] < 0) {
				fade_volume.level[AVSYS_AUDIO_CHANNEL_LEFT] = 0;
			}
			if (fade_volume.level[AVSYS_AUDIO_CHANNEL_RIGHT] < 0) {
				fade_volume.level[AVSYS_AUDIO_CHANNEL_RIGHT] = 0;
			}

			avsys_info(AVAUDIO, "fade_volume : %d (%d-%d) p->fadeup_m = %d, p->msec_per_period = %d\n",
					fade_volume.level[AVSYS_AUDIO_CHANNEL_LEFT],p->fadeup_vol,FADEUP_CALC_BIAS,
					p->fadeup_multiplier, p->msec_per_period);

			avsys_audio_logical_volume_convert(&fade_volume, &p->working_vol, &p->gain_setting);
			avsys_audio_pasimple_set_volume(p, p->working_vol.level[AVSYS_AUDIO_CHANNEL_LEFT]);
			if (p->fadeup_vol < -1) {
				p->fadeup_vol++;
			} else if (p->fadeup_vol == -1) {
				p->mute = AVSYS_AUDIO_MUTE;
			}
			if (p->msec_per_period > 50)
				p->fadeup_multiplier = 0;
			else
				p->fadeup_multiplier = FADE_DOWN_MULTIPLIER;
		} else {
			p->fadeup_multiplier--;
		}
	}
	err = avsys_audio_pasimple_write(p, buf, size);

	AVSYS_RELEASE_HANDLE_PTR(HANDLE_PTR_MODE_FAST);

	return err;
}

EXPORT_API
int avsys_audio_set_volume_fadeup(avsys_handle_t handle)
{
	int err = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_t *p = NULL;

	avsys_warning(AVAUDIO, "%s\n", __func__);

	AVSYS_GET_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	if (p->setting_vol.level[AVSYS_AUDIO_CHANNEL_LEFT] >= p->setting_vol.level[AVSYS_AUDIO_CHANNEL_RIGHT]) {
		p->fadeup_vol = p->setting_vol.level[AVSYS_AUDIO_CHANNEL_LEFT]; /* + FADEUP_CALC_BIAS */
	} else {
		p->fadeup_vol = p->setting_vol.level[AVSYS_AUDIO_CHANNEL_RIGHT]; /* + FADEUP_CALC_BIAS; */
	}
	p->fadeup_multiplier = 0;

	AVSYS_RELEASE_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	return err;
}

EXPORT_API
int avsys_audio_set_mute_fadedown(avsys_handle_t handle)
{
	int err = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_t *p = NULL;

	avsys_warning(AVAUDIO, "%s\n", __func__);

	AVSYS_GET_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	if (p->setting_vol.level[AVSYS_AUDIO_CHANNEL_LEFT] >= p->setting_vol.level[AVSYS_AUDIO_CHANNEL_RIGHT]) {
		p->fadeup_vol = (-1) * p->setting_vol.level[AVSYS_AUDIO_CHANNEL_LEFT];
	} else {
		p->fadeup_vol = (-1) * p->setting_vol.level[AVSYS_AUDIO_CHANNEL_RIGHT];
	}
	p->fadeup_multiplier = 0;

	AVSYS_RELEASE_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	return err;
}

/* Tuning part */
EXPORT_API
int avsys_audio_set_volume_table(int gain_type, int dev_type, int step, int lv, int rv)
{
	int ret = avsys_audio_logical_volume_set_to_table(gain_type, dev_type, step, lv, rv);
	avsys_audio_handle_t *ptr = NULL;
	int handle = -1;

	if (AVSYS_FAIL(ret)) {
		return ret;
	}

	while(++handle < AVSYS_AUDIO_HANDLE_MAX) {
		ptr = NULL;

		if (AVSYS_SUCCESS(avsys_audio_handle_get_ptr(handle, &ptr, HANDLE_PTR_MODE_NORMAL))) {
			avsys_audio_logical_volume_convert(&ptr->setting_vol, &ptr->working_vol, &ptr->gain_setting);
			avsys_audio_handle_release_ptr(handle, HANDLE_PTR_MODE_NORMAL);
		}
	}
	return AVSYS_STATE_SUCCESS;
}

EXPORT_API
int avsys_audio_get_volume_table(int gain_type, int dev_type, int step, int *lv, int *rv)
{
	return avsys_audio_logical_volume_get_from_table(gain_type, dev_type, step, lv, rv);
}

EXPORT_API
int avsys_audio_get_volume_max_ex(int volume_type, int *max_step)
{
	int volume_table = 0;
	if (max_step == NULL) {
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	volume_table = volume_type;

	return avsys_audio_logical_volume_get_max(volume_table, AVSYS_AUDIO_LVOL_DEV_TYPE_SPK, max_step);
}

EXPORT_API
int avsys_audio_set_mute(avsys_handle_t handle, int mute)
{
	avsys_info(AVAUDIO, "%s\n", __func__);

	if (mute > AVSYS_AUDIO_MUTE || mute < AVSYS_AUDIO_UNMUTE) {
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	if (AVSYS_FAIL(avsys_audio_handle_set_mute((int)handle, mute))) {
		avsys_error(AVAUDIO, "failed to set handle mute\n");
	}

	return AVSYS_STATE_SUCCESS;
}

EXPORT_API
int avsys_audio_get_mute(avsys_handle_t handle, int *pmute)
{
	int err = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_t *p = NULL;

	avsys_info(AVAUDIO, "%s\n", __func__);

	if (pmute == NULL) {
		avsys_error(AVAUDIO, "pvolume is null\n");
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	AVSYS_GET_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	*pmute = p->mute;

	AVSYS_RELEASE_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	return AVSYS_STATE_SUCCESS;
}

/*
 * Option : AVSYS_AUDIO_PATH_OPTION_LEGACY_MODE support NowPlus style sound path function
 * Option : AVSYS_AUDIO_PATH_OPTION_DUAL_OUT will effect only when out is speaker.
 * Option : AVSYS_AUDIO_PATH_OPTION_JACK_AUTO will effect only when out is speaker or receiver.
 * Option : AVSYS_AUDIO_PATH_OPTION_FORCED is avail only for shutdown animation
 *
 * Limitation : Only FORCED option can be used same time with other options (exclude LEGACY_MODE)
 */
EXPORT_API
int avsys_audio_set_path_ex(int gain, int out, int in, int option)
{
	if (AVSYS_AUDIO_GAIN_EX_KEYTONE > gain || AVSYS_AUDIO_GAIN_EX_MAX <= gain ||
		AVSYS_AUDIO_PATH_EX_NONE > out || AVSYS_AUDIO_PATH_EX_OUTMAX <= out ||
		AVSYS_AUDIO_PATH_EX_NONE > in || AVSYS_AUDIO_PATH_EX_INMAX <= in) {
		avsys_error(AVAUDIO, "Your input parameter is invalid. Please check\n");
		avsys_error(AVAUDIO, " gain %d, out %d, in %d\n", gain, out, in);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	return avsys_audio_path_ex_set_path(gain, out, in, option);
}
EXPORT_API
int avsys_audio_get_path_ex(int *gain, int *out, int *in, int *option)
{
	if (!gain || !out || !in || !option) {
		avsys_warning(AVAUDIO, "Your input parameter is NULL pointer. Please check.\n");
		avsys_warning(AVAUDIO, " gain %p, out %p, in %p, option %p\n", gain, out, in, option);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}
	return avsys_audio_path_ex_get_path(gain, out, in, option);
}

EXPORT_API
int avsys_audio_set_global_mute(int mute)
{
	int err = AVSYS_STATE_SUCCESS;

	avsys_info(AVAUDIO, "%s : mute=%d\n", __func__, mute);

	if (mute < AVSYS_AUDIO_UNMUTE && mute > AVSYS_AUDIO_MUTE_NOLOCK) {
		err = AVSYS_STATE_ERR_INVALID_PARAMETER;
	} else {
		err = avsys_audio_path_ex_set_mute(mute);
	}
	return err;
}

EXPORT_API
int avsys_audio_get_global_mute(int *pmute)
{
	int err = AVSYS_STATE_SUCCESS;

	avsys_info(AVAUDIO, "%s\n", __func__);
	if (pmute == NULL) {
		err = AVSYS_STATE_ERR_NULL_POINTER;
	} else {
		err = avsys_audio_path_ex_get_mute(pmute);
	}
	return err;
}

/**
 * Internal functions implementation
 */

static int __avsys_audio_set_info(avsys_audio_handle_t *p, avsys_audio_param_t *param)
{
	avsys_info(AVAUDIO, "%s\n", __func__);

	avsys_info(AVAUDIO, "=============================================\n");
	avsys_info(AVAUDIO, "      Input Parameters (Basic Information)\n");
	avsys_info(AVAUDIO, "=============================================\n");
	avsys_info(AVAUDIO, " Op Mode    = %d (0:out, 1:input)\n", param->mode);
	avsys_info(AVAUDIO, " format     = %d (0: 8bits, 1:16bits, 2:32bits)\n", param->format);
	avsys_info(AVAUDIO, " channel    = %d\n", param->channels);
	avsys_info(AVAUDIO, " samplerate = %d\n", param->samplerate);
	avsys_info(AVAUDIO, " route      = %d (0: default, 1: handset)\n", param->handle_route);
	avsys_info(AVAUDIO, " Vol type   = %d\n", param->vol_type);
	avsys_info(AVAUDIO, "=============================================\n");

	p->mode = param->mode;
	p->channels = param->channels;
	p->samplerate = param->samplerate;
	p->format = param->format;
	p->priority = param->priority;

	if ((param->vol_type < 0) || (param->vol_type >= AVSYS_AUDIO_VOLUME_TYPE_MAX)) {
		avsys_error(AVAUDIO, "[%s] Invalid volume type %d. use default system type\n", __func__, param->vol_type);
		p->gain_setting.vol_type = AVSYS_AUDIO_VOLUME_TYPE_SYSTEM;
	} else {
		p->gain_setting.vol_type = param->vol_type;
	}

	/* trivial volume value */
	p->setting_vol.level[AVSYS_AUDIO_CHANNEL_LEFT] = 0;
	p->setting_vol.level[AVSYS_AUDIO_CHANNEL_RIGHT] = 0;
	p->working_vol.level[AVSYS_AUDIO_CHANNEL_LEFT] = 0;
	p->working_vol.level[AVSYS_AUDIO_CHANNEL_RIGHT] = 0;
	p->fadeup_vol = 0;

	return AVSYS_STATE_SUCCESS;
}

EXPORT_API
int avsys_audio_earjack_manager_init(int *earjack_type, int *waitfd)
{
	return avsys_audio_path_earjack_init(earjack_type, waitfd);
}

EXPORT_API
int avsys_audio_earjack_manager_wait(int waitfd, int *current_earjack_type, int *new_earjack_type, int *need_mute)
{
	return avsys_audio_path_earjack_wait(waitfd, current_earjack_type, new_earjack_type, need_mute);
}

EXPORT_API
int avsys_audio_earjack_manager_process(int new_earjack_type)
{
	return avsys_audio_path_earjack_process(new_earjack_type);
}

EXPORT_API
int avsys_audio_earjack_manager_deinit(int waitfd)
{
	return avsys_audio_path_earjack_deinit(waitfd);
}

EXPORT_API
int avsys_audio_earjack_manager_get_type(void)
{
	return avsys_audio_path_earjack_get_type();
}

EXPORT_API
int avsys_audio_earjack_manager_unlock(void)
{
	return avsys_audio_path_earjack_unlock();
}

EXPORT_API
int avsys_audio_set_route_policy(avsys_audio_route_policy_t route)
{
	/* Deprecated */
	return 0;
}

EXPORT_API
int avsys_audio_get_route_policy(avsys_audio_route_policy_t *route)
{
	/* Deprecated */
	return 0;
}

EXPORT_API
int avsys_audio_get_current_playing_volume_type(int *volume_type)
{
	if (volume_type == NULL)
		return AVSYS_STATE_ERR_NULL_POINTER;
	return avsys_audio_handle_current_playing_volume_type(volume_type);
}

static inline int __avsys_audio_validate_volume(const int type, const int value)
{
	if (value < 0)
		return -1;
	switch (type) {
	case AVSYS_AUDIO_VOLUME_TYPE_CALL:
		if (value >= LVOLUME_MAX_BASIC) {
			return -1;
		}
		break;
	case AVSYS_AUDIO_VOLUME_TYPE_ALARM:
	case AVSYS_AUDIO_VOLUME_TYPE_RINGTONE:
	case AVSYS_AUDIO_VOLUME_TYPE_NOTIFICATION:
	case AVSYS_AUDIO_VOLUME_TYPE_SYSTEM:
	case AVSYS_AUDIO_VOLUME_TYPE_MEDIA:
	case AVSYS_AUDIO_VOLUME_TYPE_MEDIA_HL:
	case AVSYS_AUDIO_VOLUME_TYPE_EXT_SYSTEM_JAVA:
		if (value >= LVOLUME_MAX_MULTIMEDIA) {
			return -1;
		}
		break;
	case AVSYS_AUDIO_VOLUME_TYPE_EXT_SYSTEM_ANDROID:
		if (value >= LVOLUME_MAX_SINGLE) {
			return -1;
		}
		break;
	default:
		return -1;
		break;
	}
	return 0;
}

EXPORT_API
int avsys_audio_set_volume_by_type(const int type, const int value)
{
	if (type < 0 || type >= AVSYS_AUDIO_VOLUME_TYPE_MAX)
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	if (0 > __avsys_audio_validate_volume(type, value))
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	return avsys_audio_handle_update_volume_by_type(type, value);
}

EXPORT_API
int avsys_audio_set_primary_volume(const int pid, const int type)
{
	return avsys_audio_handle_set_primary_volume_type(pid, type, AVSYS_AUDIO_PRIMARY_VOLUME_SET);
}

EXPORT_API
int avsys_audio_clear_primary_volume(const int pid)
{
	return avsys_audio_handle_set_primary_volume_type(pid, 0, AVSYS_AUDIO_PRIMARY_VOLUME_CLEAR);
}

EXPORT_API
int avsys_audio_hibernation_reset(int *vol)
{
	int err = AVSYS_STATE_SUCCESS;
	err = avsys_audio_path_ex_reset(1);
	if (AVSYS_FAIL(err)) {
		avsys_error(AVAUDIO, "avsys_audio_path_ex_reset(forced) failed 0x%x\n", err);
		return err;
	}

	err = avsys_audio_handle_reset(vol);
	if (AVSYS_FAIL(err)) {
		avsys_error(AVAUDIO, "avsys_audio_handle_reset() failed 0x%x\n", err);
		return err;
	}
	return err;
}

EXPORT_API
int avsys_audio_delay(avsys_handle_t handle, int *delay)
{
	int err = AVSYS_STATE_SUCCESS;
	int frame_delay = 0;
	avsys_audio_handle_t *p = NULL;

	AVSYS_GET_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	err = avsys_audio_pasimple_delay(p, &frame_delay);
	if (AVSYS_SUCCESS(err)) {
		*delay = frame_delay;
	}

	AVSYS_RELEASE_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	return err;
}

EXPORT_API
int avsys_audio_reset(avsys_handle_t handle)
{
	int err = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_t *p = NULL;

	AVSYS_GET_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	err = avsys_audio_pasimple_reset(p);
	if (AVSYS_FAIL(err)) {
		avsys_error(AVAUDIO, "avsys_audio_reset() failed, 0x%X\n", err);
	}

	AVSYS_RELEASE_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	return err;

}

EXPORT_API
int avsys_audio_get_period_buffer_time(avsys_handle_t handle, unsigned int *period_time, unsigned int *buffer_time)
{
	int err = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_t *p = NULL;
	unsigned int p_time = 0, b_time=0;

	AVSYS_GET_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	err = avsys_audio_pasimple_get_period_buffer_time(p, &p_time, &b_time);
	if(AVSYS_FAIL(err)) {
		avsys_error(AVAUDIO, "avsys_audio_get_period_buffer_time() failed, 0x%X\n",err);
	}
	else
	{
		*period_time = p_time;
		*buffer_time = b_time;
		avsys_info(AVAUDIO,"period time : %u, buffer_time : %u\n", p_time, b_time);
	}

	AVSYS_RELEASE_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	return err;
}

EXPORT_API
int avsys_audio_cork (avsys_handle_t handle, int cork)
{
	int err = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_t *p = NULL;

	AVSYS_GET_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	err = avsys_audio_pasimple_cork(p, cork);
	if(AVSYS_FAIL(err)) {
		avsys_error(AVAUDIO, "avsys_audio_pasimple_cork() failed, 0x%X\n",err);
	}

	AVSYS_RELEASE_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	return err;
}

EXPORT_API
int avsys_audio_is_corked (avsys_handle_t handle, int *is_corked)
{
	int err = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_t *p = NULL;

	if (is_corked == NULL) {
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	AVSYS_GET_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	err = avsys_audio_pasimple_is_corked(p, is_corked);
	if(AVSYS_FAIL(err)) {
		avsys_error(AVAUDIO, "avsys_audio_pasimple_cork() failed, 0x%X\n",err);
	}

	AVSYS_RELEASE_HANDLE_PTR(HANDLE_PTR_MODE_NORMAL);

	return err;
}

EXPORT_API
int avsys_audio_get_capture_status(int *on_capture)
{
	return avsys_audio_handle_current_capture_status(on_capture);
}

__attribute__ ((constructor))
void __init_module(void)
{
}

__attribute__ ((destructor))
void __fini_module(void)
{
}
