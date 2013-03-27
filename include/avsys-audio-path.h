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

#ifndef __AVSYS_AUDIO_PATH_H__
#define __AVSYS_AUDIO_PATH_H__

#include "avsys-audio.h"
#include "avsys-audio-handle.h"

#include <sys/types.h>
#include <stdbool.h>

enum {
    AVSYS_AUDIO_INSERTED_NONE = 0,
    AVSYS_AUDIO_INSERTED_3,
    AVSYS_AUDIO_INSERTED_4,
    AVSYS_AUDIO_INSERTED_AV
};

enum avsys_audio_playback_gain{
    AVSYS_AUDIO_PLAYBACK_GAIN_AP = 0,
    AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO,
    AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL,
    AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL,
    AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT,
    AVSYS_AUDIO_PLAYBACK_GAIN_MAX
};

enum avsys_audio_capture_gain{
    AVSYS_AUDIO_CAPTURE_GAIN_AP = 0,
    AVSYS_AUDIO_CAPTURE_GAIN_FMRADIO,
    AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL,
    AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL,
    AVSYS_AUDIO_CAPTURE_GAIN_MAX
};

enum avsys_audio_ear_ctrl {
	AVSYS_AUDIO_EAR_SWITCH_AUTO_WITH_MUTE = 1,
	AVSYS_AUDIO_EAR_SWITCH_AUTO_WITHOUT_MUTE,
};


#define TYPE_EVENT_SWITCH			0x05
#define CODE_HEADPHONE_INSERT		0x02
#define CODE_MICROPHONE_INSERT		0x04
#define CODE_LINEOUT_INSERT			0x06
#define CODE_JACK_PHYSICAL_INSERT	0x07

struct audio_route_info_t {
    int playback;
    int capture;
};

typedef struct audio_route_info_t gain_info_t;
typedef struct audio_route_info_t path_info_t;
typedef struct audio_route_info_t option_info_t;
typedef struct audio_route_info_t gain_status_t;
typedef struct audio_route_info_t path_status_t;

typedef struct {
	gain_info_t gain;
	path_info_t path;

	gain_info_t pregain;
	gain_info_t reqgain;

	option_info_t option;

	/* hw mute */
	int mute;

	/* For earphone control */
	int inserted;
	int ear_auto;

	int lvol_dev_type;
	bool control_aif_before_path_set;
	bool gain_debug_mode;

	/* For Lock debugging */
	pid_t pathlock_pid[AVSYS_AUDIO_LOCK_SLOT_MAX];
} avsys_audio_path_ex_info_t;

int avsys_audio_path_ex_init(void);
int avsys_audio_path_ex_fini(void);
int avsys_audio_path_ex_reset(int forced);
int avsys_audio_path_ex_dump(void);
int avsys_audio_path_ex_set_path(int gain, int out, int in, int option);
int avsys_audio_path_ex_get_path(int *gain, int *out, int *in, int *option);
int avsys_audio_path_manage_earjack(void);
int avsys_audio_path_ex_set_amp(const int onoff);
int avsys_audio_path_ex_set_mute(const int mute);
int avsys_audio_path_ex_get_mute(int  *mute);
int avsys_audio_path_set_volume(int handle);

int avsys_audio_path_earjack_init(int *init_type, int *outfd);
int avsys_audio_path_earjack_wait(int fd, int *current_type, int *new_type, int *is_auto_mute);
int avsys_audio_path_earjack_process(int new_type);
int avsys_audio_path_earjack_deinit(int fd);
int avsys_audio_path_earjack_unlock(void);

int avsys_audio_path_set_route_policy(avsys_audio_route_policy_t route);
int avsys_audio_path_get_route_policy(avsys_audio_route_policy_t *route);
int avsys_audio_path_check_cp_audio(bool *cpaudio, bool *btpath);
int avsys_audio_path_set_single_ascn(char *str);

#endif /* __AVSYS_AUDIO_PATH_H__ */
