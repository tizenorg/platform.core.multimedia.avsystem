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

#if !defined(_MMFW_I386_ALL_SIMULATOR)
#include <alsa/ascenario.h>
#endif
#include <string.h>

#include "avsys-audio-ascenario.h"
#include "avsys-types.h"
#include "avsys-error.h"
#include "avsys-debug.h"
#if defined(TIME_CHECK)
#include <sys/time.h>
#include <time.h>
#endif

#define STR_BUFF_MAX 128
#define P_STR_MAX 42
#define O_STR_MAX 44
#define CARD_NUMBER 0 /* ALSA card number */

static int __avsys_audio_ascn_make_scenario_str(int input, char *buf, int buf_len)
{
	char fromStr[P_STR_MAX] = { 0, };
	char toStr[P_STR_MAX] = { 0, };
	char optStr[O_STR_MAX] = { 0, };

	if (buf == NULL) {
		avsys_error(AVAUDIO, "Input Buf is null\n");
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	if (input & INPUT_MAIN_MIC) {
		strncpy(fromStr, "mainmic", sizeof(fromStr) - 1);
	}
	if (input & INPUT_SUB_MIC) {
		strncpy(fromStr, "submic", sizeof(fromStr) - 1);
	}
	if (input & INPUT_STEREO_MIC) {
		strncpy(fromStr, "stereomic", sizeof(fromStr) - 1);
	}
	if (input & INPUT_EAR_MIC) {
		strncpy(fromStr, "earmic", sizeof(fromStr) - 1);
	}
	if (input & INPUT_BT_MIC) {
		strncpy(fromStr, "bt", sizeof(fromStr) - 1);
	}
	if (input & INPUT_AP) {
		strncpy(fromStr, "ap", sizeof(fromStr) - 1);
	}
	if (input & INPUT_CP) {
		strncpy(fromStr, "cp", sizeof(fromStr) - 1);
	}
	if (input & INPUT_FMRADIO) {
		strncpy(fromStr, "fmradio", sizeof(fromStr) - 1);
	}

	if (input & OUTPUT_HEADSET) {
		strncpy(toStr, "headset", sizeof(toStr) - 1);
	}
	if (input & OUTPUT_LEFT_SPK) {
		strncpy(toStr, "speaker_left", sizeof(toStr) - 1);
	}
	if (input & OUTPUT_RIGHT_SPK) {
		strncpy(toStr, "speaker_right", sizeof(toStr) - 1);
	}
	if (input & OUTPUT_STEREO_SPK) {
		strncpy(toStr, "speaker", sizeof(toStr) - 1);
	}
	if (input & OUTPUT_RECV) {
		strncpy(toStr, "receiver", sizeof(toStr) - 1);
	}
	if (input & OUTPUT_BT_HEADSET) {
		strncpy(toStr, "bt", sizeof(toStr) - 1);
	}
	if (input & OUTPUT_CP) {
		strncpy(toStr, "cp", sizeof(toStr) - 1);
	}
	if (input & OUTPUT_AP) {
		strncpy(toStr, "ap", sizeof(toStr) - 1);
	}
	if (input & OUTPUT_HDMI) {
		strncpy(toStr, "hdmi", sizeof(toStr) - 1);
	}
	if (input & OUTPUT_DOCK) {
		strncpy(toStr, "dock", sizeof(toStr) - 1);
	}
	if (input & GAIN_MODE) {
		strncpy(optStr, "_gain", sizeof(optStr) - 1);
	}
	if (input & GAIN_VIDEO_CALL) {
		strncpy(optStr, "_videocall_gain", sizeof(optStr) - 1);
	}
	if (input & GAIN_VOICE_CALL) {
		strncpy(optStr, "_voicecall_gain", sizeof(optStr) - 1);
	}
	if (input & GAIN_CALLALERT) {
		strncpy(optStr, "_ringtone_gain", sizeof(optStr) - 1);
	}

	snprintf(buf, buf_len - 1, "%s_to_%s%s", fromStr, toStr, optStr);
	//avsys_info(AVAUDIO, "rslt str : %s\n", buf);
	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_ascn_bulk_set(int * bulk, int bulk_cnt, AscnResetType clear)
{
	struct snd_scenario *scn = NULL;
	int card = CARD_NUMBER;
	char *name = NULL;
	char str_buf[STR_BUFF_MAX] = { 0, };
	char reset_str[STR_BUFF_MAX] = { 0, };
	int err = AVSYS_STATE_SUCCESS;
	int i = 0;
#if defined(TIME_CHECK)
	struct timeval t_start, t_stop;
	unsigned long check = 0;
#endif

	avsys_info(AVAUDIO, "<< %s, clear = %d\n", __func__, clear);

	if (clear < ASCN_RESET_NONE || clear > ASCN_RESET_MODEM) {
		avsys_error_r(AVAUDIO, "invalid clear type %d\n", clear);
		return AVSYS_STATE_ERR_RANGE_OVER;
	}

	if (bulk == NULL) {
		avsys_error_r(AVAUDIO, "input bulk is null\n");
		return AVSYS_STATE_ERR_NULL_POINTER;
	}
	if (bulk_cnt < 0) {
		avsys_error_r(AVAUDIO, "bulk_cnt is negative\n");
		return AVSYS_STATE_ERR_RANGE_OVER;
	}

	/* Try to get card name from CARD_NUMBER. */
	snd_card_get_name(card, &name);
#if defined(TIME_CHECK)
	gettimeofday(&t_start, NULL);
#endif
	if (name == NULL) {
		scn = snd_scenario_open("default");
		if (scn == NULL)
		{
			avsys_error(AVAUDIO, "snd_scenario_open() failed to open with default\n");
			return AVSYS_STATE_ERR_INTERNAL;
		}
	} else {
		scn = snd_scenario_open(name);
		free(name);
		if (scn == NULL)
		{
			scn = snd_scenario_open("default");
			if (scn == NULL)
			{
				avsys_error(AVAUDIO, "snd_scenario_open() failed to open with default\n");
				return AVSYS_STATE_ERR_INTERNAL;
			}
		}
	}
#if defined(TIME_CHECK)
	gettimeofday(&t_stop, NULL);
	if (t_stop.tv_sec == t_start.tv_sec)
		check = (t_stop.tv_usec - t_start.tv_usec) / 1000;
	else
		check = (t_stop.tv_sec - t_start.tv_sec) * 1000 + (t_stop.tv_usec - t_start.tv_usec) / 1000;
	avsys_warning(AVAUDIO, "[snd_scenario_open()] takes %u msec\n", check);
#endif
	if (clear != ASCN_RESET_NONE) {
		switch (clear) {
		case ASCN_RESET_ALL:
			snprintf(reset_str, sizeof(reset_str) - 1, "%s", ASCN_STR_RESET);
			break;
		case ASCN_RESET_PLAYBACK:
			snprintf(reset_str, sizeof(reset_str) - 1, "%s", ASCN_STR_RESET_PLAYBACK);
			break;
		case ASCN_RESET_CAPTURE:
			snprintf(reset_str, sizeof(reset_str) - 1, "%s", ASCN_STR_RESET_CAPTURE);
			break;
		}
#if defined(TIME_CHECK)
		gettimeofday(&t_start, NULL);
#endif
		err = snd_scenario_set_scn(scn, reset_str);
#if defined(TIME_CHECK)
		gettimeofday(&t_stop, NULL);
		if (t_stop.tv_sec == t_start.tv_sec)
			check = (t_stop.tv_usec - t_start.tv_usec) / 1000;
		else
			check = (t_stop.tv_sec - t_start.tv_sec) * 1000 + (t_stop.tv_usec - t_start.tv_usec) / 1000;
		avsys_warning(AVAUDIO, "[%s] takes %u msec\n", reset_str, check);
#endif
		if (err < 0) {
			avsys_error(AVAUDIO, "Alsa sceanrio [%s] failed\n", reset_str);
		} else {
			avsys_warning(AVAUDIO, "Set [%s] success\n", reset_str);
		}
	}

	for (i = 0; i < bulk_cnt; i++) {
		if (bulk[i] == 0)
			continue;
		memset(str_buf, '\0', sizeof(str_buf));
		//avsys_info(AVAUDIO, "make string for 0x%X\n", bulk[i]);
		__avsys_audio_ascn_make_scenario_str(bulk[i], str_buf, sizeof(str_buf));
		//avsys_info(AVAUDIO, "result string [%s]\n", str_buf);
#if defined(TIME_CHECK)
		gettimeofday(&t_start, NULL);
#endif
		err = snd_scenario_set_scn(scn, str_buf);
#if defined(TIME_CHECK)
		gettimeofday(&t_stop, NULL);
		if (t_stop.tv_sec == t_start.tv_sec)
			check = (t_stop.tv_usec - t_start.tv_usec) / 1000;
		else
			check = (t_stop.tv_sec - t_start.tv_sec) * 1000 + (t_stop.tv_usec - t_start.tv_usec) / 1000;
		avsys_warning(AVAUDIO, "[%s] takes %u msec\n", str_buf, check);
#endif
		if (err < 0) {
			avsys_error_r(AVAUDIO, "snd_scenario_set_scn(%s) failed with %d\n", str_buf, err);
			goto bulk_error;
		}
		avsys_warning(AVAUDIO, "* [0x%010X] Set [%s] success\n", bulk[i], str_buf);
	}

bulk_error:
#if defined(TIME_CHECK)
	gettimeofday(&t_start, NULL);
#endif
	snd_scenario_close(scn);
#if defined(TIME_CHECK)
	gettimeofday(&t_stop, NULL);
	if (t_stop.tv_sec == t_start.tv_sec)
		check = (t_stop.tv_usec - t_start.tv_usec) / 1000;
	else
		check = (t_stop.tv_sec - t_start.tv_sec) * 1000 + (t_stop.tv_usec - t_start.tv_usec) / 1000;
	avsys_warning(AVAUDIO, "[snd_scenario_close()] takes %u msec\n", check);
#endif
	return err;
}


int avsys_audio_ascn_single_set(char * str)
{
	struct snd_scenario *scn = NULL;
	int card = CARD_NUMBER;
	char *name = NULL;
	char cmd_str[STR_BUFF_MAX] = { 0, };
	int err = AVSYS_STATE_SUCCESS;
#if defined(TIME_CHECK)
	struct timeval t_start, t_stop;
	unsigned long check = 0;
#endif

	if (str == NULL) {
		avsys_error_r(AVAUDIO, "input str is null\n");
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

#if defined(TIME_CHECK)
	gettimeofday(&t_start, NULL);
#endif

	/* Try to get card name from CARD_NUMBER. */
	snd_card_get_name(card, &name);

	if (name == NULL) {
		scn = snd_scenario_open("default");
		if (scn == NULL)
		{
			avsys_error(AVAUDIO, "snd_scenario_open() failed to open with default\n");
			return AVSYS_STATE_ERR_INTERNAL;
		}
	} else {
		scn = snd_scenario_open(name);
		free(name);
		if (scn == NULL)
		{
			scn = snd_scenario_open("default");
			if (scn == NULL)
			{
				avsys_error(AVAUDIO, "snd_scenario_open() failed to open with default\n");
				return AVSYS_STATE_ERR_INTERNAL;
			}
		}
	}

	/* Set scenario */
	strncpy(cmd_str, str, sizeof(cmd_str) - 1);
	err = snd_scenario_set_scn(scn, str);
	avsys_info(AVAUDIO, "alsa scenario set [%s]\n", str);

	if (err < 0) {
		avsys_error(AVAUDIO, "snd_scenario_set(%s) failed\n", str);
	}

	/* Close scenario manager core */
	snd_scenario_close(scn);

#if defined(TIME_CHECK)
	gettimeofday(&t_stop, NULL);
	if (t_stop.tv_sec == t_start.tv_sec)
		check = (t_stop.tv_usec - t_start.tv_usec) / 1000;
	else
		check = (t_stop.tv_sec - t_start.tv_sec) * 1000 + (t_stop.tv_usec - t_start.tv_usec) / 1000;
	avsys_warning(AVAUDIO, "[%s] takes %u msec\n", str, check);
#endif

	return err;
}
