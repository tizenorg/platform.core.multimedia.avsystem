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

#ifndef __AVSYS_AUDIO_PASIMPLE_H__
#define __AVSYS_AUDIO_PASIMPLE_H__

#ifdef __cplusplus
	extern "C" {
#endif
#include <avsys-audio-handle.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>
#include <pulse/proplist.h>

typedef struct {
	void				*pasimple_handle;
	unsigned int		samplesize;
	unsigned int		period_frames;
	unsigned int		buffer_frames;
	unsigned int		mode;
	unsigned int		periods_per_buffer;
} avsys_audio_pasimple_handle_t;

int avsys_audio_pasimple_open_device(const int mode, const unsigned int format, const unsigned int channel, const unsigned int samplerate, avsys_audio_handle_t *handle,int policy);
int avsys_audio_pasimple_close_device(avsys_audio_handle_t *handle);
int avsys_audio_pasimple_write(avsys_audio_handle_t *handle, const void *buf, int size);
int avsys_audio_pasimple_read(avsys_audio_handle_t *handle, void *buf, int size);
int avsys_audio_pasimple_reset(avsys_audio_handle_t *handle);
int avsys_audio_pasimple_drain(avsys_audio_handle_t *handle);
int avsys_audio_pasimple_delay(avsys_audio_handle_t *handle, int *delay_frames);
int avsys_audio_pasimple_set_volume(avsys_audio_handle_t *handle, int volume);
int avsys_audio_pasimple_get_period_buffer_time(avsys_audio_handle_t *handle, unsigned int *period_time, unsigned int *buffer_time);

#ifdef __cplusplus
	}
#endif

#endif /* __AVSYS_AUDIO_PASIMPLE_H__ */
