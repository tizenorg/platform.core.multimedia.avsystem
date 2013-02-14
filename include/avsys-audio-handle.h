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

#ifndef __AVSYS_AUDIO_HANDLE_H__
#define __AVSYS_AUDIO_HANDLE_H__

#include <pthread.h>
#include "avsys-audio-logical-volume.h"

#define AVSYS_AUDIO_HANDLE_MAX 64 /* this is related with allocated bit size in avsys_audio_handle_info_t*/
#define AVSYS_AUDIO_LOCK_SLOT_MAX 4

enum {
	AVSYS_AUDIO_PRIMARY_VOLUME_CLEAR = 0,
	AVSYS_AUDIO_PRIMARY_VOLUME_SET
};

enum {
	AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_0 = 0,	/**< normal */
	AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_1,		/**< solo */
	AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_2,		/**< solo with transition effect */
	AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_MAX,
};

enum {
	AVSYS_AUDIO_HANDLE_EXT_DEV_NONE		= 0x00000000,
	AVSYS_AUDIO_HANDLE_EXT_DEV_FMRADIO	= 0x00000001,
};

typedef struct {
	/* common base */
	int mode;
	int channels;
	int mute;
	int format;
	int samplerate;
	int period;
	int msec_per_period;
	int pid;
	int tid;
	int priority;

	/* logical volume */
	avsys_audio_volume_setting_t gain_setting;
	avsys_audio_volume_t setting_vol;
	avsys_audio_volume_t working_vol;
	int fadeup_vol;
	int fadeup_multiplier;
	char dirty_volume;

	/* routing information */
	char during_cp_audio;/* if this value set 1, this handle created during cp audio status (means call) */
	char path_off;
	int	stream_index;
	int handle_route;

	/* backend specific */
	void	*device;
} avsys_audio_handle_t;

typedef struct {
	long long int allocated;
	int handle_amp; /* 1 for unmute, 0 for mute per each handle*/
	int ext_device_amp;
	int ext_device_status;
	int volume_value[AVSYS_AUDIO_VOLUME_TYPE_MAX];
	int primary_volume_type;
	pid_t primary_volume_pid;
	char handle_priority[AVSYS_AUDIO_HANDLE_MAX];
	pid_t handlelock_pid[AVSYS_AUDIO_LOCK_SLOT_MAX];
	avsys_audio_handle_t handles[AVSYS_AUDIO_HANDLE_MAX];
} avsys_audio_handle_info_t;

enum {
	HANDLE_PTR_MODE_NORMAL,
	HANDLE_PTR_MODE_FAST,
	HANDLE_PTR_MODE_NUM,
};

int avsys_audio_handle_init(void);
int avsys_audio_handle_fini(void);
int avsys_audio_handle_reset(int *volume_value);
int avsys_audio_handle_dump(void);
int avsys_audio_handle_rejuvenation(void);
int avsys_audio_handle_alloc(int *handle);
int avsys_audio_handle_free(int handle);
int avsys_audio_handle_get_ptr(int handle, avsys_audio_handle_t **ptr, const int mode);
int avsys_audio_handle_release_ptr(int handle, const int mode);
int avsys_audio_handle_set_mute(int handle, int mute);
int avsys_audio_handle_ext_dev_set_mute(avsysaudio_ext_device_t device_type, int mute);
int avsys_audio_handle_ext_dev_status(avsysaudio_ext_device_t device_type, int *onoff);
int avsys_audio_handle_ext_dev_status_update(avsysaudio_ext_device_t device_type, int onoff);
int avsys_audio_handle_current_playing_volume_type(int *type);
int avsys_audio_handle_update_volume(avsys_audio_handle_t *p, const int volume_config);
int avsys_audio_handle_update_volume_by_type(const int volume_type, const int volume_value);
int avsys_audio_handle_set_primary_volume_type(const int pid, const int type, const int command);
int avsys_audio_handle_update_priority(int handle, int priority, int handle_route, int cmd);
int avsys_audio_handle_current_capture_status(int *on_capture);
#endif /* __AVSYS_AUDIO_HANDLE_H__ */
