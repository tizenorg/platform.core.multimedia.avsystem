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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <linux/input.h>
#include <iniparser.h>

#include "avsys-audio-shm.h"
#include "avsys-audio-sync.h"
#include "avsys-audio-path.h"
#include "avsys-audio-shm.h"
#include "avsys-debug.h"
#include "avsys-common.h"
#include "avsys-audio-handle.h"
#include "avsys-audio-logical-volume.h"
#include "avsys-audio-alsa.h"
#include "avsys-audio-ascenario.h"

#define EXPORT_API __attribute__((__visibility__("default")))

#define RET_IO_CTL_ERR_IF_FAIL(SECTION) { if(AVSYS_FAIL(SECTION)) { \
									avsys_error_r(AVAUDIO,"%s %d\n",__func__,__LINE__); \
									return AVSYS_STATE_ERR_IO_CONTROL; \
									} }

static int g_playback_gain_select_data[AVSYS_AUDIO_PLAYBACK_GAIN_MAX][AVSYS_AUDIO_PLAYBACK_GAIN_MAX] = {
		{	/* AVSYS_AUDIO_PLAYBACK_GAIN_AP */
				AVSYS_AUDIO_PLAYBACK_GAIN_AP,				/* AVSYS_AUDIO_PLAYBACK_GAIN_AP */
				AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO,			/* AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO */
				AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL,		/* AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL */
				AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL,		/* AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL */
				AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT,		/* AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT */
		},
		{	/* AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO */
				AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO,			/* AVSYS_AUDIO_PLAYBACK_GAIN_AP */
				AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO,			/* AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO */
				AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL,		/* AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL */
				AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL,		/* AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL */
				AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO,			/* AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT */
		},
		{	/* AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL */
				AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL,		/* AVSYS_AUDIO_PLAYBACK_GAIN_AP */
				AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL,		/* AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO */
				AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL,		/* AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL */
				AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL,		/* AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL */
				AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT,		/* AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT */
		},
		{	/* AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL */
				AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL,		/* AVSYS_AUDIO_PLAYBACK_GAIN_AP */
				AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL,		/* AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO */
				AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL,		/* AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL */
				AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL,		/* AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL */
				AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT,		/* AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT */
		},
		{	/* AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT */
				AVSYS_AUDIO_PLAYBACK_GAIN_AP,				/* AVSYS_AUDIO_PLAYBACK_GAIN_AP */
				AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO,			/* AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO */
				AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL,		/* AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL */
				AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL,		/* AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL */
				AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT,		/* AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT */
		},
};

static int g_capture_gain_select_data[AVSYS_AUDIO_CAPTURE_GAIN_MAX][AVSYS_AUDIO_CAPTURE_GAIN_MAX] = {
		{	/* AVSYS_AUDIO_CAPTURE_GAIN_AP */
				AVSYS_AUDIO_CAPTURE_GAIN_AP,			/* AVSYS_AUDIO_CAPTURE_GAIN_AP */
				AVSYS_AUDIO_CAPTURE_GAIN_FMRADIO,		/* AVSYS_AUDIO_CAPTURE_GAIN_FMRADIO */
				AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL,		/* AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL */
				AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL,		/* AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL */
		},
		{	/* AVSYS_AUDIO_CAPTURE_GAIN_FMRADIO */
				AVSYS_AUDIO_CAPTURE_GAIN_FMRADIO,		/* AVSYS_AUDIO_CAPTURE_GAIN_AP */
				AVSYS_AUDIO_CAPTURE_GAIN_FMRADIO,		/* AVSYS_AUDIO_CAPTURE_GAIN_FMRADIO */
				AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL,		/* AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL */
				AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL,		/* AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL */
		},
		{	/* AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL */
				AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL,		/* AVSYS_AUDIO_CAPTURE_GAIN_AP */
				AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL,		/* AVSYS_AUDIO_CAPTURE_GAIN_FMRADIO */
				AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL,		/* AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL */
				AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL,		/* AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL */
		},
		{	/* AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL */
				AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL,		/* AVSYS_AUDIO_CAPTURE_GAIN_AP */
				AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL,		/* AVSYS_AUDIO_CAPTURE_GAIN_FMRADIO */
				AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL,		/* AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL */
				AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL,		/* AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL */
		},
};


static int __avsys_audio_path_set_ascn_ap_playback(avsys_audio_path_ex_info_t *control);
static int __avsys_audio_path_set_ascn_voicecall(avsys_audio_path_ex_info_t *control);
static int __avsys_audio_path_set_ascn_videocall(avsys_audio_path_ex_info_t *control);
static int __avsys_audio_path_set_ascn_fmradio(avsys_audio_path_ex_info_t *control);
static int __avsys_audio_path_set_ascn_ap_capture(avsys_audio_path_ex_info_t *control);
static int __avsys_audio_path_set_hw_controls(avsys_audio_path_ex_info_t *control);
static int __avsys_audio_path_get_earjack_type(void);

#define AUDIOSYSTEM_CONF	"/opt/etc/audio_system.conf"
#define CONF_ITEM_COUNT	2
#define INPUT_DEV_MAX 20

#define EARJACK_EVENT_PATH	"/dev/input/event"

#define AVSYS_AUDIO_INI_DEFAULT_PATH "/usr/etc/mmfw_avsystem.ini"
//#define HAL
#ifdef HAL
#define AVSYS_AUDIO_DEFAULT_CONTROL_AIF_BEFORE_PATH_SET		0
#endif

#define AVSYS_AUDIO_DEFAULT_GAIN_DEBUG_MODE					0

static char *conf_string[] = {
	"headset_detection",
	"headset_node",
};

typedef struct {
	char headset_detection;
	char headset_node_number;
	bool control_aif_before_path_set;
	bool gain_debug_mode;
} avsys_audio_conf;

static int __load_conf(avsys_audio_conf *data)
{
	dictionary *dict = NULL;

#if defined(_MMFW_I386_ALL_SIMULATOR)
	if (data == NULL)
		return AVSYS_STATE_ERR_NULL_POINTER;

	data->headset_detection = 1;
	data->headset_node_number = 4;
#else
	FILE *fp = NULL;
	int i = 0;
	char buffer[64] = { 0, };
	char conf_data[CONF_ITEM_COUNT] = { 1, 0 };

	if (data == NULL)
		return AVSYS_STATE_ERR_NULL_POINTER;

	fp = fopen(AUDIOSYSTEM_CONF, "r");
	if (fp == NULL) {
		char filename[128] = { 0, };
		char readBuffer[32] = { 0, };
		char headset_find = 0;
		int num = 0, headset_num = -1;

		for (num = 0; num < INPUT_DEV_MAX; num++) {
			FILE *sfp = NULL;
			memset(filename, '\0', sizeof(filename));
			snprintf(filename, sizeof(filename), "/sys/class/input/input%01d/name", num);
			if (NULL == (sfp = fopen(filename, "r")))
				continue;
			memset(readBuffer, '\0', sizeof(readBuffer));
			if (NULL == fgets(readBuffer, sizeof(readBuffer) - 1, sfp)) {
				fclose(sfp);
				continue;
			}
			if (strstr(readBuffer, "Headset")) {
				headset_find = 1;
				headset_num = num;
			}

			fclose(sfp);
			if (headset_num != -1) {
				break;
			}
		}
		if (headset_num == -1)
			return AVSYS_STATE_ERR_INTERNAL;

		if (NULL == (fp = fopen(AUDIOSYSTEM_CONF, "w"))) {
			return AVSYS_STATE_ERR_INTERNAL;
		}

		fprintf(fp, "%s:1\n", conf_string[0]);
		fprintf(fp, "%s:%d\n", conf_string[1], headset_num);
		fflush(fp);
		fclose(fp);
		fp = NULL;
		sync();
		data->headset_detection = 1;
		data->headset_node_number = headset_num;
		return AVSYS_STATE_SUCCESS;
	}

	while (fgets(buffer, sizeof(buffer) - 1, fp) != NULL) {
		if ((strlen(buffer) < 3) || (buffer[0] == '#') || (buffer[0] == '!'))
			continue;
		if (buffer[strlen(buffer) - 1] == '\n')
			buffer[strlen(buffer) - 1] = '\0';
		for (i = 0; i < CONF_ITEM_COUNT; i++) {
			if (0 == strncmp(buffer, conf_string[i], strlen(conf_string[i]))) {
				char *ptr = NULL;
				if (NULL == (ptr = strstr(buffer, ":")))
					break;
				conf_data[i] = atoi(ptr + 1);
				avsys_warning(AVAUDIO, "%s[%d]\n", buffer, conf_data[i]);
			}
		}
	}
	fclose(fp);
	data->headset_detection = conf_data[0];
	data->headset_node_number = conf_data[1];
#endif

	/* first, try to load existing ini file */
	dict = iniparser_load(AVSYS_AUDIO_INI_DEFAULT_PATH);
	if (dict) { /* if dict is available */
#ifdef HAL
		data->control_aif_before_path_set = iniparser_getboolean(dict, "aif:control aif before path set", AVSYS_AUDIO_DEFAULT_CONTROL_AIF_BEFORE_PATH_SET);
#endif
		data->gain_debug_mode = iniparser_getboolean(dict, "debug:gain debug mode", AVSYS_AUDIO_DEFAULT_GAIN_DEBUG_MODE);

		/* free dict as we got our own structure */
		iniparser_freedict (dict);
	} else { /* if no file exists. create one with set of default values */
#ifdef HAL
		data->control_aif_before_path_set = AVSYS_AUDIO_DEFAULT_CONTROL_AIF_BEFORE_PATH_SET;
#endif
		data->gain_debug_mode = AVSYS_AUDIO_DEFAULT_GAIN_DEBUG_MODE;
	}

	return AVSYS_STATE_SUCCESS;
}

