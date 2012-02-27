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

#include <string.h>
#include <stdio.h>

#include "avsys-audio-pactrl.h"
#include "avsys-debug.h"

enum {
	AVSYS_PA_CTL_CMD_VOLUME,
	AVSYS_PA_CTL_CMD_MUTE,
	AVSYS_PA_CTL_CMD_GET_SINK_INFO,
	AVSYS_PA_CTL_CMD_GET_DEFAULT_SINK,
	AVSYS_PA_CTL_CMD_MAX,
};

typedef struct avsys_pa_ctrl_info {
	pa_threaded_mainloop *m;
	int cmd;
	int mute;
	int idx;
	int vol;
	int ch;
	int res;
} avsys_pa_ctrl_info_t;

static void __avsys_audio_pa_ctrl_success_cb(pa_context *c, int success, void *userdata)
{
	pa_threaded_mainloop *mainloop = (pa_threaded_mainloop *)userdata;
	avsys_assert_r(c);
	avsys_assert_r(mainloop);

	if (!success) {
		avsys_error(AVAUDIO, "pa control failed\n");
	} else {
		avsys_info(AVAUDIO, "pa control success\n");
	}
	pa_threaded_mainloop_signal(mainloop, 0);
}

static void __avsys_audio_pa_ctrl_get_sink_info_cb(pa_context *c, const pa_sink_info *i, int is_last, void *userdata)
{
	avsys_pa_ctrl_info_t *info = NULL;
	avsys_assert_r(userdata);
	info = (avsys_pa_ctrl_info_t *)userdata;
	int sink_state;
	int sink_name;

	if (is_last < 0) {
		avsys_error(AVAUDIO, "Failed to get sink information: %s", pa_strerror(pa_context_errno(c)));
		return;
	}

	if (is_last) {
		pa_threaded_mainloop_signal(info->m, 0);
		return;
	}

	avsys_assert_r(i);

	if (!strncmp(i->name, "alsa_", 4)) {
		sink_name = ALSA_SINK;
	} else if (!strncmp(i->name, "bluez", 5)) {
		sink_name = BLUEZ_SINK;
	} else {
		return;
	}

	switch (i->state) {
	case PA_SINK_RUNNING:
		sink_state = _ACTIVE;
		break;
	case PA_SINK_IDLE:
	case PA_SINK_SUSPENDED:
		sink_state = _DEACTIVE;
		break;
	default:
		sink_state = _EXIST;
		break;
	}

	info->res |= ((1 << (sink_state + sink_name)));
}

static void __avsys_audio_pa_ctrl_get_server_info_cb(pa_context *c, const pa_server_info *i, void *userdata)
{
	avsys_pa_ctrl_info_t *info = NULL;
	avsys_assert_r(userdata);
	info = (avsys_pa_ctrl_info_t *)userdata;

	if (!i) {
		info->res = AVSYS_AUDIO_PA_CTL_SINK_UNKNOWN;
	} else {
		if (strstr(i->default_sink_name, "alsa_"))
			info->res = AVSYS_AUDIO_PA_CTL_SINK_ALSA;
		else if (strstr(i->default_sink_name, "bluez"))
			info->res = AVSYS_AUDIO_PA_CTL_SINK_BLUEZ;
		else
			info->res = AVSYS_AUDIO_PA_CTL_SINK_UNKNOWN;
	}
	pa_threaded_mainloop_signal(info->m, 0);
}

static void __avsys_audio_pa_ctrl_state_cb(pa_context *c, void *userdata)
{
	pa_cvolume cv;
	avsys_pa_ctrl_info_t *info = NULL;

	avsys_assert_r(c);
	info = (avsys_pa_ctrl_info_t *)userdata;
	avsys_assert_r(info);
	avsys_assert_r(info->m);

	switch (pa_context_get_state(c)) {
	case PA_CONTEXT_READY:
		if (info->cmd == AVSYS_PA_CTL_CMD_VOLUME) {
			pa_cvolume_set(&cv, info->ch, info->vol);
			pa_operation_unref(pa_context_set_sink_input_volume(c, info->idx, &cv, __avsys_audio_pa_ctrl_success_cb, (void *)info->m));
		} else if (info->cmd == AVSYS_PA_CTL_CMD_MUTE) {
			pa_operation_unref(pa_context_set_sink_input_mute(c, info->idx, info->mute, __avsys_audio_pa_ctrl_success_cb, (void *)info->m));
		} else if (info->cmd == AVSYS_PA_CTL_CMD_GET_SINK_INFO) {
			pa_operation_unref(pa_context_get_sink_info_list(c, __avsys_audio_pa_ctrl_get_sink_info_cb, (void *)info));
		} else if (info->cmd == AVSYS_PA_CTL_CMD_GET_DEFAULT_SINK) {
			pa_operation_unref(pa_context_get_server_info(c, __avsys_audio_pa_ctrl_get_server_info_cb, (void *)info));
		}
		break;
	case PA_CONTEXT_TERMINATED:
	case PA_CONTEXT_FAILED:
		pa_threaded_mainloop_signal(info->m, 0);
		break;

	case PA_CONTEXT_UNCONNECTED:
	case PA_CONTEXT_CONNECTING:
	case PA_CONTEXT_AUTHORIZING:
	case PA_CONTEXT_SETTING_NAME:
		break;
	}
}

