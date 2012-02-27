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

#ifndef __AVSYS_AUDIO_LOGICAL_VOLUME_H__
#define __AVSYS_AUDIO_LOGICAL_VOLUME_H__

#include "avsys-audio.h"

#define LVOLUME_MAX_MULTIMEDIA	16 /**< same with AVSYS_AUDIO_VOLUME_MAX_MULTIMEDIA */
#define LVOLUME_MAX_BASIC		8 /**< same with AVSYS_AUDIO_VOLUME_MAX_BASIC */
#define LVOLUME_MAX_SINGLE		1 /**< same with AVSYS_AUDIO_VOLUME_MAX_SINGLE */

#define FADE_UP_MULTIPLIER 1//2
#define FADE_DOWN_MULTIPLIER 1

#define VOLUME_FILE_PATH		"/opt/system/volume.txt"

enum
{
	AVSYS_AUDIO_LVOL_GAIN_TYPE_0 = 0,	/* system */
	AVSYS_AUDIO_LVOL_GAIN_TYPE_1,		/* notification */
	AVSYS_AUDIO_LVOL_GAIN_TYPE_2,		/* alarm */
	AVSYS_AUDIO_LVOL_GAIN_TYPE_3,		/* ringtone */
	AVSYS_AUDIO_LVOL_GAIN_TYPE_4,		/* media */
	AVSYS_AUDIO_LVOL_GAIN_TYPE_5,		/* call */
	AVSYS_AUDIO_LVOL_GAIN_TYPE_6,		/* android */
	AVSYS_AUDIO_LVOL_GAIN_TYPE_7,		/* java */
	AVSYS_AUDIO_LVOL_GAIN_TYPE_8,		/* music (media2) */
	AVSYS_AUDIO_LVOL_GAIN_TYPE_MAX,
};

enum
{
	AVSYS_AUDIO_LVOL_DEV_TYPE_SPK,
	AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET,
	AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET,
	AVSYS_AUDIO_LVOL_DEV_TYPE_MAX,
};


typedef struct {
	int max_len;
	avsys_audio_volume_t gain[LVOLUME_MAX_MULTIMEDIA];
} avsys_a_logical_volume_t;


typedef struct {
	int type;
	avsys_a_logical_volume_t devices[AVSYS_AUDIO_LVOL_DEV_TYPE_MAX];
} avsys_logical_gain_t;

typedef struct {
	int vol_type;
	int dev_type;
	int max_len;
	avsys_audio_volume_t *table;
} avsys_audio_volume_setting_t;

int avsys_audio_logical_volume_get_max(int vol_type, int dev_type, int *max);
int avsys_audio_logical_volume_set_table(int vol_type, int dev_type, avsys_audio_volume_setting_t *setting);
int avsys_audio_logical_volume_update_table(int dev_type, avsys_audio_volume_setting_t *setting);
int avsys_audio_logical_volume_convert(avsys_audio_volume_t *level, avsys_audio_volume_t *converted, avsys_audio_volume_setting_t *setting);
/* Tuning */
int avsys_audio_logical_volume_init(void);
int avsys_audio_logical_volume_set_to_table(int gain_type, int dev_type, int step, int lv, int rv);
int avsys_audio_logical_volume_get_from_table(int gain_type, int dev_type, int step, int *lv, int *rv);
int avsys_audio_load_volume_from_file(void);

#endif /* __AVSYS_AUDIO_LOGICAL_VOLUME_H__ */