EXPORT_API
int avsys_audio_path_ex_init(void)
{
	avsys_audio_path_ex_info_t *control = NULL;
	avsys_audio_path_ex_info_t **temp = NULL;
	gain_info_t default_gain = { AVSYS_AUDIO_PLAYBACK_GAIN_AP, AVSYS_AUDIO_CAPTURE_GAIN_AP };
	path_info_t default_path = { AVSYS_AUDIO_PATH_EX_SPK, AVSYS_AUDIO_PATH_EX_MIC };
	option_info_t default_option = { AVSYS_AUDIO_PATH_OPTION_NONE, AVSYS_AUDIO_PATH_OPTION_NONE };
	int index = 0;
	int err = AVSYS_STATE_SUCCESS;
	avsys_audio_conf conf = { 0, };

	/* Check root user */
	err = avsys_check_root_privilege();
	if (AVSYS_FAIL(err)) {
		return err;
	}

	temp = &control;
	avsys_assert(AVSYS_SUCCESS(avsys_audio_create_sync(AVSYS_AUDIO_SYNC_IDEN_PATH)));
	avsys_assert(AVSYS_SUCCESS(avsys_audio_create_shm(AVSYS_AUDIO_SHM_IDEN_PATH)));

	avsys_assert(AVSYS_SUCCESS(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_PATH, (void **)temp)));
	if (control == NULL)
		return AVSYS_STATE_ERR_NULL_POINTER;

	/* init values */
	control->pregain = default_gain;
	control->gain = default_gain;
	control->reqgain = default_gain;
	control->path = default_path;
	control->option = default_option;

	control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
	control->inserted = AVSYS_AUDIO_INSERTED_NONE;

	if (AVSYS_FAIL(__load_conf(&conf)))
		avsys_error_r(AVAUDIO, "Can not load audio system configuration file\n");

	if (conf.headset_detection) {
		control->inserted = __avsys_audio_path_get_earjack_type();
		if (control->inserted == AVSYS_AUDIO_INSERTED_NONE)
			control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
		else
			control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET;
	} else {
		avsys_warning(AVAUDIO, "Ignore headset detection. Use speaker device\n");
		control->inserted = AVSYS_AUDIO_INSERTED_NONE;
		control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
	}

	avsys_error_r(AVAUDIO, "Earjack init value is %d\n", control->inserted);

	control->control_aif_before_path_set = conf.control_aif_before_path_set;
	control->gain_debug_mode = conf.gain_debug_mode;

	control->mute = AVSYS_AUDIO_UNMUTE;

	index = 0;
	do {
		control->pathlock_pid[index] = -1;
		index++;
	} while (index < AVSYS_AUDIO_LOCK_SLOT_MAX);

	/* call path control */
	err = __avsys_audio_path_set_ascn_ap_playback(control);
	if (AVSYS_SUCCESS(err))
		err = __avsys_audio_path_set_hw_controls(control);

	if (AVSYS_SUCCESS(err))
		err = __avsys_audio_path_set_ascn_ap_capture(control);

	return err;
}

