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
	AVSYS_AUDIO_EAR_SWITCH_MANUAL,
	AVSYS_AUDIO_EAR_SWITCH_AUTO_WITH_MUTE,
	AVSYS_AUDIO_EAR_SWITCH_AUTO_WITHOUT_MUTE,
};

struct avsys_audio_jack_event {
	struct timeval time;
	unsigned short type;
	unsigned short code;
	int value;
};

#define PATH_MASK_MAX	23
#define GAIN_MASK_MAX	30
/* sound path status bit */
#define PS_PATH_NONE				(0)
#define PS_AP_TO_SPK			(1 << 0)
#define PS_AP_TO_HEADSET		(1 << 1)
#define PS_AP_TO_RECV			(1 << 2)
#define PS_AP_TO_HDMI			(1 << 3)
#define PS_AP_TO_BT				(1 << 4)
#define PS_AP_TO_MODEM			(1 << 5)
#define PS_MODEM_TO_SPK			(1 << 6)
#define PS_MODEM_TO_HEADSET		(1 << 7)
#define PS_MODEM_TO_RECV		(1 << 8)
#define PS_MODEM_TO_BT			(1 << 9)
#define PS_MODEM_TO_AP			(1 << 10)
#define PS_FMRADIO_TO_SPK		(1 << 11)
#define PS_FMRADIO_TO_HEADSET	(1 << 12)
#define PS_MAINMIC_TO_AP		(1 << 13)
#define PS_MAINMIC_TO_MODEM		(1 << 14)
#define PS_SUBMIC_TO_AP			(1 << 15)
#define PS_SUBMIC_TO_MODEM		(1 << 16)
#define PS_STEREOMIC_TO_AP		(1 << 17)
#define PS_EARMIC_TO_AP			(1 << 18)
#define PS_EARMIC_TO_MODEM		(1 << 19)
#define PS_BTMIC_TO_AP			(1 << 20)
#define PS_BTMIC_TO_MODEM		(1 << 21)
#define PS_FMRADIO_TO_AP		(1 << 22)
#define PS_CODEC_DISABLE_ON_SUSPEND	(1 << 23)
#define PS_CP_TO_AP		(1 << PATH_MASK_MAX)


/* hw gain status enum */
#define GS_GAIN_NONE				(0)
#define GS_AP_TO_SPK				(1 << 0)
#define GS_AP_TO_SPK_CALLALERT		(1 << 1)
#define GS_AP_TO_HEADSET			(1 << 2)
#define GS_AP_TO_HEADSET_CALLALERT	(1 << 3)
#define GS_AP_TO_RECV				(1 << 4)
#define GS_AP_TO_HDMI				(1 << 5)
#define GS_AP_TO_BT					(1 << 6)
#define GS_AP_TO_MODEM				(1 << 7)
#define GS_MODEM_TO_SPK_VOICE		(1 << 8)
#define GS_MODEM_TO_HEADSET_VOICE	(1 << 9)
#define GS_MODEM_TO_RECV_VOICE		(1 << 10)
#define GS_MODEM_TO_BT_VOICE		(1 << 11)
#define GS_MODEM_TO_AP_VOICE		(1 << 12)
#define GS_MODEM_TO_SPK_VIDEO		(1 << 13)
#define GS_MODEM_TO_HEADSET_VIDEO	(1 << 14)
#define GS_MODEM_TO_RECV_VIDEO		(1 << 15)
#define GS_MODEM_TO_BT_VIDEO		(1 << 16)
#define GS_MODEM_TO_AP_VIDEO		(1 << 17)
#define GS_FMRADIO_TO_SPK			(1 << 18)
#define GS_FMRADIO_TO_HEADSET		(1 << 19)
#define GS_MAINMIC_TO_AP			(1 << 20)
#define GS_MAINMIC_TO_MODEM_VOICE	(1 << 21)
#define GS_SUBMIC_TO_AP				(1 << 22)
#define GS_SUBMIC_TO_MODEM_VOICE	(1 << 23)
#define GS_STEREOMIC_TO_AP			(1 << 24)
#define GS_EARMIC_TO_AP				(1 << 25)
#define GS_EARMIC_TO_MODEM_VOICE	(1 << 26)
#define GS_BTMIC_TO_AP				(1 << 27)
#define GS_BTMIC_TO_MODEM_VOICE		(1 << 28)
#define GS_FMRADIO_TO_AP			(1 << 29)
#define GS_CP_TO_AP					(1 << GAIN_MASK_MAX)

#define TYPE_EVENT_SWITCH			0x05
#define CODE_HEADPHONE_INSERT		0x02
#define CODE_MICROPHONE_INSERT		0x04
#define CODE_LINEOUT_INSERT			0x06
#define CODE_JACK_PHYSICAL_INSERT	0x07

#define PATH_FIXED_NONE                     (0x00000000)
#define PATH_FIXED_WITH_FMRADIO     (1 << PATH_FIXED_TYPE_FMRADIO)  /* 0x00000001 */
#define PATH_FIXED_WITH_CALL            (1 << PATH_FIXED_TYPE_CALL)         /* 0x00000002 */

enum avsys_audio_amp_t {
    AVSYS_AUDIO_AMP_OFF = 0,	/**< AMP OFF in pda out */
    AVSYS_AUDIO_AMP_ON,			/**< AMP ON in pda out */
    AVSYS_AUDIO_AMP_OFF_ALL,
};

enum path_fixed_type_t {
    PATH_FIXED_TYPE_FMRADIO = 0,
    PATH_FIXED_TYPE_CALL,
    PATH_FIXED_TYPE_MAX,
};

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

	gain_info_t backup_gain;
	path_info_t backup_path;

	gain_info_t pregain;
	gain_info_t reqgain;

	option_info_t option;

	/* path fixed information */
	int	path_fixed;
	pid_t path_fixed_pid[PATH_FIXED_TYPE_MAX];

	/* hw mute */
	int mute;

	/* For earphone control */
	int inserted;
	int ear_auto;

	/* for alsa scenario, aquila */
	gain_status_t gain_status;
	path_status_t path_status;

	gain_status_t p_gain_status;
	path_status_t p_path_status;

	int lvol_dev_type;
	int gain_debug_mode;

	/* For Lock debugging */
	pid_t pathlock_pid[AVSYS_AUDIO_LOCK_SLOT_MAX];

	/* system route policy */
	avsys_audio_route_policy_t route_policy;
	int a2dp_status;
	int  earpiece_on;
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
int avsys_audio_path_check_loud(bool *loud);
int avsys_audio_path_check_cp_audio(bool *cpaudio, bool *btpath);
int avsys_audio_path_set_single_ascn(char *str);

#endif /* __AVSYS_AUDIO_PATH_H__ */
