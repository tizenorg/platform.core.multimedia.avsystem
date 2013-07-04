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
#include <string.h>

#include "avsys-audio-pasimple.h"
#include "avsys-types.h"
#include "avsys-error.h"
#include "avsys-debug.h"

#define PA_SIMPLE_SAMPLES_PER_PERIOD_DEFAULT				1536	/* frames */
#define PA_SIMPLE_PERIODS_PER_BUFFER_FASTMODE				4
#define PA_SIMPLE_PERIODS_PER_BUFFER_DEFAULT				6
#define PA_SIMPLE_PERIODS_PER_BUFFER_PLAYBACK				8
#define PA_SIMPLE_PERIODS_PER_BUFFER_CAPTURE				12
#define PA_SIMPLE_PERIODS_PER_BUFFER_VIDEO					10

#define PA_SIMPLE_PERIOD_TIME_FOR_ULOW_LATENCY_MSEC			20
#define PA_SIMPLE_PERIOD_TIME_FOR_LOW_LATENCY_MSEC			25
#define PA_SIMPLE_PERIOD_TIME_FOR_MID_LATENCY_MSEC			50
#define PA_SIMPLE_PERIOD_TIME_FOR_HIGH_LATENCY_MSEC			75

#define MSEC_TO_SAMPLE(samplerate,period_time)		(samplerate*period_time/1000)

#define CHECK_VALID_HANDLE(handle) \
do { \
	if (handle == NULL) { \
		return AVSYS_STATE_ERR_NULL_POINTER; \
	} \
	device = (avsys_audio_pasimple_handle_t *)handle->device; \
		if (device == NULL) { \
			return AVSYS_STATE_ERR_NULL_POINTER; \
		} \
	if (device->pasimple_handle == NULL) { \
		return AVSYS_STATE_ERR_NULL_POINTER; \
	} \
} while (0)

#define SET_PA_ATTR(pt,spp,ppb,pb,mr,tl,ml,fs)	\
do { \
	period_time = pt; \
	samples_per_period = spp; \
	periods_per_buffer = ppb; \
	attr.prebuf = pb; \
	attr.minreq = mr; \
	attr.tlength = tl; \
	attr.maxlength = ml; \
	attr.fragsize = fs; \
} while (0)

#define MEDIA_POLICY_AUTO	"auto"
#define MEDIA_POLICY_PHONE	"phone"
#define MEDIA_POLICY_ALL	"all"
#define MEDIA_ROLE_FILTER	"filter"