EXPORT_API
int avsys_audio_path_ex_fini(void)
{
	if (AVSYS_FAIL(avsys_audio_remove_shm(AVSYS_AUDIO_SHM_IDEN_PATH))) {
		avsys_error_r(AVAUDIO, "avsys_audio_remove_shm() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}
	if (AVSYS_FAIL(avsys_audio_remove_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
		avsys_error_r(AVAUDIO, "avsys_audio_remove_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}
	return AVSYS_STATE_SUCCESS;
}

EXPORT_API
int avsys_audio_path_ex_reset(int forced)
{
	avsys_audio_path_ex_info_t *control = NULL;
	avsys_audio_path_ex_info_t **temp = NULL;
	gain_info_t default_gain = { AVSYS_AUDIO_PLAYBACK_GAIN_AP, AVSYS_AUDIO_CAPTURE_GAIN_AP };
	path_info_t default_path = { AVSYS_AUDIO_PATH_EX_SPK, AVSYS_AUDIO_PATH_EX_MIC };
	option_info_t default_option = { AVSYS_AUDIO_PATH_OPTION_NONE, AVSYS_AUDIO_PATH_OPTION_NONE };
	int index = 0;
	int err = AVSYS_STATE_SUCCESS;
	int backup_debug = 0;
	avsys_audio_conf conf = { 0, };

	/* Check root user */
	err = avsys_check_root_privilege();
	if (AVSYS_FAIL(err)) {
		return err;
	}

	temp = &control;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_PATH, (void **)temp))) {
		avsys_error_r(AVAUDIO, "avsys_audio_get_shm() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}
	if (control == NULL)
		return AVSYS_STATE_ERR_NULL_POINTER;

	if (AVSYS_FAIL(avsys_audio_lock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
		avsys_error_r(AVAUDIO, "avsys_audio_lock_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	/* init values */
	control->pregain = default_gain;
	control->gain = default_gain;
	control->reqgain = default_gain;
	control->path = default_path;
	control->option = default_option;

	control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
	control->inserted = AVSYS_AUDIO_INSERTED_NONE;

	if (AVSYS_FAIL(__load_conf(&conf)))
		avsys_error_r(AVAUDIO, "Can not load audio system configuration file\n");

	if (conf.headset_detection) {
		control->inserted = __avsys_audio_path_get_earjack_type();
		if (control->inserted == AVSYS_AUDIO_INSERTED_NONE)
			control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
		else {
			control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET;
			control->path.playback = AVSYS_AUDIO_PATH_EX_HEADSET;

			if (control->inserted == AVSYS_AUDIO_INSERTED_4)
				control->path.capture = AVSYS_AUDIO_PATH_EX_HEADSETMIC;
		}
	} else {
		avsys_warning(AVAUDIO, "Ignore headset detection. Use speaker device\n");
		control->inserted = AVSYS_AUDIO_INSERTED_NONE;
		control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
	}

	avsys_error_r(AVAUDIO, "Earjack init value is %d\n", control->inserted);

	control->control_aif_before_path_set = conf.control_aif_before_path_set;
	control->gain_debug_mode = conf.gain_debug_mode;

	control->mute = AVSYS_AUDIO_UNMUTE;

	index = 0;
	do {
		control->pathlock_pid[index] = -1;
		index++;
	} while (index < AVSYS_AUDIO_LOCK_SLOT_MAX);

	if (forced) {
		backup_debug = control->gain_debug_mode;
		control->gain_debug_mode = 1;
	}
	/* call path control */
	err = __avsys_audio_path_set_ascn_ap_playback(control);
	if (AVSYS_SUCCESS(err))
		err = __avsys_audio_path_set_hw_controls(control);

	if (AVSYS_SUCCESS(err))
		err = __avsys_audio_path_set_ascn_ap_capture(control);

	if (forced) {
		control->gain_debug_mode = backup_debug;
	}

	if (AVSYS_FAIL(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
		avsys_error_r(AVAUDIO, "avsys_audio_unlock_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	return err;
}

EXPORT_API
int avsys_audio_path_ex_dump(void)
{
	avsys_audio_path_ex_info_t *control = NULL;
	avsys_audio_path_ex_info_t **temp = NULL;
	const static char *str_earType[] = { "None", "EarOnly", "EarMic", "TVout" };
	const static char *str_yn[] = { "NO", "YES" };
	const static char *str_ear[] = { "MANUAL", "AUTO_MUTE", "AUTO_NOMUTE" };
	const static char *str_out[AVSYS_AUDIO_PATH_EX_OUTMAX] = {
		"NONE", "SPK", "RECV", "HEADSET", "BTHEADSET", "A2DP", "HANSFREE", "HDMI", "DOCK", "USBAUDIO"
	};
	const static char *str_in[AVSYS_AUDIO_PATH_EX_INMAX] = {
		"NONE", "MIC", "HEADMIC", "BTMIC", "FMINPUT", "HANSFREEMIC"
	};
	const static char *str_route[AVSYS_AUDIO_ROUTE_POLICY_MAX] = {
		"DEFAULT", "IGN_A2DP", "HANDSET"
	};
	/*
	const static char *str_gain[AVSYS_AUDIO_GAIN_EX_MAX] = {
		"KEYTONE", "RINGTONE", "ALARMTONE", "CALLTONE", "AUDIOPLAYER", "VIDEOPLAYER",
		"VOICECALL", "VIDEOCALL", "FMRADIO", "VOICEREC", "CAMCORDER", "CAMERA", "GAME"};
	*/
	const static char *str_playback_gain[AVSYS_AUDIO_PLAYBACK_GAIN_MAX] = {
		"AP", "FMRADIO", "VOICECALL", "VIDEOCALL", "CALLALERT",
	};
	const static char *str_capture_gain[AVSYS_AUDIO_CAPTURE_GAIN_MAX] = {
		"AP", "FMRADIO", "VOICECALL", "VIDEOCALL",
	};

	int index = 0;

	temp = &control;

	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_PATH, (void **)temp))) {
		avsys_error_r(AVAUDIO, "avsys_audio_get_shm() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}
	if (control == NULL)
		return AVSYS_STATE_ERR_NULL_POINTER;

	fprintf(stdout, "======================================================================\n");
	fprintf(stdout, "               Avsystem Audio Path Control Information                \n");
	fprintf(stdout, "======================================================================\n");
#if defined(_MMFW_I386_ALL_SIMULATOR)
	fprintf(stdout, " In simulator, follow informations don`t have means.\n");
#endif

	fprintf(stdout, " GAIN                          : P (%-s / %-s) - R (%-s / %-s) - C (%-s / %-s)\n",
			str_playback_gain[control->pregain.playback], str_capture_gain[control->pregain.capture],
			str_playback_gain[control->reqgain.playback], str_capture_gain[control->reqgain.capture],
			str_playback_gain[control->gain.playback], str_capture_gain[control->gain.capture]);
	fprintf(stdout, " Current Out / In              : %-s / %-s\n", str_out[control->path.playback], str_in[control->path.capture] );
	fprintf(stdout, " Gain debug mode               : 0x%-x\n", control->gain_debug_mode);
	fprintf(stdout, " Auto EarJack Control          : %-s\n", str_ear[control->ear_auto]);
	fprintf(stdout, " Physical Earjack? [type]      : %-s [%-s]\n", str_yn[control->inserted != AVSYS_AUDIO_INSERTED_NONE], str_earType[control->inserted]);
	fprintf(stdout, " Mute status                   : %d\n", control->mute);

	index = 0;
	do {
		if (control->pathlock_pid[index] != -1)
			fprintf(stdout, " Path sync lock required PIDs   : %d\n", control->pathlock_pid[index]);
		index++;
	} while (index < AVSYS_AUDIO_LOCK_SLOT_MAX);
	fprintf(stdout, " Option Dual out               : %-s\n", str_yn[(control->option.playback & AVSYS_AUDIO_PATH_OPTION_DUAL_OUT) ? 1 : 0]);
	fprintf(stdout, " Option Forced                 : %-s\n", str_yn[(control->option.playback & AVSYS_AUDIO_PATH_OPTION_FORCED) ? 1 : 0]);

	return AVSYS_STATE_SUCCESS;
}

static int __avsys_audio_path_get_earjack_type (void)
{
	int fd = 0;
	char readval = AVSYS_AUDIO_INSERTED_NONE;
	fd = open("/sys/devices/platform/jack/earjack_online", O_RDONLY);

	if (fd == -1) {
		avsys_error_r(AVAUDIO, "Can not get initial jack type\n");
		return AVSYS_AUDIO_INSERTED_NONE;
	}
	read(fd, &readval, sizeof(readval));
	switch (readval) {
	case '0':
		readval = AVSYS_AUDIO_INSERTED_NONE;
		break;
	case '1':
		readval = AVSYS_AUDIO_INSERTED_3;
		break;
	case '3':
		readval = AVSYS_AUDIO_INSERTED_4;
		break;
	case '8':
		readval = AVSYS_AUDIO_INSERTED_AV;
		break;
	case '2':
		if (1 == read(fd, &readval, sizeof(readval))) {
			if (readval == '0') {
				readval = AVSYS_AUDIO_INSERTED_AV;
			} else {
				avsys_error(AVAUDIO, "Unknown jack type value...2%d\n", readval);
				readval = AVSYS_AUDIO_INSERTED_NONE;
			}
		} else {
			avsys_error(AVAUDIO, "jack type read error...\n");
			readval = AVSYS_AUDIO_INSERTED_NONE;
		}
		break;
	default:
		avsys_error(AVAUDIO, "jack type unknown value...%c\n", readval);
		readval = AVSYS_AUDIO_INSERTED_NONE;
		break;
	}

	close(fd);
	return readval;
}

int avsys_audio_path_earjack_init(int *init_type, int *outfd)
{
#if !defined(_MMFW_I386_ALL_SIMULATOR)
	char eventnode_filename[32] = { 0, };
	int fd = 0;
	avsys_audio_conf conf = { 0, };

	if (outfd == NULL || init_type == NULL) {
		avsys_error(AVAUDIO, "input parameter is null\n");
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	if (AVSYS_FAIL(__load_conf(&conf))) {
		avsys_error_r(AVAUDIO, "Can not load audio system configuration file\n");
	}

	if (!conf.headset_detection) {
		avsys_error(AVAUDIO, "Earjack control daemon will be closed by user option...\n");
		return AVSYS_STATE_ERR_DEVICE_NOT_SUPPORT;
	}

	snprintf(eventnode_filename, sizeof(eventnode_filename), "%s%01d", EARJACK_EVENT_PATH, conf.headset_node_number);

	fd = open(eventnode_filename, O_RDONLY);
	if (fd == -1) {
		avsys_error_r(AVAUDIO, "Device file open error\n");
		return AVSYS_STATE_ERR_INTERNAL;
	} else {
		avsys_audio_path_ex_info_t *control = NULL;
		avsys_audio_path_ex_info_t **temp = NULL;
		void *vol_data = NULL;
		void *handle_data = NULL;
		temp = &control;

		if(AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, &vol_data))) {
			avsys_error(AVAUDIO,"attach logical volume shared memory failed\n");
			return AVSYS_STATE_ERR_ALLOCATION;
		}
		if(AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_HANDLE, &handle_data))) {
			avsys_error(AVAUDIO,"attach handle shared memory failed\n");
			return AVSYS_STATE_ERR_ALLOCATION;
		}
		if(AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_PATH, (void**)temp))) {
			avsys_error_r(AVAUDIO,"avsys_audio_get_shm() failed in %s\n", __func__);
			return AVSYS_STATE_ERR_INTERNAL;
		}

		*init_type = control->inserted;
		*outfd = fd;
		return AVSYS_STATE_SUCCESS;
	}
#else
	return AVSYS_STATE_ERR_DEVICE_NOT_SUPPORT;
#endif
}

int avsys_audio_path_earjack_wait(int fd, int *current_type, int *new_type, int *is_auto_mute)
{
#if !defined(_MMFW_I386_ALL_SIMULATOR)
	fd_set set;
	int readtemp;
	int select_ret = 0;
	struct input_event jevent;
	int res = AVSYS_STATE_SUCCESS;
	int cur_type = -1;

	if (new_type == NULL || is_auto_mute == NULL)
		return AVSYS_STATE_ERR_NULL_POINTER;

	FD_ZERO(&set);
	FD_SET(fd, &set);

	cur_type = *current_type;

	select_ret = select(fd + 1, &set, NULL, NULL, NULL);
	avsys_info(AVAUDIO, "SELECT returns......\n");

	if (select_ret != 1) {
		if (select_ret == 0) {
			avsys_error_r(AVAUDIO, "Earjack timeout in autocontrol\n");
		} else if (select_ret == -1) {
			avsys_error_r(AVAUDIO, "Earjack detect unknown error: %d\n", errno);
		}
		return AVSYS_STATE_WAR_INVALID_VALUE;
	}
#ifdef EARJACK_LOCK		/* currently this is disabled to avoid semapore value increase */
	if (AVSYS_FAIL(avsys_audio_lock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
		avsys_error_r(AVAUDIO, "avsys_audio_lock_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}
#endif
	if (read(fd, &jevent, sizeof(jevent)) < 0) {
		avsys_error(AVAUDIO, "read fd failed with 0x%x\n", errno);
		return AVSYS_STATE_WAR_INVALID_MODE;
	}
	avsys_info(AVAUDIO, "*** JEVENT : code=%d, value=%d\n", jevent.code, jevent.value);
	if (jevent.type != TYPE_EVENT_SWITCH) {
		avsys_info(AVAUDIO, "Not a switch event\n");
		return AVSYS_STATE_WAR_INVALID_MODE;
	}

	switch (jevent.code) {
	case CODE_HEADPHONE_INSERT:
	case CODE_LINEOUT_INSERT:
	case CODE_JACK_PHYSICAL_INSERT:
		if (jevent.value == 1) {
			readtemp = __avsys_audio_path_get_earjack_type();
		} else {
			readtemp = 0;
		}
		break;
	default:
		readtemp = cur_type; /* same value */
		break;
	}

	*new_type = readtemp;

	avsys_audio_path_ex_info_t *control = NULL;
	avsys_audio_path_ex_info_t **temp = NULL;
	temp = &control;

	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_PATH, (void **)temp))) {
		avsys_error_r(AVAUDIO, "avsys_audio_get_shm() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	avsys_info(AVAUDIO, "control->ear_auto = %d\n", control->ear_auto);
	if (control->ear_auto == AVSYS_AUDIO_EAR_SWITCH_AUTO_WITH_MUTE) {
		*current_type = control->inserted;
		*is_auto_mute = 1;
		res = AVSYS_STATE_SUCCESS;
	} else if (control->ear_auto == AVSYS_AUDIO_EAR_SWITCH_AUTO_WITHOUT_MUTE) {
		*current_type = control->inserted;
		*is_auto_mute = 0;
		res = AVSYS_STATE_SUCCESS;
	}

	return res;

#else
	return AVSYS_STATE_ERR_DEVICE_NOT_SUPPORT;
#endif
}

int avsys_audio_path_earjack_process(int new_type)
{
#if !defined(_MMFW_I386_ALL_SIMULATOR)
	avsys_audio_path_ex_info_t *control = NULL;
	avsys_audio_path_ex_info_t **temp = NULL;
	int err = AVSYS_STATE_SUCCESS;
	temp = &control;

	avsys_info(AVAUDIO, "new_type = %d\n", new_type);
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_PATH, (void **)temp))) {
		avsys_error_r(AVAUDIO, "avsys_audio_get_shm() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	control->inserted = new_type;

	return err;
#else
	return AVSYS_STATE_ERR_DEVICE_NOT_SUPPORT;
#endif
}

int avsys_audio_path_earjack_deinit(int fd)
{
#if !defined(_MMFW_I386_ALL_SIMULATOR)
	close(fd);
	return AVSYS_STATE_SUCCESS;
#else
	return AVSYS_STATE_ERR_DEVICE_NOT_SUPPORT;
#endif
}

int avsys_audio_path_earjack_unlock()
{
#ifdef EARJACK_LOCK		/* currently this is disabled to avoid semapore value increase */
	if (AVSYS_FAIL(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
		avsys_error_r(AVAUDIO, "avsys_audio_unlock_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}
#endif
	return AVSYS_STATE_SUCCESS;
};

#define DO_IF_VALID(c, p) { if(c > -1) p; }
#define DO_IF_INVALID(c, p) { if(c == -1) p; }
#define CHECK_VALID(c) (c>-1 ? 1 : 0)

#ifdef HAL
enum {
	CMD_DEVICE_NONE = 0,
	CMD_DEVICE_OPEN,
	CMD_DEVICE_CLOSE,
	CMD_DEVICE_MAX
};

avsys_audio_alsa_aif_handle_t *g_hAIF[AIF_DEVICE_MAX] = { NULL, NULL, NULL, NULL, NULL };
char *strAIF[AIF_DEVICE_MAX] = { "AIF2 Capture", "AIF2 Playback", "AIF3 Capture", "AIF3 Playback", "AIF4 Playback" };

#define SET_AIF(index)	\
do {																\
	if(g_hAIF[index]) {												\
		avsys_warning(AVAUDIO,#index" device already opened\n");	\
		AIF_control[index] = CMD_DEVICE_NONE;						\
	} else {														\
		AIF_control[index] = CMD_DEVICE_OPEN;						\
	}																\
} while (0)

static int __avsys_open_aif(char AIF_control[])
{
	int iAIF = 0;
	int err = AVSYS_STATE_SUCCESS;

	for (iAIF = 0; iAIF < AIF_DEVICE_MAX; iAIF++) {
		/* check command */
		if (AIF_control[iAIF] != CMD_DEVICE_OPEN)
			continue;

		/* check handle allocation */
		if (g_hAIF[iAIF]) {
			avsys_warning(AVAUDIO, "Oops! Free %s device handle first", strAIF[iAIF]);
			free(g_hAIF[iAIF]);
			g_hAIF[iAIF] = NULL;
		}
		/* memory allocation for handle */
		avsys_warning(AVAUDIO, "%s handle alloc", strAIF[iAIF]);
		g_hAIF[iAIF] = calloc(sizeof(avsys_audio_handle_t), 1);
		if (!g_hAIF[iAIF]) {
			avsys_error_r(AVAUDIO, "Can not alloc memory for %s device handle", strAIF[iAIF]);
			err = AVSYS_STATE_ERR_ALLOCATION;
			continue;
		}

		if (AVSYS_FAIL(avsys_audio_alsa_open_AIF_device(iAIF, g_hAIF[iAIF]))) {
			avsys_error_r(AVAUDIO, "open %s device failed\n", strAIF[iAIF]);
			err = AVSYS_STATE_ERR_INVALID_HANDLE;
		} else {
			avsys_warning(AVAUDIO, "open %s device success\n", strAIF[iAIF]);
			if (AVSYS_FAIL(avsys_audio_alsa_set_AIF_params(g_hAIF[iAIF]))) {
				avsys_error_r(AVAUDIO, "%s device set parameter failed\n", strAIF[iAIF]);
				err = AVSYS_STATE_ERR_INVALID_PARAMETER;
			} else {
				avsys_warning(AVAUDIO, "%s device set parameter success\n", strAIF[iAIF]);
			}
		}
	}
	return err;
}

static void __avsys_close_aif ()
{
	int iAIF = 0;

	for (iAIF = 0; iAIF < AIF_DEVICE_MAX; iAIF++) {
		if (g_hAIF[iAIF]) {
			avsys_info(AVAUDIO, "close device :: %s\n", strAIF[iAIF]);
			if (AVSYS_FAIL(avsys_audio_alsa_close_AIF_device(g_hAIF[iAIF]))) {
				avsys_error_r(AVAUDIO, "close %s device failed\n", strAIF[iAIF]);
			}
			free(g_hAIF[iAIF]);
			g_hAIF[iAIF] = NULL;
			avsys_warning(AVAUDIO, "%s device handle free\n", strAIF[iAIF]);
		} else {
			avsys_info(AVAUDIO, "skip closing device :: %s\n", strAIF[iAIF]);
		}
	}
}
#endif

static int __avsys_audio_release_path (gain_info_t local_gain, avsys_audio_path_ex_info_t *control)
{
	int err = AVSYS_STATE_SUCCESS;
#ifdef HAL
	int iAIF = 0;
	bool close_aif_later = false;
#endif

	avsys_warning(AVAUDIO, "Release path for %d %d\n", local_gain.playback, local_gain.capture);

	switch (local_gain.playback) {
	case AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL:
	case AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL:
#ifdef HAL
		if (!control->control_aif_before_path_set) {
			__avsys_close_aif();
		} else {
			close_aif_later = true;
		}
#endif

		if (AVSYS_FAIL(avsys_audio_ascn_single_set(ASCN_CODEC_DISABLE_ON_SUSPEND))) {
			avsys_error_r(AVAUDIO, "[%s] failed to set codec_disable_on_suspend\n", __func__);
		}
		break;

	case AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO:
		/* TODO: Reset & Codec disable on suspend script */
		if (AVSYS_FAIL(avsys_audio_ascn_single_set(ASCN_CODEC_DISABLE_ON_SUSPEND))) {
			avsys_error_r(AVAUDIO, "[%s] failed to set codec_disable_on_suspend\n", __func__);
		}

		if (AVSYS_FAIL(avsys_audio_ascn_single_set(ASCN_STR_RESET_CAPTURE))) {
			avsys_error_r(AVAUDIO, "[%s] failed to set reset\n", __func__);
		}

		break;

	default:
		avsys_warning(AVAUDIO, "unexpected path release\n");
		break;
	}

	avsys_warning(AVAUDIO, "Path Release to default condition....\n");
	control->gain.playback = AVSYS_AUDIO_PLAYBACK_GAIN_AP;
	control->gain.capture = AVSYS_AUDIO_CAPTURE_GAIN_AP;
	control->path.playback = AVSYS_AUDIO_PATH_EX_SPK;
	control->path.capture = AVSYS_AUDIO_PATH_EX_MIC;
	if (control->inserted != AVSYS_AUDIO_INSERTED_NONE) {
		control->path.playback = AVSYS_AUDIO_PATH_EX_HEADSET;
		if (control->inserted == AVSYS_AUDIO_INSERTED_4)
			control->path.capture = AVSYS_AUDIO_PATH_EX_HEADSETMIC;
	}

	/* Playback */
	err = __avsys_audio_path_set_ascn_ap_playback(control);
	if (AVSYS_SUCCESS(err)) {
		err = __avsys_audio_path_set_hw_controls(control);
		if (AVSYS_FAIL(err)) {
			avsys_error(AVAUDIO, "Update logical volume failure\n");
		}
	} else {
		avsys_error(AVAUDIO, "Set ap playback failure\n");
	}

	/* Capture */
	err = __avsys_audio_path_set_ascn_ap_capture(control);
	if (AVSYS_FAIL(err)) {
		avsys_error(AVAUDIO, "Set ap capture failure\n");
	}

#ifdef HAL
	if (close_aif_later == true) {
		__avsys_close_aif();
	}
#endif

	return err;
}

int avsys_audio_path_earjack_get_type()
{
	avsys_audio_path_ex_info_t *control = NULL;
	avsys_audio_path_ex_info_t **temp = NULL;
	int err = AVSYS_STATE_SUCCESS;
	int ret = 0;

	temp = &control;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_PATH, (void **)temp))) {
		avsys_error_r(AVAUDIO, "avsys_audio_get_shm() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}
	if (control == NULL)
		return AVSYS_STATE_ERR_NULL_POINTER;

	if (AVSYS_FAIL(avsys_audio_lock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
		avsys_error_r(AVAUDIO, "avsys_audio_lock_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	ret = control->inserted;

	if (AVSYS_FAIL(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
		avsys_error_r(AVAUDIO, "avsys_audio_unlock_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	return ret;
}

int avsys_audio_path_ex_set_path(int gain, int out, int in, int option)
{
	avsys_audio_path_ex_info_t *control = NULL;
	avsys_audio_path_ex_info_t **temp = NULL;
	gain_info_t local_gain = { -1, -1 };
	gain_info_t req_gain = { -1, -1 };
	pid_t current_pid;
	int err = AVSYS_STATE_SUCCESS;
	char req_release_path = 0;
#ifdef HAL
	char AIF_control[AIF_DEVICE_MAX] = { CMD_DEVICE_NONE, CMD_DEVICE_NONE, CMD_DEVICE_NONE, CMD_DEVICE_NONE, CMD_DEVICE_NONE };
	int iAIF = 0;
#endif

	avsys_warning(AVAUDIO, "=================== [Input Param] gain %d, out %d, in %d, opt 0x%x ====================\n", gain, out, in, option);

	/* Determine REQUESTs */
	switch (gain) {
	case AVSYS_AUDIO_GAIN_EX_KEYTONE:
	case AVSYS_AUDIO_GAIN_EX_ALARMTONE:
	case AVSYS_AUDIO_GAIN_EX_AUDIOPLAYER:
	case AVSYS_AUDIO_GAIN_EX_VIDEOPLAYER:
	case AVSYS_AUDIO_GAIN_EX_CAMERA:
	case AVSYS_AUDIO_GAIN_EX_GAME:
		req_gain.playback = AVSYS_AUDIO_PLAYBACK_GAIN_AP;
		req_gain.capture = AVSYS_AUDIO_CAPTURE_GAIN_AP;
		break;

	case AVSYS_AUDIO_GAIN_EX_RINGTONE:
	case AVSYS_AUDIO_GAIN_EX_CALLTONE:
		req_gain.playback = AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT;
		break;

	case AVSYS_AUDIO_GAIN_EX_VOICEREC:
	case AVSYS_AUDIO_GAIN_EX_CAMCORDER:
		req_gain.capture = AVSYS_AUDIO_CAPTURE_GAIN_AP;
		break;

	case AVSYS_AUDIO_GAIN_EX_VOICECALL:
		req_gain.playback = AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL;
		req_gain.capture = AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL;
		if (out == AVSYS_AUDIO_PATH_EX_NONE && in == AVSYS_AUDIO_PATH_EX_NONE)
			req_release_path = 1;
		break;

	case AVSYS_AUDIO_GAIN_EX_VIDEOCALL:
		req_gain.playback = AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL;
		req_gain.capture = AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL;
		if (out == AVSYS_AUDIO_PATH_EX_NONE && in == AVSYS_AUDIO_PATH_EX_NONE)
			req_release_path = 1;
		break;

	case AVSYS_AUDIO_GAIN_EX_FMRADIO:
		req_gain.playback = AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO;
		req_gain.capture = AVSYS_AUDIO_CAPTURE_GAIN_FMRADIO;
		if (out == AVSYS_AUDIO_PATH_EX_NONE && in == AVSYS_AUDIO_PATH_EX_NONE)
			req_release_path = 1;
		break;
	}

	/* Get avsys shared memeory */
	temp = &control;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_PATH, (void **)temp))) {
		avsys_error_r(AVAUDIO, "avsys_audio_get_shm() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}
	/* LOCK */
	if (AVSYS_FAIL(avsys_audio_lock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
		avsys_error_r(AVAUDIO, "avsys_audio_lock_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	current_pid = getpid(); /* moved from below */

	/* Check FORCED option */
	DO_IF_VALID(req_gain.playback, local_gain.playback = g_playback_gain_select_data[control->gain.playback][req_gain.playback])
	DO_IF_VALID(req_gain.capture, local_gain.capture = g_capture_gain_select_data[control->gain.capture][req_gain.capture])

	avsys_info(AVAUDIO, "Gain : req(%d,%d)  local(%d,%d)\n", req_gain.playback, req_gain.capture, local_gain.playback, local_gain.capture);

	/* forced gain setting when path fixed by dead process */
	if (req_gain.playback != local_gain.playback) {
		local_gain.playback = req_gain.playback;
	}
	if (req_gain.capture != local_gain.capture) {
		local_gain.capture = req_gain.capture;
	}

	/* overwrite local_gain with current gain if it is simplex sound path */
	DO_IF_INVALID(local_gain.playback, local_gain.playback = control->gain.playback)
	DO_IF_INVALID(local_gain.capture, local_gain.capture = control->gain.capture)
	control->pregain = control->gain;
	control->gain = local_gain;

	DO_IF_VALID(req_gain.playback, control->reqgain.playback = req_gain.playback)
	DO_IF_VALID(req_gain.capture, control->reqgain.capture = req_gain.capture)
	DO_IF_VALID(local_gain.playback, control->option.playback = option)
	DO_IF_VALID(local_gain.capture, control->option.capture = option)

	/* Check for Release PATH */
	if (req_release_path && (req_gain.playback == local_gain.playback) && (req_gain.capture == local_gain.capture)) {
		avsys_warning(AVAUDIO,"Release path for %d %d\n", local_gain.playback, local_gain.capture);
		err = __avsys_audio_release_path(local_gain, control);
		goto FINISHED;
	}

	if (CHECK_VALID(req_gain.playback)) {
		control->path.playback = out;

		switch (local_gain.playback) {
			case AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL:
			case AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT:
#ifdef HAL
				if (control->path.playback == AVSYS_AUDIO_PATH_EX_BTHEADSET) {
					SET_AIF(AIF3_PLAYBACK);
					SET_AIF(AIF3_CAPTURE);
				}
#endif
				break;

			case AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL:
#ifdef HAL
				SET_AIF(AIF2_PLAYBACK);
				if (control->path.playback == AVSYS_AUDIO_PATH_EX_BTHEADSET) {
					SET_AIF(AIF3_PLAYBACK);
				}
#endif
				break;

			case AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO:
#ifdef HAL
				SET_AIF(AIF4_PLAYBACK);
#endif
				break;
		}
	}

	if (CHECK_VALID(req_gain.capture)) {
			control->path.capture = in;

			switch (local_gain.capture) {
			case AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL:
#ifdef HAL
				if (control->path.capture == AVSYS_AUDIO_PATH_EX_BTMIC) {
					SET_AIF(AIF3_CAPTURE);
				}
#endif
				break;

			case AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL:
#ifdef HAL
				SET_AIF(AIF2_CAPTURE);
				if (control->path.capture == AVSYS_AUDIO_PATH_EX_BTMIC) {
					SET_AIF(AIF3_CAPTURE);
				}
#endif
				break;

			case AVSYS_AUDIO_CAPTURE_GAIN_FMRADIO:
				break;
			}
	}
#ifdef HAL
	/* Open AIFs */
	if (control->control_aif_before_path_set) {
		err = __avsys_open_aif(AIF_control);
	}
#endif

	/* Do ALSA scenario control based on gain */
	/* Playback */
	if (local_gain.playback == AVSYS_AUDIO_PLAYBACK_GAIN_AP) {
		avsys_warning(AVAUDIO, "playback gain : ap\n");
		err = __avsys_audio_path_set_ascn_ap_playback(control);
		if (AVSYS_SUCCESS(err)) {
			err = __avsys_audio_path_set_hw_controls(control);
		}
	} else if(local_gain.playback == AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT) {
		avsys_warning(AVAUDIO,"playback gain : callalert\n");
		err = __avsys_audio_path_set_ascn_ap_playback(control);
		if (AVSYS_SUCCESS(err)) {
			err = __avsys_audio_path_set_hw_controls(control);
		}
	} else if (local_gain.playback == AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO &&
			   local_gain.capture == AVSYS_AUDIO_CAPTURE_GAIN_FMRADIO) {
		avsys_warning(AVAUDIO, "fmradio gain\n");
		err = __avsys_audio_path_set_ascn_fmradio(control);
	} else if (local_gain.playback == AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL &&
			   local_gain.capture == AVSYS_AUDIO_CAPTURE_GAIN_VOICECALL) {
		avsys_warning(AVAUDIO, "voicecall gain\n");
		err = __avsys_audio_path_set_ascn_voicecall(control);
	} else if (local_gain.playback == AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL &&
			   local_gain.capture == AVSYS_AUDIO_CAPTURE_GAIN_VIDEOCALL) {
		avsys_warning(AVAUDIO, "videocall gain\n");
		err = __avsys_audio_path_set_ascn_videocall(control);
	}
	/* Capture */
	if (local_gain.capture == AVSYS_AUDIO_CAPTURE_GAIN_AP) {
		avsys_warning(AVAUDIO, "capture gain : ap\n");
		err = __avsys_audio_path_set_ascn_ap_capture(control);
	}

#ifdef HAL
	if (!control->control_aif_before_path_set) {
		err = __avsys_open_aif(AIF_control);
	}
#endif

FINISHED:
	/* UnLOCK */
	if (AVSYS_FAIL(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
		avsys_error_r(AVAUDIO, "avsys_audio_unlock_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}
	avsys_info(AVAUDIO, "------------------------------------------------\n");
	return err;
}

int avsys_audio_path_ex_get_path(int *gain, int *out, int *in, int *option)
{
	avsys_audio_path_ex_info_t *control = NULL;
	avsys_audio_path_ex_info_t ** temp = NULL;

	if(gain == NULL || out == NULL || in == NULL || option == NULL)	{
		avsys_error(AVAUDIO,"Invalid parameter\n");
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	temp = &control;

	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_PATH, (void **)temp))) {
		avsys_error_r(AVAUDIO, "avsys_audio_get_shm() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}
	if (AVSYS_FAIL(avsys_audio_lock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
		avsys_error_r(AVAUDIO, "avsys_audio_lock_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	switch (control->gain.playback) {
	case AVSYS_AUDIO_PLAYBACK_GAIN_AP:
		*gain = AVSYS_AUDIO_GAIN_EX_KEYTONE;
		*out = control->path.playback;
		*in = AVSYS_AUDIO_PATH_EX_NONE;
		*option = control->option.playback;
		break;
	case AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT:
		*gain = AVSYS_AUDIO_GAIN_EX_RINGTONE;
		*out = control->path.playback;
		*in = AVSYS_AUDIO_PATH_EX_NONE;
		*option = control->option.playback;
		break;
	case AVSYS_AUDIO_PLAYBACK_GAIN_VOICECALL:
		*gain = AVSYS_AUDIO_GAIN_EX_VOICECALL;
		*out = control->path.playback;
		*in = control->path.capture;
		*option = control->option.playback;
		break;
	case AVSYS_AUDIO_PLAYBACK_GAIN_VIDEOCALL:
		*gain = AVSYS_AUDIO_GAIN_EX_VIDEOCALL;
		*out = control->path.playback;
		*in = control->path.capture;
		*option = control->option.playback;
		break;
	case AVSYS_AUDIO_PLAYBACK_GAIN_FMRADIO:
		*gain = AVSYS_AUDIO_GAIN_EX_FMRADIO;
		*out = control->path.playback;
		*in = control->path.capture;
		*option = control->option.playback;
		break;
	}

	if (AVSYS_FAIL(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
		avsys_error_r(AVAUDIO, "avsys_audio_unlock_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_path_ex_set_amp(const int onoff)
{
	//not yet implemented.
	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_path_ex_set_mute(const int mute)
{
	avsys_audio_path_ex_info_t *control = NULL;
	avsys_audio_path_ex_info_t **temp = NULL;

	temp = &control;

	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_PATH, (void **)temp))) {
		avsys_error_r(AVAUDIO, "avsys_audio_get_shm() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	if (control == NULL) {
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	if (mute == AVSYS_AUDIO_UNMUTE || mute == AVSYS_AUDIO_MUTE) {
		if (AVSYS_FAIL(avsys_audio_lock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
			avsys_error_r(AVAUDIO, "avsys_audio_lock_sync() failed in %s\n", __func__);
			return AVSYS_STATE_ERR_INTERNAL;
		}

		if (control->mute == mute) {
			avsys_info(AVAUDIO, "[Path Mute] skip mute ctrl op: %d\n", mute);
			if (AVSYS_FAIL(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
				avsys_error_r(AVAUDIO, "avsys_audio_unlock_sync() 2 failed in %s\n", __func__);
				return AVSYS_STATE_ERR_INTERNAL;
			}
			return AVSYS_STATE_SUCCESS;
		} else {
			control->mute = mute;
			avsys_warning(AVAUDIO, "[Path Mute] run mute ctrl op: %d\n", mute);
		}

		if (control->mute) {
			if (AVSYS_FAIL(avsys_audio_ascn_single_set(ASCN_STR_PLAYBACK_MUTE))) {
				if (AVSYS_FAIL(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
					avsys_error_r(AVAUDIO, "avsys_audio_unlock_sync() 1 failed in %s\n", __func__);
					return AVSYS_STATE_ERR_INTERNAL;
				}
				avsys_error(AVAUDIO, "Mute fail %s\n", __func__);
				return AVSYS_STATE_ERR_IO_CONTROL;
			}
		} else {
			if (AVSYS_FAIL(avsys_audio_ascn_single_set(ASCN_STR_PLAYBACK_UNMUTE))) {
				if (AVSYS_FAIL(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
					avsys_error_r(AVAUDIO, "avsys_audio_unlock_sync() 1 failed in %s\n", __func__);
					return AVSYS_STATE_ERR_INTERNAL;
				}
				avsys_error(AVAUDIO, "Unmute fail %s\n", __func__);
				return AVSYS_STATE_ERR_IO_CONTROL;
			}
		}

		if (AVSYS_FAIL(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
			avsys_error_r(AVAUDIO, "avsys_audio_unlock_sync() 2 failed in %s\n", __func__);
			return AVSYS_STATE_ERR_INTERNAL;
		}
	} else {
		int mute_nolock;
		if (mute == AVSYS_AUDIO_UNMUTE_NOLOCK)	/* set nomalize */
			mute_nolock = AVSYS_AUDIO_UNMUTE;
		else
			mute_nolock = AVSYS_AUDIO_MUTE;

		if (control->mute == mute_nolock) {
			avsys_info(AVAUDIO, "[Path Mute] skip mute ctrl op: %d\n", mute);
			return AVSYS_STATE_SUCCESS;
		} else {
			control->mute = mute_nolock;
			avsys_warning(AVAUDIO, "[Path Mute] run mute ctrl op: %d\n", mute);
		}

		if (control->mute) {
			if (AVSYS_FAIL(avsys_audio_ascn_single_set(ASCN_STR_PLAYBACK_MUTE))) {
				avsys_error(AVAUDIO, "Mute fail %s\n", __func__);
				return AVSYS_STATE_ERR_IO_CONTROL;
			}
		} else {
			if (AVSYS_FAIL(avsys_audio_ascn_single_set(ASCN_STR_PLAYBACK_UNMUTE))) {
				avsys_error(AVAUDIO, "Unmute fail %s\n", __func__);
				return AVSYS_STATE_ERR_IO_CONTROL;
			}
		}
	}

	if (mute == AVSYS_AUDIO_UNMUTE || mute == AVSYS_AUDIO_UNMUTE_NOLOCK)
		avsys_info_r(AVAUDIO, "Global Mute Disabled\n");
	else if (mute == AVSYS_AUDIO_MUTE || mute == AVSYS_AUDIO_MUTE_NOLOCK)
		avsys_info_r(AVAUDIO, "Global Mute Enabled\n");
	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_path_ex_get_mute(int *mute)
{
	avsys_audio_path_ex_info_t *control = NULL;
	avsys_audio_path_ex_info_t **temp = NULL;

	temp = &control;

	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_PATH, (void **)temp))) {
		avsys_error_r(AVAUDIO, "avsys_audio_get_shm() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	if (control == NULL) {
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	if (AVSYS_FAIL(avsys_audio_lock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
		avsys_error_r(AVAUDIO, "avsys_audio_lock_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	*mute = control->mute;

	if (AVSYS_FAIL(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH))) {
		avsys_error_r(AVAUDIO, "avsys_audio_unlock_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	return AVSYS_STATE_SUCCESS;
}

static int __avsys_audio_path_set_ascn_ap_playback(avsys_audio_path_ex_info_t *control)
{
	int cmd_gain[2] = { 0, 0 };
	int cmd_path[3] = { 0, 0, 0 };
	char callalert_mode = 0;

	avsys_info(AVAUDIO, "<< path.playback = %d, option = %x, gain.playback = %d, inserted = %d\n",
				control->path.playback, control->option.playback, control->gain.playback, control->inserted);

	callalert_mode = (control->gain.playback == AVSYS_AUDIO_PLAYBACK_GAIN_CALLALERT) ? 1 : 0;

	switch (control->path.playback) {
	case AVSYS_AUDIO_PATH_EX_SPK:
		if (control->option.playback & AVSYS_AUDIO_PATH_OPTION_DUAL_OUT) {
			if (callalert_mode) {
				cmd_gain[0] = INPUT_AP | OUTPUT_STEREO_SPK | GAIN_CALLALERT;
			} else {
				cmd_gain[0] = INPUT_AP | OUTPUT_STEREO_SPK | GAIN_MODE;
			}

			if (control->inserted == AVSYS_AUDIO_INSERTED_NONE) {
				cmd_path[0] = INPUT_AP | OUTPUT_STEREO_SPK;
			} else {
				cmd_path[0] = INPUT_AP | OUTPUT_STEREO_SPK;
				cmd_path[1] = INPUT_AP | OUTPUT_HEADSET;
			}
			control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
		} else {
			if (callalert_mode) {
				cmd_gain[0] = INPUT_AP | OUTPUT_STEREO_SPK | GAIN_CALLALERT;
			} else {
				cmd_gain[0] = INPUT_AP | OUTPUT_STEREO_SPK | GAIN_MODE;
			}
			control->ear_auto = AVSYS_AUDIO_EAR_SWITCH_AUTO_WITHOUT_MUTE;
			control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;

			cmd_path[0] = INPUT_AP | OUTPUT_STEREO_SPK;
		}
		break;

	case AVSYS_AUDIO_PATH_EX_RECV:
		control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
		cmd_gain[0] = INPUT_AP | OUTPUT_RECV | GAIN_MODE;
		cmd_path[0] = INPUT_AP | OUTPUT_RECV;
		break;

	case AVSYS_AUDIO_PATH_EX_HEADSET:
		if (callalert_mode) {
			cmd_gain[0] = INPUT_AP | OUTPUT_HEADSET | GAIN_CALLALERT;
		} else {
			cmd_gain[0] = INPUT_AP | OUTPUT_HEADSET | GAIN_MODE;
		}
		control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET;
		cmd_path[0] = INPUT_AP | OUTPUT_HEADSET;
		break;

	case AVSYS_AUDIO_PATH_EX_HDMI:
		avsys_warning(AVAUDIO, "Does not support dedicated HDMI sound path\n");
		control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
		break;

	case AVSYS_AUDIO_PATH_EX_DOCK:
		control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
		cmd_gain[0] = INPUT_AP | OUTPUT_DOCK | GAIN_MODE;
		cmd_path[0] = INPUT_AP | OUTPUT_DOCK;
		break;

	case AVSYS_AUDIO_PATH_EX_BTHEADSET:
		control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET;
		cmd_gain[0] = INPUT_AP | OUTPUT_BT_HEADSET | GAIN_MODE;
		cmd_path[0] = INPUT_AP | OUTPUT_BT_HEADSET;
		break;
	case AVSYS_AUDIO_PATH_EX_HANDSFREE:
	default:		/* DEFAULT PATH CONTROL TO NONE */
		control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
		break;
	}

	avsys_warning(AVAUDIO, "Run Alsa Scenario Script\n");
	RET_IO_CTL_ERR_IF_FAIL(avsys_audio_ascn_bulk_set(cmd_gain, 1, ASCN_RESET_PLAYBACK))
	RET_IO_CTL_ERR_IF_FAIL(avsys_audio_ascn_bulk_set(cmd_path, 2, ASCN_RESET_NONE))

	avsys_info(AVAUDIO, ">> leave");
	return AVSYS_STATE_SUCCESS;
}

static int __avsys_audio_path_set_ascn_voicecall(avsys_audio_path_ex_info_t *control)
{
	return AVSYS_STATE_SUCCESS;
}

static int __avsys_audio_path_set_ascn_videocall(avsys_audio_path_ex_info_t *control)
{
	int cmd_gain[2] = { 0, 0 };
	int cmd_path[3] = { 0, 0, 0 };
	int gain_idx = 0;
	int path_idx = 0;

	switch (control->path.playback) {
	case AVSYS_AUDIO_PATH_EX_NONE:
		avsys_warning(AVAUDIO, "[SZ] playback AVSYS_AUDIO_PATH_EX_NONE\n");
		break;

	case AVSYS_AUDIO_PATH_EX_SPK:
		if (control->reqgain.playback == control->gain.playback) {
			cmd_gain[gain_idx++] = INPUT_AP | OUTPUT_STEREO_SPK | GAIN_MODE;
			cmd_path[path_idx++] = INPUT_AP | OUTPUT_STEREO_SPK;
			control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
		} else {
			avsys_warning(AVAUDIO, "Sound Path request during VT call ignored.");
		}
		break;

	case AVSYS_AUDIO_PATH_EX_RECV:
		if (control->gain.playback == control->reqgain.playback) {
			cmd_gain[gain_idx++] = INPUT_AP | OUTPUT_RECV | GAIN_MODE;
			cmd_path[path_idx++] = INPUT_AP | OUTPUT_RECV;
			control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
		} else {
			avsys_warning(AVAUDIO, "Sound Path request during VT call ignored.");
		}
		break;

	case AVSYS_AUDIO_PATH_EX_HEADSET:
		if (control->reqgain.playback == control->gain.playback) {
			cmd_gain[gain_idx++] = INPUT_AP | OUTPUT_HEADSET | GAIN_MODE;
			cmd_path[path_idx++] = INPUT_AP | OUTPUT_HEADSET;
			control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET;
		} else {
			avsys_warning(AVAUDIO, "Sound Path request during VT call ignored.");
		}
		break;

	case AVSYS_AUDIO_PATH_EX_BTHEADSET:
		if (control->reqgain.playback == control->gain.playback) {
			cmd_gain[gain_idx++] = INPUT_AP | OUTPUT_BT_HEADSET | GAIN_MODE;
			cmd_path[path_idx++] = INPUT_AP | OUTPUT_BT_HEADSET;
			control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET;
		} else {
			avsys_warning(AVAUDIO, "Sound Path request during VT call ignored.");
		}
		break;

	case AVSYS_AUDIO_PATH_EX_HANDSFREE:
	default:
		if (control->reqgain.playback == control->gain.playback) {
			control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
		}
		break;
	}

	switch (control->path.capture) {
	case AVSYS_AUDIO_PATH_EX_NONE:
		avsys_warning(AVAUDIO, "[SZ] capture AVSYS_AUDIO_PATH_EX_NONE\n");
		break;

	case AVSYS_AUDIO_PATH_EX_MIC:
		if (control->option.capture & AVSYS_AUDIO_PATH_OPTION_USE_SUBMIC) {
			cmd_gain[gain_idx++] = INPUT_SUB_MIC | OUTPUT_AP | GAIN_MODE;
			cmd_path[path_idx++] = INPUT_SUB_MIC | OUTPUT_AP;
		} else {
			cmd_gain[gain_idx++] = INPUT_MAIN_MIC | OUTPUT_AP | GAIN_MODE;
			cmd_path[path_idx++] = INPUT_MAIN_MIC | OUTPUT_AP;
		}
		break;

	case AVSYS_AUDIO_PATH_EX_HEADSETMIC:
		cmd_gain[gain_idx++] = INPUT_EAR_MIC | OUTPUT_AP | GAIN_MODE;
		cmd_path[path_idx++] = INPUT_EAR_MIC | OUTPUT_AP;
		break;

	case AVSYS_AUDIO_PATH_EX_BTMIC:
		cmd_gain[gain_idx++] = INPUT_BT_MIC | OUTPUT_AP | GAIN_MODE;
		cmd_path[path_idx++] = INPUT_BT_MIC | OUTPUT_AP;

		break;
	case AVSYS_AUDIO_PATH_EX_HANDSFREE:
	default:
		break;
	}

	RET_IO_CTL_ERR_IF_FAIL(avsys_audio_ascn_single_set(ASCN_STR_RESET));
	RET_IO_CTL_ERR_IF_FAIL(avsys_audio_ascn_bulk_set(cmd_gain, 2, ASCN_RESET_NONE));
	RET_IO_CTL_ERR_IF_FAIL(avsys_audio_ascn_bulk_set(cmd_path, 2, ASCN_RESET_NONE));

	return AVSYS_STATE_SUCCESS;
}


static int __avsys_audio_path_set_ascn_fmradio(avsys_audio_path_ex_info_t *control)
{
	int cmd_gain[2] = { 0, 0 };
	int cmd_path[3] = { 0, 0, 0 };
	int skip_clear = 0;
	int skip_clear_record = 0;
	int gain_idx = 0;
	int path_idx = 0;

	avsys_warning(AVAUDIO, "req gain playback [%x], control gain playback [%x]\n",
			control->reqgain.playback, control->gain.playback);
	avsys_warning(AVAUDIO, "req gain capture [%x], control gain capture [%x]\n",
				control->reqgain.capture, control->gain.capture);

	switch (control->path.playback) {
	case AVSYS_AUDIO_PATH_EX_NONE:
		avsys_warning(AVAUDIO, "[SZ] playback AVSYS_AUDIO_PATH_EX_NONE\n");
		break;

	case AVSYS_AUDIO_PATH_EX_SPK:
		if (control->reqgain.playback == control->gain.playback) {
			avsys_warning(AVAUDIO, "req gain playback == control gain playback\n");
			cmd_gain[gain_idx++] = INPUT_AP | OUTPUT_STEREO_SPK | GAIN_MODE;
			cmd_path[path_idx++] = INPUT_AP | OUTPUT_STEREO_SPK;
		} else {
			avsys_warning(AVAUDIO, "req gain playback != control gain playback\n");
			/* append ap playback sound path */
			control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;
			cmd_path[path_idx++] = INPUT_AP | OUTPUT_STEREO_SPK;
			skip_clear = 1;
		}
		break;

	case AVSYS_AUDIO_PATH_EX_HEADSET:
		if (control->reqgain.playback == control->gain.playback) {
			avsys_warning(AVAUDIO, "req gain playback  == control gain playback\n");
			cmd_gain[gain_idx++] = INPUT_AP | OUTPUT_HEADSET | GAIN_MODE;
			cmd_path[path_idx++] = INPUT_AP | OUTPUT_HEADSET;
		} else {
			//append ap playback
			avsys_warning(AVAUDIO, "req gain playback != control gain playback\n");
			control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET;
			cmd_path[path_idx++] = INPUT_AP | OUTPUT_HEADSET;
			skip_clear = 1;
		}
		break;

	case AVSYS_AUDIO_PATH_EX_A2DP:
		if (control->reqgain.playback == control->gain.playback) {
			avsys_warning(AVAUDIO, "req gain playback == control gain playback\n");
			//control->ear_auto = AVSYS_AUDIO_EAR_SWITCH_MANUAL;
		} else {
			avsys_warning(AVAUDIO, "req gain playback != control gain playback\n");
			control->lvol_dev_type = AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET;
			skip_clear = 1;
		}
		break;

	default:
		break;
	}

	switch (control->path.capture) {
	case AVSYS_AUDIO_PATH_EX_FMINPUT:
		if (control->reqgain.capture == control->gain.capture) {
			avsys_warning(AVAUDIO, "req gain capture == control gain capture\n");
			cmd_path[path_idx++] = INPUT_FMRADIO | OUTPUT_AP | GAIN_MODE;
			cmd_path[path_idx++] = INPUT_FMRADIO | OUTPUT_AP;
			if (control->reqgain.capture == control->pregain.capture) {
				avsys_warning(AVAUDIO, "req gain capture == control pregain capture\n");
				skip_clear_record = 1;
			}
		}
		break;
	default:
		break;
	}

	if (skip_clear_record) {
		RET_IO_CTL_ERR_IF_FAIL(avsys_audio_ascn_single_set(ASCN_STR_RESET_PLAYBACK))
	} else if (!skip_clear) {
		RET_IO_CTL_ERR_IF_FAIL(avsys_audio_ascn_single_set(ASCN_STR_RESET))
	}
	RET_IO_CTL_ERR_IF_FAIL(avsys_audio_ascn_bulk_set(cmd_gain, gain_idx, ASCN_RESET_NONE))
	RET_IO_CTL_ERR_IF_FAIL(avsys_audio_ascn_bulk_set(cmd_path, path_idx, ASCN_RESET_NONE))

	return AVSYS_STATE_SUCCESS;
}

static int __avsys_audio_path_set_ascn_ap_capture(avsys_audio_path_ex_info_t *control)
{
	int cmd_gain[2] = { 0, 0 };
	int cmd_path[3] = { 0, 0, 0 };

	avsys_info(AVAUDIO, "<< path.capture = %d, option = %x, gain.capture = %d, inserted = %d\n",
				control->path.capture, control->option.capture, control->gain.capture, control->inserted);
	switch(control->path.capture) {
	case AVSYS_AUDIO_PATH_EX_MIC:
		control->ear_auto = AVSYS_AUDIO_EAR_SWITCH_AUTO_WITHOUT_MUTE;
		if (control->option.capture & AVSYS_AUDIO_PATH_OPTION_USE_SUBMIC) {
			cmd_gain[0] = INPUT_SUB_MIC | OUTPUT_AP | GAIN_MODE;
			cmd_path[0] = INPUT_SUB_MIC | OUTPUT_AP;
		} else if (control->option.capture & AVSYS_AUDIO_PATH_OPTION_USE_STEREOMIC) {
			cmd_gain[0] = INPUT_STEREO_MIC | OUTPUT_AP | GAIN_MODE;
			cmd_path[0] = INPUT_STEREO_MIC | OUTPUT_AP;
		} else {
			cmd_gain[0] = INPUT_MAIN_MIC | OUTPUT_AP | GAIN_MODE;
			cmd_path[0] = INPUT_MAIN_MIC | OUTPUT_AP;
		}
		break;

	case AVSYS_AUDIO_PATH_EX_HEADSETMIC:
		cmd_gain[0] = INPUT_EAR_MIC | OUTPUT_AP | GAIN_MODE;
		cmd_path[0] = INPUT_EAR_MIC | OUTPUT_AP;
		break;

	default:
		break;
	}

	RET_IO_CTL_ERR_IF_FAIL(avsys_audio_ascn_bulk_set(cmd_gain, 2, ASCN_RESET_CAPTURE))
	RET_IO_CTL_ERR_IF_FAIL(avsys_audio_ascn_bulk_set(cmd_path, 2, ASCN_RESET_NONE))

	avsys_info (AVAUDIO, ">> leave");

	return AVSYS_STATE_SUCCESS;
}


static int __avsys_audio_path_set_hw_controls(avsys_audio_path_ex_info_t *control)
{
	avsys_audio_handle_info_t *handle_control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	int ret = AVSYS_STATE_SUCCESS;
	avsys_info(AVAUDIO, "global mute %d\n", control->mute);

	/* update logical volume table - about output device - information for open handles */
	avsys_info(AVAUDIO, "Control handle informations\n");
	{
		avsys_audio_handle_t *ptr = NULL;
		int handle = -1;
		int out_device = AVSYS_AUDIO_LVOL_DEV_TYPE_SPK;

		temp = &handle_control;
		if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_HANDLE, (void**)temp))) {
			avsys_error(AVAUDIO, "avsys_audio_get_shm() failed in %s\n",__func__);
			return AVSYS_STATE_ERR_INTERNAL;
		}

		if (AVSYS_FAIL(avsys_audio_lock_sync(AVSYS_AUDIO_SYNC_IDEN_HANDLE))) {
			avsys_error(AVAUDIO, "avsys_audio_lock_sync() failed in %s\n",__func__);
			return AVSYS_STATE_ERR_INTERNAL;
		}

		while (++handle < AVSYS_AUDIO_HANDLE_MAX) {
			long long int flag = 0x01;
			flag <<= handle;

			if (handle_control->allocated & flag)
			{
				ptr = &(handle_control->handles[handle]);

				if (ptr->mode != AVSYS_AUDIO_MODE_OUTPUT && ptr->mode != AVSYS_AUDIO_MODE_OUTPUT_CLOCK
							&& ptr->mode != AVSYS_AUDIO_MODE_OUTPUT_LOW_LATENCY && ptr->mode != AVSYS_AUDIO_MODE_OUTPUT_AP_CALL) {
					continue;
				}
				ptr->path_off = 0;
				out_device = control->lvol_dev_type;
				avsys_audio_logical_volume_update_table(out_device, &ptr->gain_setting);
				avsys_audio_logical_volume_convert(&ptr->setting_vol, &ptr->working_vol, &ptr->gain_setting);
			}
		}

		if(AVSYS_FAIL(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_HANDLE))) {
			avsys_error(AVAUDIO,"avsys_audio_unlock_sync() failed in %s\n",__func__);
			return AVSYS_STATE_ERR_INTERNAL;
		}
	}
	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_path_set_volume(int handle)
{
	avsys_audio_path_ex_info_t *control = NULL;
	avsys_audio_path_ex_info_t **temp = NULL;
	avsys_audio_handle_t *ptr = NULL;
	int err;
	int gain_type;
	int out_device = AVSYS_AUDIO_DEVICE_TYPE_SPK;

	err = avsys_audio_handle_get_ptr(handle, &ptr, HANDLE_PTR_MODE_NORMAL);
	if (AVSYS_FAIL(err)) {
		avsys_error(AVAUDIO, "Handle is not allocated\n");
		avsys_audio_handle_release_ptr(handle, HANDLE_PTR_MODE_NORMAL);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	temp = &control;
	avsys_assert(AVSYS_SUCCESS(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_PATH, (void **)temp)));
	avsys_assert(control != NULL);
	avsys_assert(AVSYS_SUCCESS(avsys_audio_lock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH)));

	gain_type = ptr->gain_setting.vol_type;
	out_device = control->lvol_dev_type;
	avsys_assert(AVSYS_SUCCESS(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_PATH)));
	avsys_warning(AVAUDIO, "set path volume  : gain(%d), out_dev(%d)\n", gain_type, out_device);
	err = avsys_audio_logical_volume_set_table(gain_type, out_device, &ptr->gain_setting);
	avsys_audio_handle_release_ptr(handle, HANDLE_PTR_MODE_NORMAL);
	return err;
}

int avsys_audio_path_set_route_policy(avsys_audio_route_policy_t route)
{
	/* Deprecated */
	return 0;
}

int avsys_audio_path_get_route_policy(avsys_audio_route_policy_t *route)
{
	/* Deprecated */
	return 0;
}

int avsys_audio_path_set_single_ascn(char *str)
{
	if (!str)
		return AVSYS_STATE_ERR_INVALID_PARAMETER;

	RET_IO_CTL_ERR_IF_FAIL(avsys_audio_ascn_single_set(str))

	return AVSYS_STATE_SUCCESS;
}