int avsys_audio_pa_ctrl_volume_by_index(unsigned int idx, int volume, int ch)
{
	pa_threaded_mainloop *mainloop = NULL;
	pa_context *context = NULL;
	int error = PA_ERR_INTERNAL;
	avsys_pa_ctrl_info_t *info = NULL;

	if (volume < 0 || volume > 65535) {
		avsys_error(AVAUDIO, "invalid volume value %d\n", volume);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}
	if (ch < 0) {
		avsys_error(AVAUDIO, "invalid channel value\n", ch);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	if (!(mainloop = pa_threaded_mainloop_new()))
		goto fail;

	if (!(context = pa_context_new(pa_threaded_mainloop_get_api(mainloop), NULL)))
		goto fail;

	if (!(info = (avsys_pa_ctrl_info_t *)calloc(sizeof(avsys_pa_ctrl_info_t), 1)))
		goto fail;

	info->m = mainloop;
	info->cmd = AVSYS_PA_CTL_CMD_VOLUME;
	info->idx = idx;
	info->vol = volume;
	info->ch = ch;
	pa_context_set_state_callback(context, __avsys_audio_pa_ctrl_state_cb, (void *)info);

	if (pa_context_connect(context, NULL, 0, NULL) < 0) {
		error = pa_context_errno(context);
		goto fail;
	}

	pa_threaded_mainloop_lock(mainloop);

	if (pa_threaded_mainloop_start(mainloop) < 0)
		goto unlock_and_fail;

	for (;;) {
		pa_context_state_t state;

		state = pa_context_get_state(context);

		if (state == PA_CONTEXT_READY)
			break;

		if (!PA_CONTEXT_IS_GOOD(state)) {
			error = pa_context_errno(context);
			goto unlock_and_fail;
		}

		/* Wait until the context is ready */
		pa_threaded_mainloop_wait(mainloop);
	}

	pa_threaded_mainloop_unlock(mainloop);

	pa_threaded_mainloop_stop(mainloop);
	pa_context_disconnect(context);
	pa_context_unref(context);
	pa_threaded_mainloop_free(mainloop);
	free(info);

	return AVSYS_STATE_SUCCESS;	/* for success */

unlock_and_fail:
	pa_threaded_mainloop_unlock(mainloop);

	if (mainloop)
		pa_threaded_mainloop_stop(mainloop);

	if (context)
		pa_context_disconnect(context);

fail:
	avsys_error(AVAUDIO, "pa error : %s\n", pa_strerror(error));

	if (context)
		pa_context_unref(context);

	if (mainloop)
		pa_threaded_mainloop_free(mainloop);

	if (info)
		free(info);

	return AVSYS_STATE_ERR_INTERNAL;
}

int avsys_audio_pa_ctrl_mute_by_index(unsigned int idx, int mute)
{
	pa_threaded_mainloop *mainloop = NULL;
	pa_context *context = NULL;
	int error = PA_ERR_INTERNAL;
	avsys_pa_ctrl_info_t *info = NULL;

	if (mute != AVSYS_AUDIO_MUTE && mute != AVSYS_AUDIO_UNMUTE) {
		avsys_error(AVAUDIO, "invalid mute value %d\n", mute);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	if (!(mainloop = pa_threaded_mainloop_new()))
		goto fail;

	if (!(context = pa_context_new(pa_threaded_mainloop_get_api(mainloop), NULL)))
		goto fail;

	if (!(info = (avsys_pa_ctrl_info_t *)calloc(sizeof(avsys_pa_ctrl_info_t), 1)))
		goto fail;

	info->m = mainloop;
	info->cmd = AVSYS_PA_CTL_CMD_MUTE;
	info->idx = idx;
	info->mute = mute;

	pa_context_set_state_callback(context, __avsys_audio_pa_ctrl_state_cb, (void *)info);

	if (pa_context_connect(context, NULL, 0, NULL) < 0) {
		error = pa_context_errno(context);
		goto fail;
	}

	pa_threaded_mainloop_lock(mainloop);

	if (pa_threaded_mainloop_start(mainloop) < 0)
		goto unlock_and_fail;

	for (;;) {
		pa_context_state_t state;

		state = pa_context_get_state(context);

		if (state == PA_CONTEXT_READY)
			break;

		if (!PA_CONTEXT_IS_GOOD(state)) {
			error = pa_context_errno(context);
			goto unlock_and_fail;
		}

		/* Wait until the context is ready */
		pa_threaded_mainloop_wait(mainloop);
	}

	pa_threaded_mainloop_unlock(mainloop);

	pa_threaded_mainloop_stop(mainloop);
	pa_context_disconnect(context);
	pa_context_unref(context);
	pa_threaded_mainloop_free(mainloop);
	free(info);

	return AVSYS_STATE_SUCCESS;	/* for success */

unlock_and_fail:
	pa_threaded_mainloop_unlock(mainloop);

	if (mainloop)
		pa_threaded_mainloop_stop(mainloop);

	if (context)
		pa_context_disconnect(context);

fail:
	avsys_error(AVAUDIO, "pa error : %s\n", pa_strerror(error));

	if (context)
		pa_context_unref(context);

	if (mainloop)
		pa_threaded_mainloop_free(mainloop);

	if (info)
		free(info);

	return AVSYS_STATE_ERR_INTERNAL;
}

int avsys_audio_pa_ctrl_get_default_sink(int *sink)
{
	pa_threaded_mainloop *mainloop = NULL;
	pa_context *context = NULL;
	int error = PA_ERR_INTERNAL;
	avsys_pa_ctrl_info_t *info = NULL;

	if (!sink)
		return AVSYS_STATE_ERR_INVALID_PARAMETER;

	if (!(mainloop = pa_threaded_mainloop_new()))
		goto fail;

	if (!(context = pa_context_new(pa_threaded_mainloop_get_api(mainloop), NULL)))
		goto fail;

	if (!(info = (avsys_pa_ctrl_info_t *)calloc(sizeof(avsys_pa_ctrl_info_t), 1)))
		goto fail;

	info->m = mainloop;
	info->cmd = AVSYS_PA_CTL_CMD_GET_DEFAULT_SINK;

	pa_context_set_state_callback(context, __avsys_audio_pa_ctrl_state_cb, (void *)info);

	if (pa_context_connect(context, NULL, 0, NULL) < 0) {
		error = pa_context_errno(context);
		goto fail;
	}

	pa_threaded_mainloop_lock(mainloop);

	if (pa_threaded_mainloop_start(mainloop) < 0)
		goto unlock_and_fail;

	for (;;) {
		pa_context_state_t state;

		state = pa_context_get_state(context);

		if (state == PA_CONTEXT_READY)
			break;

		if (!PA_CONTEXT_IS_GOOD(state)) {
			error = pa_context_errno(context);
			goto unlock_and_fail;
		}

		/* Wait until the context is ready */
		pa_threaded_mainloop_wait(mainloop);
	}

	*sink = info->res;

	pa_threaded_mainloop_unlock(mainloop);

	pa_threaded_mainloop_stop(mainloop);
	pa_context_disconnect(context);
	pa_context_unref(context);
	pa_threaded_mainloop_free(mainloop);
	free(info);

	return AVSYS_STATE_SUCCESS;	/* for success */

unlock_and_fail:
	pa_threaded_mainloop_unlock(mainloop);

	if (mainloop)
		pa_threaded_mainloop_stop(mainloop);

	if (context)
		pa_context_disconnect(context);

fail:
	avsys_error(AVAUDIO, "pa error : %s\n", pa_strerror(error));

	if (context)
		pa_context_unref(context);

	if (mainloop)
		pa_threaded_mainloop_free(mainloop);

	if (info)
		free(info);

	return AVSYS_STATE_ERR_INTERNAL;
}