int avsys_audio_pasimple_open_device(const int mode, const unsigned int format, const unsigned int channel, const unsigned int samplerate, avsys_audio_handle_t *handle, int policy)
{
	pa_simple *s = NULL;
	pa_sample_spec ss;
	avsys_audio_pasimple_handle_t *device = NULL;
	pa_buffer_attr attr;
	int err = AVSYS_STATE_SUCCESS;
	int period_time = PA_SIMPLE_PERIOD_TIME_FOR_MID_LATENCY_MSEC;
	int samples_per_period = PA_SIMPLE_SAMPLES_PER_PERIOD_DEFAULT;
	int periods_per_buffer = PA_SIMPLE_PERIODS_PER_BUFFER_DEFAULT;
	int vol_conf_type = AVSYS_AUDIO_VOLUME_CONFIG_TYPE(handle->gain_setting.volume_config);
	pa_channel_map channel_map;

	int p_time = PA_SIMPLE_PERIOD_TIME_FOR_HIGH_LATENCY_MSEC;
	int p_count = PA_SIMPLE_PERIODS_PER_BUFFER_PLAYBACK;
	char *time = getenv("AVSYS_PERIOD_TIME");
	char *count = getenv("AVSYS_PERIOD_COUNT");
	if(time)
		p_time = atoi(time);
	if(count)
		p_count = atoi(count);

	avsys_info(AVAUDIO, ">>>[%s] mode=%d, format=%d, channel=%d, samplerate=%d\n", __func__, mode, format, channel, samplerate);
	avsys_assert(handle != NULL);

	if (channel < AVSYS_CHANNEL_MIN || channel > AVSYS_CHANNEL_MAX)
		return AVSYS_STATE_ERR_DEVICE_NOT_SUPPORT;

	device = (avsys_audio_pasimple_handle_t *)malloc(sizeof(avsys_audio_pasimple_handle_t));
	if (device == NULL) {
		avsys_critical(AVAUDIO, "PA Simple handle alloc fail\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}

	ss.rate = samplerate;
	ss.channels = channel;

	pa_channel_map_init_auto(&channel_map, ss.channels, PA_CHANNEL_MAP_ALSA);

	switch (format) {
	case AVSYS_AUDIO_FORMAT_8BIT:
		ss.format = PA_SAMPLE_U8;
		device->samplesize = 1 * channel;
		break;
	case AVSYS_AUDIO_FORMAT_16BIT:

		ss.format = PA_SAMPLE_S16LE;
		device->samplesize = 2 * channel;
		break;
	default:
		free(device);
		avsys_error(AVAUDIO, "Invalid format\n");
		return AVSYS_STATE_ERR_DEVICE_NOT_SUPPORT;
	}
	handle->device = (void *)device;

	pa_proplist *proplist = pa_proplist_new();

	/* Set policy property */
	avsys_info(AVAUDIO, ">>>[%s] policy=[%d], vol_type=[%d]\n", __func__, policy, vol_conf_type);
	if (policy == AVSYS_AUDIO_HANDLE_ROUTE_HANDSET_ONLY) {
		avsys_info(AVAUDIO, ": set media plicy to PHONE\n");
		pa_proplist_sets(proplist, PA_PROP_MEDIA_POLICY, MEDIA_POLICY_PHONE);
	} else {
		/* AVSYS_AUDIO_HANDLE_ROUTE_FOLLOWING_POLICY */
		/* check stream type (vol type) */

		switch (vol_conf_type)
		{
		case AVSYS_AUDIO_VOLUME_TYPE_NOTIFICATION:
		case AVSYS_AUDIO_VOLUME_TYPE_ALARM:
			avsys_info(AVAUDIO, ": set media plicy to ALL\n");
			pa_proplist_sets(proplist, PA_PROP_MEDIA_POLICY, MEDIA_POLICY_ALL);
			break;

		case AVSYS_AUDIO_VOLUME_TYPE_FIXED:	/* Used for Emergency */
			avsys_info(AVAUDIO, ": set media plicy to PHONE\n");
			pa_proplist_sets(proplist, PA_PROP_MEDIA_POLICY, MEDIA_POLICY_PHONE);
			break;

		default:
			avsys_info(AVAUDIO, ": set media plicy to AUTO\n");
			pa_proplist_sets(proplist, PA_PROP_MEDIA_POLICY, MEDIA_POLICY_AUTO);
			break;
		}
	}

	handle->handle_route = policy;

	memset(&attr, '\0', sizeof(attr));

	switch (mode) {
	case AVSYS_AUDIO_MODE_INPUT:
		SET_PA_ATTR(PA_SIMPLE_PERIOD_TIME_FOR_MID_LATENCY_MSEC,
					MSEC_TO_SAMPLE(samplerate,period_time),
					PA_SIMPLE_PERIODS_PER_BUFFER_DEFAULT,
					0, -1, -1, -1, samples_per_period * device->samplesize);

		s = pa_simple_new_proplist(NULL, "AVSYSTEM", PA_STREAM_RECORD, NULL, "CAPTURE", &ss, &channel_map, &attr, proplist, &err);
		break;

	case AVSYS_AUDIO_MODE_INPUT_LOW_LATENCY:
		SET_PA_ATTR(PA_SIMPLE_PERIOD_TIME_FOR_ULOW_LATENCY_MSEC,
					MSEC_TO_SAMPLE(samplerate,period_time),
					PA_SIMPLE_PERIODS_PER_BUFFER_FASTMODE,
					0, -1, -1, -1, samples_per_period * device->samplesize);

		s = pa_simple_new_proplist(NULL, "AVSYSTEM", PA_STREAM_RECORD, NULL, "LOW LATENCY CAPTURE", &ss, &channel_map, &attr, proplist, &err);
		break;

	case AVSYS_AUDIO_MODE_INPUT_HIGH_LATENCY:
		SET_PA_ATTR(PA_SIMPLE_PERIOD_TIME_FOR_HIGH_LATENCY_MSEC,
					MSEC_TO_SAMPLE(samplerate,period_time),
					PA_SIMPLE_PERIODS_PER_BUFFER_CAPTURE,
					0, -1, -1, -1, samples_per_period * device->samplesize);

		s = pa_simple_new_proplist(NULL, "AVSYSTEM", PA_STREAM_RECORD, NULL, "HIGH LATENCY CAPTURE", &ss, &channel_map, &attr, proplist, &err);
		break;

	case AVSYS_AUDIO_MODE_OUTPUT:	/* mid latency playback for normal audio case. */
		SET_PA_ATTR(PA_SIMPLE_PERIOD_TIME_FOR_MID_LATENCY_MSEC,
						MSEC_TO_SAMPLE(samplerate,period_time),
						PA_SIMPLE_PERIODS_PER_BUFFER_DEFAULT,
						-1, -1, periods_per_buffer * samples_per_period * device->samplesize, attr.tlength, 0);

		s = pa_simple_new_proplist(NULL, "AVSYSTEM", PA_STREAM_PLAYBACK, NULL, "PLAYBACK", &ss, &channel_map, &attr, proplist, &err);
		break;

	case AVSYS_AUDIO_MODE_OUTPUT_DSP:	/* audio dsp hw decoding case. */
		avsys_info(AVAUDIO, ": set media role to filter\n");
		pa_proplist_sets(proplist, PA_PROP_MEDIA_ROLE, MEDIA_ROLE_FILTER);

		s = pa_simple_new_proplist(NULL, "AVSYSTEM", PA_STREAM_PLAYBACK, NULL, "DSP PLAYBACK", &ss, &channel_map, &attr, proplist, &err);
		break;

	case AVSYS_AUDIO_MODE_OUTPUT_LOW_LATENCY:	/* This is special case for touch sound playback */
		SET_PA_ATTR(PA_SIMPLE_PERIOD_TIME_FOR_LOW_LATENCY_MSEC,
					MSEC_TO_SAMPLE(samplerate,period_time),
					PA_SIMPLE_PERIODS_PER_BUFFER_FASTMODE,
					samples_per_period * device->samplesize, -1, samples_per_period * device->samplesize + 3430, (uint32_t)-1, 0);

		s = pa_simple_new_proplist(NULL,"AVSYSTEM", PA_STREAM_PLAYBACK, NULL, "LOW LATENCY PLAYBACK", &ss, &channel_map, &attr, proplist, &err);
		break;
	case AVSYS_AUDIO_MODE_OUTPUT_CLOCK: /* high latency playback - lager buffer size */
		SET_PA_ATTR(p_time,
					MSEC_TO_SAMPLE(samplerate,period_time),
					p_count,
					(uint32_t) -1, (uint32_t) -1, periods_per_buffer * samples_per_period * device->samplesize, (uint32_t)-1, 0);

		s = pa_simple_new_proplist(NULL, "AVSYSTEM", PA_STREAM_PLAYBACK, NULL, "HIGH LATENCY PLAYBACK", &ss, &channel_map, &attr, proplist, &err);
		break;

	case AVSYS_AUDIO_MODE_OUTPUT_VIDEO:	/* low latency playback */
		SET_PA_ATTR(PA_SIMPLE_PERIOD_TIME_FOR_LOW_LATENCY_MSEC,
					MSEC_TO_SAMPLE(samplerate,period_time),
					PA_SIMPLE_PERIODS_PER_BUFFER_VIDEO,
					4*(samples_per_period * device->samplesize), samples_per_period * device->samplesize, periods_per_buffer * samples_per_period * device->samplesize, (uint32_t)-1, 0);

		s = pa_simple_new_proplist(NULL, "AVSYSTEM", PA_STREAM_PLAYBACK, NULL, "LOW LATENCY PLAYBACK", &ss, &channel_map, &attr, proplist, &err);
		break;

	case AVSYS_AUDIO_MODE_OUTPUT_AP_CALL:
#if defined(_MMFW_I386_ALL_SIMULATOR)
		avsys_warning(AVAUDIO, "Does not support AP call mode at i386 simulator\n");
		s = NULL;
#else
		SET_PA_ATTR(PA_SIMPLE_PERIOD_TIME_FOR_LOW_LATENCY_MSEC,
					MSEC_TO_SAMPLE(samplerate,period_time),
					PA_SIMPLE_PERIODS_PER_BUFFER_DEFAULT,
					(uint32_t) -1, (uint32_t) -1, periods_per_buffer * samples_per_period * device->samplesize, attr.tlength, 0);


		s = pa_simple_new_proplist(NULL, "AVSYSTEM", PA_STREAM_PLAYBACK, NULL, "VoIP PLAYBACK", &ss, &channel_map, &attr, proplist, &err);
#endif
		break;
	case AVSYS_AUDIO_MODE_INPUT_AP_CALL:
#if defined(_MMFW_I386_ALL_SIMULATOR)
		avsys_warning(AVAUDIO, "Does not support AP call mode at i386 simulator\n");
		s = NULL;
#else
		SET_PA_ATTR(PA_SIMPLE_PERIOD_TIME_FOR_LOW_LATENCY_MSEC,
					MSEC_TO_SAMPLE(samplerate,period_time),
					PA_SIMPLE_PERIODS_PER_BUFFER_DEFAULT,
					0, (uint32_t) -1, (uint32_t) -1, (uint32_t) -1, samples_per_period * device->samplesize);

		s = pa_simple_new_proplist(NULL, "AVSYSTEM", PA_STREAM_RECORD, NULL, "VoIP CAPTURE", &ss, &channel_map, &attr, proplist, &err);
#endif
		break;
	case AVSYS_AUDIO_MODE_CALL_OUT:
	case AVSYS_AUDIO_MODE_CALL_IN:
		//TODO
		avsys_error(AVAUDIO, "Does not support call device handling\n");
		avsys_assert_r(0);
		break;
	default:
		avsys_critical_r(AVAUDIO, "Invalid open mode %d\n", mode);
		avsys_assert_r(0);
		return AVSYS_STATE_ERR_INVALID_MODE;
		break;
	}

	if (!s) {
		avsys_error_r(AVAUDIO, "Open pulseaudio handle has failed - %s\n", pa_strerror(err));
		err = AVSYS_STATE_ERR_INTERNAL;
		goto fail;
	}

	avsys_info(AVAUDIO, "Samples(per period) : %d\t Periods(per buffer) : %d\n", samples_per_period, periods_per_buffer);

	device->pasimple_handle = (void *)s;
	device->mode = mode;
	device->period_frames = samples_per_period;
	device->buffer_frames = periods_per_buffer * device->period_frames;
	device->periods_per_buffer = periods_per_buffer;
	handle->period = device->period_frames * device->samplesize;
	handle->msec_per_period = period_time;
	if (0 > pa_simple_get_stream_index(s, &handle->stream_index, &err)) {
		avsys_error(AVAUDIO, "Can not get stream index %s\n", pa_strerror(err));
		err = AVSYS_STATE_ERR_INVALID_HANDLE;
	}

fail:
	if (proplist)
		pa_proplist_free(proplist);

	return err;
}

int avsys_audio_pasimple_close_device(avsys_audio_handle_t *handle)
{
	int err = 0;
	avsys_audio_pasimple_handle_t *device = NULL;
	pa_simple *s = NULL;

	avsys_info(AVAUDIO, "%s\n", __func__);

	avsys_assert(handle != NULL);
	CHECK_VALID_HANDLE(handle);

	switch (handle->mode) {
	case AVSYS_AUDIO_MODE_CALL_OUT:
	case AVSYS_AUDIO_MODE_CALL_IN:
		avsys_warning(AVAUDIO, "Unsupported close mode in pa function\n");
		return AVSYS_STATE_ERR_INVALID_MODE;
	case AVSYS_AUDIO_MODE_OUTPUT_AP_CALL:
	case AVSYS_AUDIO_MODE_INPUT_AP_CALL:
#if defined(_MMFW_I386_ALL_SIMULATOR)
		avsys_warning(AVAUDIO, "Skip close call device in SDK");
		return AVSYS_STATE_SUCCESS;
#endif
	default:
		break;
	}

	s = (pa_simple *)device->pasimple_handle;
	avsys_assert(s != NULL);

	switch (handle->mode) {
	case AVSYS_AUDIO_MODE_OUTPUT:
	case AVSYS_AUDIO_MODE_OUTPUT_CLOCK:
	case AVSYS_AUDIO_MODE_OUTPUT_LOW_LATENCY:
	case AVSYS_AUDIO_MODE_OUTPUT_AP_CALL:
	case AVSYS_AUDIO_MODE_OUTPUT_VIDEO:
		if (0 > pa_simple_flush(s, &err)) {
			avsys_error(AVAUDIO, "pa_simple_flush() failed with %s\n", pa_strerror(err));
		}
		break;
	default:
		break;
	}

	pa_simple_free(s);

	device->pasimple_handle = NULL;
	free(device);

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_pasimple_write(avsys_audio_handle_t *handle, const void *buf, int size)
{
	pa_simple *s = NULL;
	avsys_audio_pasimple_handle_t *device = NULL;
	int err = 0;

	if (buf == NULL)
			return AVSYS_STATE_ERR_NULL_POINTER;
	CHECK_VALID_HANDLE(handle);

	if (size < 0)
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	else if (size == 0)
		return 0;

	s = (pa_simple *)device->pasimple_handle;

	if (0 > pa_simple_write(s, buf, size, &err)) {
		avsys_error(AVAUDIO, "pa_simple_write() failed with %s\n", pa_strerror(err));
		return AVSYS_STATE_ERR_INTERNAL;
	}

	return size;
}

int avsys_audio_pasimple_read(avsys_audio_handle_t *handle, void *buf, int size)
{
	pa_simple *s = NULL;
	avsys_audio_pasimple_handle_t *device = NULL;
	int err = 0;

	if (buf == NULL)
			return AVSYS_STATE_ERR_NULL_POINTER;
	CHECK_VALID_HANDLE(handle);

	if (size < 0)
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	else if (size == 0)
		return 0;

	s = (pa_simple *)device->pasimple_handle;

	if (0 > pa_simple_read(s, buf, size, &err)) {
		avsys_error(AVAUDIO, "pa_simple_read() failed with %s\n", pa_strerror(err));
		return AVSYS_STATE_ERR_INTERNAL;
	}

	return size;
}

int avsys_audio_pasimple_reset(avsys_audio_handle_t *handle)
{
	pa_simple *s = NULL;
	avsys_audio_pasimple_handle_t *device = NULL;
	int err = 0;

	CHECK_VALID_HANDLE(handle);

	if (handle->mode == AVSYS_AUDIO_MODE_INPUT || handle->mode == AVSYS_AUDIO_MODE_INPUT_LOW_LATENCY) {
		avsys_warning(AVAUDIO, "Skip pa_simple_flush() when input mode\n");
		return AVSYS_STATE_SUCCESS;
	}

	s = (pa_simple *)device->pasimple_handle;

	if (0 > pa_simple_flush(s, &err)) {
		avsys_error(AVAUDIO, "pa_simple_flush() failed with %s\n", pa_strerror(err));
		return AVSYS_STATE_ERR_INTERNAL;
	}

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_pasimple_drain(avsys_audio_handle_t *handle)
{
	pa_simple *s = NULL;
	avsys_audio_pasimple_handle_t *device = NULL;
	int err = 0;

	CHECK_VALID_HANDLE(handle);

	s = (pa_simple *)device->pasimple_handle;

	if (0 > pa_simple_drain(s, &err)) {
		avsys_error(AVAUDIO, "pa_simple_drain() failed with %s\n", pa_strerror(err));
		return AVSYS_STATE_ERR_INTERNAL;
	}

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_pasimple_set_volume(avsys_audio_handle_t *handle, int volume)
{
	pa_simple *s = NULL;
	avsys_audio_pasimple_handle_t *device = NULL;
	int err = 0;

	CHECK_VALID_HANDLE(handle);

	s = (pa_simple *)device->pasimple_handle;

	if (0 > pa_simple_set_volume(s, volume, &err)) {
		avsys_error(AVAUDIO, "pa_simple_set_volume() failed with %s\n", pa_strerror(err));
		return AVSYS_STATE_ERR_INTERNAL;
	}

	return AVSYS_STATE_SUCCESS;
}

#define USEC_TO_SAMPLE(usec, rate)	((usec*rate)/1000000)
#define SAMPLES_TO_USEC(samples,rate)	((samples*1000000)/rate)
#define BYTES_TO_USEC(bytes,size_per_sample,rate)	((bytes*1000000)/(size_per_sample*rate))

int avsys_audio_pasimple_delay(avsys_audio_handle_t *handle, int *delay)
{
	pa_simple *s = NULL;
	avsys_audio_pasimple_handle_t *device = NULL;
	int err = 0;
	pa_usec_t latency_time = 0;
	unsigned int latency_frames = 0;

	if (delay == NULL) {
		return AVSYS_STATE_ERR_NULL_POINTER;
	}
	CHECK_VALID_HANDLE(handle);

	s = (pa_simple *)device->pasimple_handle;

	latency_time = pa_simple_get_latency(s, &err);
	if (err > 0 && latency_time == 0) {
		avsys_error(AVAUDIO, "pa_simple_get_latency() failed with %s\n", pa_strerror(err));
		return AVSYS_STATE_ERR_INTERNAL;
	}
	/* convert time to sample */
	latency_frames = USEC_TO_SAMPLE(latency_time, handle->samplerate);
	*delay = latency_frames;

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_pasimple_get_period_buffer_time(avsys_audio_handle_t *handle, unsigned int *period_time, unsigned int *buffer_time)
{
	avsys_audio_pasimple_handle_t *device = NULL;

	if ((period_time == NULL) || (buffer_time == NULL))
		return AVSYS_STATE_ERR_INTERNAL;

	CHECK_VALID_HANDLE(handle);

	*period_time = SAMPLES_TO_USEC(device->period_frames,handle->samplerate);
	*buffer_time = *period_time * device->periods_per_buffer;

	avsys_info(AVAUDIO, "[%s][%d] period = %d, buffer = %d\n", __func__, __LINE__, *period_time, *buffer_time);

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_pasimple_cork(avsys_audio_handle_t *handle, int cork)
{
	pa_simple *s = NULL;
	avsys_audio_pasimple_handle_t *device = NULL;
	int err = 0;

	CHECK_VALID_HANDLE(handle);

	s = (pa_simple *)device->pasimple_handle;

	if (0 > pa_simple_cork(s, cork, &err)) {
		avsys_error(AVAUDIO, "pa_simple_cork() failed with %s\n", pa_strerror(err));
		return AVSYS_STATE_ERR_INTERNAL;
	}

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_pasimple_is_corked(avsys_audio_handle_t *handle, int *is_corked)
{
	pa_simple *s = NULL;
	avsys_audio_pasimple_handle_t *device = NULL;
	int err = 0;

	if (is_corked == NULL)
		return AVSYS_STATE_ERR_INTERNAL;

	CHECK_VALID_HANDLE(handle);

	s = (pa_simple *)device->pasimple_handle;

	*is_corked = pa_simple_is_corked(s);

	return AVSYS_STATE_SUCCESS;
}


