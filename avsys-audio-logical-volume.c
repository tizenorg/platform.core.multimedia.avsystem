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
#include <stdlib.h>
#include <string.h>
#include <iniparser.h>
#include "avsys-audio-logical-volume.h"
#include "avsys-audio-shm.h"
#include "avsys-error.h"
#include "avsys-debug.h"

/* {TYPE, {MAXLEN, {MAXLEN, SPK{L,R}...}, {MAXLEN, RECV {L,R}...}, {MAXLEN, HEADSET{L,R}...}, {MAXLEN, BT{L,R}...}}} */
static const avsys_audio_lvol_volume_info_t g_volume_table[AVSYS_AUDIO_LVOL_GAIN_TYPE_MAX] = {
	/* AVSYS_AUDIO_LVOL_GAIN_TYPE_0 */ /* system : 0~15, default : 5 */
	{
		{
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{19000,19000}}, {{22323,22323}}, {{25647,25647}}, {{28971,28971}}, {{32295,32295}}, {{35619,35619}}, {{38943,38943}}, {{42267,42267}}, {{45591,45591}}, {{48915,48915}}, {{52239,52239}}, {{55563,55563}}, {{58887,58887}}, {{62211,62211}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	/* AVSYS_AUDIO_LVOL_GAIN_TYPE_1 */ /* notification : 0~15, default : 7 */
	{
		{
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{19000,19000}}, {{22323,22323}}, {{25647,25647}}, {{28971,28971}}, {{32295,32295}}, {{35619,35619}}, {{38943,38943}}, {{42267,42267}}, {{45591,45591}}, {{48915,48915}}, {{52239,52239}}, {{55563,55563}}, {{58887,58887}}, {{62211,62211}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	/* AVSYS_AUDIO_LVOL_GAIN_TYPE_2 */ /* alarm : 0~15, default : 7 */
	{
		{
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{19000,19000}}, {{22323,22323}}, {{25647,25647}}, {{28971,28971}}, {{32295,32295}}, {{35619,35619}}, {{38943,38943}}, {{42267,42267}}, {{45591,45591}}, {{48915,48915}}, {{52239,52239}}, {{55563,55563}}, {{58887,58887}}, {{62211,62211}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	/* AVSYS_AUDIO_LVOL_GAIN_TYPE_3 */ /* ringtone : 0~15, default : 13 */
	{
		{
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{19000,19000}}, {{22323,22323}}, {{25647,25647}}, {{28971,28971}}, {{32295,32295}}, {{35619,35619}}, {{38943,38943}}, {{42267,42267}}, {{45591,45591}}, {{48915,48915}}, {{52239,52239}}, {{55563,55563}}, {{58887,58887}}, {{62211,62211}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	/* AVSYS_AUDIO_LVOL_GAIN_TYPE_4 */ /* media */
	{
		{
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{19000,19000}}, {{22323,22323}}, {{25647,25647}}, {{28971,28971}}, {{32295,32295}}, {{35619,35619}}, {{38943,38943}}, {{42267,42267}}, {{45591,45591}}, {{48915,48915}}, {{52239,52239}}, {{55563,55563}}, {{58887,58887}}, {{62211,62211}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	/* AVSYS_AUDIO_LVOL_GAIN_TYPE_5 */ /* call */
	{
		{
			{LVOLUME_MAX_BASIC, {{{0,0}}, {{19000,19000}}, {{25991,25991}}, {{32982,32982}}, {{39973,39973}}, {{46964,46964}}, {{53955,53955}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_BASIC, {{{0,0}}, {{20480,20480}}, {{27225,27225}}, {{33969,33969}}, {{40714,40714}}, {{47458,47458}}, {{54203,54203}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET */
			{LVOLUME_MAX_BASIC, {{{0,0}}, {{20480,20480}}, {{27225,27225}}, {{33969,33969}}, {{40714,40714}}, {{47458,47458}}, {{54203,54203}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	/* AVSYS_AUDIO_LVOL_GAIN_TYPE_6 */ /* voip */
	{
		{
			{LVOLUME_MAX_BASIC, {{{0,0}}, {{19000,19000}}, {{25991,25991}}, {{32982,32982}}, {{39973,39973}}, {{46964,46964}}, {{53955,53955}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_BASIC, {{{0,0}}, {{20480,20480}}, {{27225,27225}}, {{33969,33969}}, {{40714,40714}}, {{47458,47458}}, {{54203,54203}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET */
			{LVOLUME_MAX_BASIC, {{{0,0}}, {{20480,20480}}, {{27225,27225}}, {{33969,33969}}, {{40714,40714}}, {{47458,47458}}, {{54203,54203}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	/* AVSYS_AUDIO_LVOL_GAIN_TYPE_7 */ /* fixed */
	{
		{
			{LVOLUME_MAX_SINGLE, {{{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_SINGLE, {{{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET */
			{LVOLUME_MAX_SINGLE, {{{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	/* AVSYS_AUDIO_LVOL_GAIN_TYPE_8 */ /* java */
	{
		{
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{19000,19000}}, {{22323,22323}}, {{25647,25647}}, {{28971,28971}}, {{32295,32295}}, {{35619,35619}}, {{38943,38943}}, {{42267,42267}}, {{45591,45591}}, {{48915,48915}}, {{52239,52239}}, {{55563,55563}}, {{58887,58887}}, {{62211,62211}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
};

static const avsys_audio_lvol_gain_info_t g_gain_table[AVSYS_AUDIO_LVOL_GAIN_EXT_TYPE_MAX_IDX] = {
	/* AVSYS_AUDIO_LVOL_GAIN_EXT_DIALER */
	{ 1.0, 1.0, 1.0 },
	/* AVSYS_AUDIO_LVOL_GAIN_EXT_TOUCH */
	{ 1.0, 1.0, 1.0 },
	/* AVSYS_AUDIO_LVOL_GAIN_EXT_AF */
	{ 1.0, 1.0, 1.0 },
	/* AVSYS_AUDIO_LVOL_GAIN_EXT_SHUTTER1 */
	{ 1.0, 1.0, 1.0 },
	/* AVSYS_AUDIO_LVOL_GAIN_EXT_SHUTTER2 */
	{ 1.0, 1.0, 1.0 },
	/* AVSYS_AUDIO_LVOL_GAIN_EXT_CAMCORDING */
	{ 1.0, 1.0, 1.0 },
	/* AVSYS_AUDIO_LVOL_GAIN_EXT_MIDI */
	{ 1.0, 1.0, 1.0 },
	/* AVSYS_AUDIO_LVOL_GAIN_EXT_BOOTING */
	{ 1.0, 1.0, 1.0 },
	/* AVSYS_AUDIO_LVOL_GAIN_EXT_VIDEO */
	{ 1.0, 1.0, 1.0 },
	/* AVSYS_AUDIO_LVOL_GAIN_EXT_VIDEO_HDMI */
	{ 1.0, 1.0, 1.0 },
};

int avsys_audio_logical_volume_get_max(int volume_type, int dev_type, int *max_level)
{
	void **data = NULL;
	avsys_audio_lvol_info_t *info = NULL;
	avsys_audio_lvol_volume_info_t *volume_table;

	data = (void **)&info;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}
	volume_table = info->volume_table;

	if ((max_level == NULL)
		|| (volume_type < 0 || volume_type >= AVSYS_AUDIO_LVOL_GAIN_TYPE_MAX)
		|| (dev_type < 0 || dev_type >= AVSYS_AUDIO_LVOL_DEV_TYPE_MAX)) {
		avsys_error(AVAUDIO, "Input param wrong. Please check params\n\t type %d\n\tdevicetype %d\n",
					volume_type, dev_type);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	*max_level = volume_table[volume_type].devices[dev_type].max_level;
	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_logical_volume_set_table(int volume_config, int dev_type, avsys_audio_volume_setting_t *setting)
{
	void **data = NULL;
	avsys_audio_lvol_info_t *info = NULL;
	avsys_audio_lvol_volume_info_t *volume_table;
	int vol_conf_type, vol_conf_gain;

	data = (void**)&info;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}
	volume_table = info->volume_table;
	vol_conf_type = AVSYS_AUDIO_VOLUME_CONFIG_TYPE(volume_config);
	vol_conf_gain = AVSYS_AUDIO_VOLUME_CONFIG_GAIN(volume_config);

	avsys_info(AVAUDIO, "Set Logical volume table\n");
	avsys_info(AVAUDIO, "\t vol_type %d vol_gain %x dev_type %d\n", vol_conf_type, vol_conf_gain, dev_type);

	if (setting == NULL) {
		avsys_error(AVAUDIO, "setting is null in %s\n", __func__);
		return AVSYS_STATE_ERR_NULL_POINTER;
	}
	if ((vol_conf_type < 0 || vol_conf_type >= AVSYS_AUDIO_LVOL_GAIN_TYPE_MAX)
		|| (dev_type < 0 || dev_type >= AVSYS_AUDIO_LVOL_DEV_TYPE_MAX)) {
		avsys_error(AVAUDIO, "Input param wrong. Please check params\n\ttype %d\n\tdevicetype %d\n",
					vol_conf_type, dev_type);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	setting->volume_config = volume_config;
	setting->dev_type = dev_type;
	setting->max_level = volume_table[vol_conf_type].devices[dev_type].max_level;

	avsys_info(AVAUDIO, "vol %d, dev%d table setted.\n", vol_conf_type, dev_type);
	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_logical_volume_update_table(int dev_type, avsys_audio_volume_setting_t *setting)
{
	void **data = NULL;
	avsys_audio_lvol_info_t *info = NULL;
	avsys_audio_lvol_volume_info_t *volume_table;
	int vol_conf_type;

	if (setting == NULL) {
		avsys_error(AVAUDIO, "setting is null in %s\n", __func__);
		return AVSYS_STATE_ERR_NULL_POINTER;
	}
	if (dev_type < 0 || dev_type >= AVSYS_AUDIO_LVOL_DEV_TYPE_MAX) {
		avsys_error(AVAUDIO, "Input param wrong. Please check params\tdevicetype %d\n", dev_type);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	data = (void **)&info;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}
	volume_table = info->volume_table;
	vol_conf_type = AVSYS_AUDIO_VOLUME_CONFIG_TYPE(setting->volume_config);

	avsys_info(AVAUDIO, "Update Logical volume table\n");
	avsys_info(AVAUDIO, "\t dev_type %d\n", dev_type);

	setting->dev_type = dev_type;
	setting->max_level = volume_table[vol_conf_type].devices[dev_type].max_level;

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_logical_volume_convert(avsys_audio_volume_t *level, avsys_audio_volume_t *converted, avsys_audio_volume_setting_t *setting)
{
	void **data = NULL;
	avsys_audio_lvol_info_t *info = NULL;
	avsys_audio_lvol_volume_info_t *volume_table;
	avsys_audio_lvol_gain_info_t *gain_table;
	int vol_conf_type, vol_conf_gain, volume_gain_idx;

	data = (void **)&info;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}
	volume_table = info->volume_table;
	gain_table = info->gain_table;
	vol_conf_type = AVSYS_AUDIO_VOLUME_CONFIG_TYPE(setting->volume_config);
	vol_conf_gain = AVSYS_AUDIO_VOLUME_CONFIG_GAIN(setting->volume_config);
	volume_gain_idx = AVSYS_AUDIO_VOLUME_GAIN_IDX(vol_conf_gain);

	if (level == NULL) {
		avsys_error(AVAUDIO, "level is null in %s\n", __func__);
		return AVSYS_STATE_ERR_NULL_POINTER;
	}
	if ((level->level[AVSYS_AUDIO_CHANNEL_LEFT] < 0 || level->level[AVSYS_AUDIO_CHANNEL_LEFT] >= setting->max_level)
		|| (level->level[AVSYS_AUDIO_CHANNEL_RIGHT] < 0 || level->level[AVSYS_AUDIO_CHANNEL_RIGHT] >= setting->max_level)) {
		avsys_error(AVAUDIO, "Input param wrong. Please check params\n\t level %d %d\n", level->level[AVSYS_AUDIO_CHANNEL_LEFT], level->level[AVSYS_AUDIO_CHANNEL_RIGHT]);
		return AVSYS_STATE_ERR_INVALID_VALUE;
	}

	converted->level[AVSYS_AUDIO_CHANNEL_LEFT] = volume_table[vol_conf_type].devices[setting->dev_type].logical_values[level->level[AVSYS_AUDIO_CHANNEL_LEFT]].level[AVSYS_AUDIO_CHANNEL_LEFT];
	converted->level[AVSYS_AUDIO_CHANNEL_RIGHT] = volume_table[vol_conf_type].devices[setting->dev_type].logical_values[level->level[AVSYS_AUDIO_CHANNEL_RIGHT]].level[AVSYS_AUDIO_CHANNEL_RIGHT];

	if (volume_gain_idx >= 0) {
		converted->level[AVSYS_AUDIO_CHANNEL_LEFT] *= gain_table[volume_gain_idx].devices[setting->dev_type];
		converted->level[AVSYS_AUDIO_CHANNEL_RIGHT] *= gain_table[volume_gain_idx].devices[setting->dev_type];
	}

	avsys_warning(AVAUDIO, "Volume converted vol_type %d vol_gain %x dev_type %d\n", vol_conf_type, vol_conf_gain, setting->dev_type);
	avsys_warning(AVAUDIO, "\tL: %d to %d\n", level->level[AVSYS_AUDIO_CHANNEL_LEFT], converted->level[AVSYS_AUDIO_CHANNEL_LEFT]);
	avsys_warning(AVAUDIO, "\tR: %d to %d\n", level->level[AVSYS_AUDIO_CHANNEL_RIGHT], converted->level[AVSYS_AUDIO_CHANNEL_RIGHT]);

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_logical_volume_init(void)
{
	void **data = NULL;
	avsys_audio_lvol_info_t *info = NULL;

	data = (void **)&info;
	avsys_audio_create_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME);
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}

	memcpy(info->volume_table, g_volume_table, sizeof(avsys_audio_lvol_volume_info_t) * AVSYS_AUDIO_LVOL_GAIN_TYPE_MAX);
	memcpy(info->gain_table, g_gain_table, sizeof(avsys_audio_lvol_gain_info_t) * AVSYS_AUDIO_LVOL_GAIN_EXT_TYPE_MAX_IDX);

	if (AVSYS_FAIL(avsys_audio_load_volume_from_ini())) {
		avsys_error(AVAUDIO, "Loading volume table from file failed. use default table\n");
	}

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_logical_volume_set_to_table(int volume_type, int dev_type, int step, int lv, int rv)
{
	void **data = NULL;
	avsys_audio_lvol_info_t *info = NULL;
	avsys_audio_lvol_volume_info_t *volume_table;

	data = (void **)&info;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}
	volume_table = info->volume_table;

	volume_table[volume_type].devices[dev_type].logical_values[step].level[AVSYS_AUDIO_CHANNEL_LEFT] = lv;
	volume_table[volume_type].devices[dev_type].logical_values[step].level[AVSYS_AUDIO_CHANNEL_RIGHT] = rv;
	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_logical_volume_get_from_table(int volume_type, int dev_type, int step, int *lv, int *rv)
{
	void **data = NULL;
	avsys_audio_lvol_info_t *info = NULL;
	avsys_audio_lvol_volume_info_t *volume_table;

	data = (void **)&info;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}
	volume_table = info->volume_table;

	*lv = volume_table[volume_type].devices[dev_type].logical_values[step].level[AVSYS_AUDIO_CHANNEL_LEFT];
	*rv = volume_table[volume_type].devices[dev_type].logical_values[step].level[AVSYS_AUDIO_CHANNEL_RIGHT];

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_logical_volume_set_gain_to_table(int volume_gain_idx, int dev_type, float lv, float rv)
{
	void **data = NULL;
	avsys_audio_lvol_info_t *info = NULL;
	avsys_audio_lvol_gain_info_t *gain_table;

	data = (void **)&info;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}
	gain_table = info->gain_table;

	gain_table[volume_gain_idx].devices[dev_type] = lv;
	gain_table[volume_gain_idx].devices[dev_type] = rv;

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_logical_volume_get_gain_from_table(int volume_gain_idx, int dev_type, float *lv, float *rv)
{
	void **data = NULL;
	avsys_audio_lvol_info_t *info = NULL;
	avsys_audio_lvol_gain_info_t *gain_table;

	data = (void **)&info;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}
	gain_table = info->gain_table;

	*lv = gain_table[volume_gain_idx].devices[dev_type];
	*rv = gain_table[volume_gain_idx].devices[dev_type];

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_load_volume_from_ini(void)
{
	const char *dev_str[] = {
		"speaker",		/* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
		"headset",		/* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET */
		"btheadset"		/* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
	};
	const char *vol_type_str[] = {
		"system",		/* AVSYS_AUDIO_LVOL_GAIN_TYPE_0 */
		"notification",	/* AVSYS_AUDIO_LVOL_GAIN_TYPE_1 */
		"alarm",		/* AVSYS_AUDIO_LVOL_GAIN_TYPE_2 */
		"ringtone",		/* AVSYS_AUDIO_LVOL_GAIN_TYPE_3 */
		"media",		/* AVSYS_AUDIO_LVOL_GAIN_TYPE_4 */
		"call"			/* AVSYS_AUDIO_LVOL_GAIN_TYPE_5 */
	};
	dictionary * dict = NULL;
	int dev_idx, vol_type_idx, dev_cnt, vol_type_cnt;

	dict = iniparser_load(AVSYS_VOLUME_INI_DEFAULT_PATH);
	if (!dict) {
		avsys_info(AVAUDIO, "Use temporary volume ini file");
		dict = iniparser_load(AVSYS_VOLUME_INI_TEMP_PATH);
		if (!dict) {
			avsys_warning(AVAUDIO, "Loading volume table from ini file failed");
			return AVSYS_STATE_ERR_INTERNAL;
		}
	}

	dev_cnt = sizeof(dev_str) / sizeof(char *);
	vol_type_cnt = sizeof(vol_type_str) / sizeof(char *);

	for (dev_idx = 0; dev_idx < dev_cnt; dev_idx++) {
		const char delimiter[] = ", ";
		char *key, *list_str, *token, *ptr = NULL;
		int vol_gain_idx = 0;
		float gain_value = 1.0f;

		/* Get volume table */
		for (vol_type_idx = 0; vol_type_idx < vol_type_cnt; vol_type_idx++) {
			int step_idx = 0, volume_value;

			key = malloc(strlen(dev_str[dev_idx]) + strlen(vol_type_str[vol_type_idx]) + 2);
			if (key) {
				sprintf(key, "%s:%s", dev_str[dev_idx], vol_type_str[vol_type_idx]);
				list_str = iniparser_getstr(dict, key);
				if (list_str) {
					token = strtok_r(list_str, delimiter, &ptr);
					while (token) {
						volume_value = atoi(token);
						avsys_audio_logical_volume_set_to_table(vol_type_idx, dev_idx, step_idx++, volume_value, volume_value);
						token = strtok_r(NULL, delimiter, &ptr);
					}
				}
				free(key);
			}
		}

		/* Get gain table */
		key = malloc(strlen(dev_str[dev_idx]) + strlen("gain") + 2);
		if (key) {
			sprintf(key, "%s:gain", dev_str[dev_idx]);
			list_str = iniparser_getstr(dict, key);
			if (list_str) {
				token = strtok_r(list_str, delimiter, &ptr);
				while (token) {
					gain_value = (float)atof(token);
					avsys_audio_logical_volume_set_gain_to_table(vol_gain_idx++, dev_idx, gain_value, gain_value);
					token = strtok_r(NULL, delimiter, &ptr);
				}
			}
			free(key);
		}
	}

	iniparser_freedict(dict);
	return AVSYS_STATE_SUCCESS;
}
