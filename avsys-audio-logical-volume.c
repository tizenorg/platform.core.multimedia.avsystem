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
#include "avsys-audio-logical-volume.h"
#include "avsys-audio-shm.h"
#include "avsys-error.h"
#include "avsys-debug.h"

/* {TYPE, {MAXLEN, {MAXLEN, SPK{L,R}...}, {MAXLEN, RECV {L,R}...}, {MAXLEN, HEADSET{L,R}...}, {MAXLEN, BT{L,R}...}}} */
static const avsys_logical_gain_t g_volume_table[AVSYS_AUDIO_LVOL_GAIN_TYPE_MAX] = {
	{ AVSYS_AUDIO_LVOL_GAIN_TYPE_0, /* system : 0~15, default : 5 */
		{ /* FNT DEFAULT */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{19000,19000}}, {{22323,22323}}, {{25647,25647}}, {{28971,28971}}, {{32295,32295}}, {{35619,35619}}, {{38943,38943}}, {{42267,42267}}, {{45591,45591}}, {{48915,48915}}, {{52239,52239}}, {{55563,55563}}, {{58887,58887}}, {{62211,62211}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET0 */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	{ AVSYS_AUDIO_LVOL_GAIN_TYPE_1, /* notification : 0~15, default : 7 */
		{ /* FNT DEFAULT */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{19000,19000}}, {{22323,22323}}, {{25647,25647}}, {{28971,28971}}, {{32295,32295}}, {{35619,35619}}, {{38943,38943}}, {{42267,42267}}, {{45591,45591}}, {{48915,48915}}, {{52239,52239}}, {{55563,55563}}, {{58887,58887}}, {{62211,62211}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET0 */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	{ AVSYS_AUDIO_LVOL_GAIN_TYPE_2, /* alarm */
		{ /* FNT DEFAULT */
			{LVOLUME_MAX_BASIC, {{{0,0}}, {{19000,19000}}, {{25991,25991}}, {{32982,32982}}, {{39973,39973}}, {{46964,46964}}, {{53955,53955}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_BASIC, {{{0,0}}, {{20480,20480}}, {{27225,27225}}, {{33969,33969}}, {{40714,40714}}, {{47458,47458}}, {{54203,54203}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET0 */
			{LVOLUME_MAX_BASIC, {{{0,0}}, {{20480,20480}}, {{27225,27225}}, {{33969,33969}}, {{40714,40714}}, {{47458,47458}}, {{54203,54203}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	{ AVSYS_AUDIO_LVOL_GAIN_TYPE_3, /* ringtone : 0~15, default : 13 */
		{ /* FNT DEFAULT */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{19000,19000}}, {{22323,22323}}, {{25647,25647}}, {{28971,28971}}, {{32295,32295}}, {{35619,35619}}, {{38943,38943}}, {{42267,42267}}, {{45591,45591}}, {{48915,48915}}, {{52239,52239}}, {{55563,55563}}, {{58887,58887}}, {{62211,62211}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET0 */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	{ AVSYS_AUDIO_LVOL_GAIN_TYPE_4, /* media */
		{ /* FNT DEFAULT */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{19000,19000}}, {{22323,22323}}, {{25647,25647}}, {{28971,28971}}, {{32295,32295}}, {{35619,35619}}, {{38943,38943}}, {{42267,42267}}, {{45591,45591}}, {{48915,48915}}, {{52239,52239}}, {{55563,55563}}, {{58887,58887}}, {{62211,62211}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET0 */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	{ AVSYS_AUDIO_LVOL_GAIN_TYPE_5, /* call */
		{ /* FNT DEFAULT */
			{LVOLUME_MAX_BASIC, {{{0,0}}, {{19000,19000}}, {{25991,25991}}, {{32982,32982}}, {{39973,39973}}, {{46964,46964}}, {{53955,53955}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_BASIC, {{{0,0}}, {{20480,20480}}, {{27225,27225}}, {{33969,33969}}, {{40714,40714}}, {{47458,47458}}, {{54203,54203}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET0 */
			{LVOLUME_MAX_BASIC, {{{0,0}}, {{20480,20480}}, {{27225,27225}}, {{33969,33969}}, {{40714,40714}}, {{47458,47458}}, {{54203,54203}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}, {{60947,60947}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	{ AVSYS_AUDIO_LVOL_GAIN_TYPE_6, /* fixed */
		{ /* FNT DEFAULT */
			{LVOLUME_MAX_SINGLE, {{{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_SINGLE, {{{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET0 */
			{LVOLUME_MAX_SINGLE, {{{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	{ AVSYS_AUDIO_LVOL_GAIN_TYPE_7, /* java */
		{ /* FNT DEFAULT */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{19000,19000}}, {{22323,22323}}, {{25647,25647}}, {{28971,28971}}, {{32295,32295}}, {{35619,35619}}, {{38943,38943}}, {{42267,42267}}, {{45591,45591}}, {{48915,48915}}, {{52239,52239}}, {{55563,55563}}, {{58887,58887}}, {{62211,62211}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET0 */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
	{ AVSYS_AUDIO_LVOL_GAIN_TYPE_8, /* music only (max level) */
		{ /* FNT DEFAULT */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{19000,19000}}, {{22323,22323}}, {{25647,25647}}, {{28971,28971}}, {{32295,32295}}, {{35619,35619}}, {{38943,38943}}, {{42267,42267}}, {{45591,45591}}, {{48915,48915}}, {{52239,52239}}, {{55563,55563}}, {{58887,58887}}, {{62211,62211}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_SPK */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_HEADSET0 */
			{LVOLUME_MAX_MULTIMEDIA, {{{0,0}}, {{20480,20480}}, {{23698,23698}}, {{26916,26916}}, {{30135,30135}}, {{33353,33353}}, {{36571,36571}}, {{39789,39789}}, {{43008,43008}}, {{46226,46226}}, {{49444,49444}}, {{52662,52662}}, {{55880,55880}}, {{59099,59099}}, {{62317,62317}}, {{65535,65535}}}}, /* AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET */
		},
	},
};

int avsys_audio_logical_volume_get_max(int vol_type, int dev_type, int *max)
{
	void **data = NULL;
	avsys_logical_gain_t *table = NULL;
	data = (void **)&table;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}

	if (vol_type < 0 || vol_type >= AVSYS_AUDIO_LVOL_GAIN_TYPE_MAX ||
		dev_type < 0 || dev_type >= AVSYS_AUDIO_LVOL_DEV_TYPE_MAX ||
		max == NULL) {
		avsys_error(AVAUDIO, "Input param wrong. Please check params\n\t type %d\n\tdevicetype %d\n",
					vol_type, dev_type);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	*max = table[vol_type].devices[dev_type].max_len;
	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_logical_volume_set_table(int vol_type, int dev_type, avsys_audio_volume_setting_t *setting)
{
	void **data = NULL;
	avsys_logical_gain_t *table = NULL;
	data = (void**)&table;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}

	avsys_info(AVAUDIO, "Set Logical gain table\n");
	avsys_info(AVAUDIO, "\t vol_type %d dev_type %d\n", vol_type, dev_type);

	if (setting == NULL) {
		avsys_error(AVAUDIO, "setting is null in %s\n", __func__);
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	/* Change  ">" -> ">=" for CRASH LOGGER */
	if ((vol_type < 0 || vol_type >= AVSYS_AUDIO_LVOL_GAIN_TYPE_MAX) ||
		(dev_type < 0 || dev_type >= AVSYS_AUDIO_LVOL_DEV_TYPE_MAX)) {
		avsys_error(AVAUDIO, "Input param wrong. Please check params\n\ttype %d\n\tdevicetype %d\n",
					vol_type, dev_type);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	if (table[vol_type].type != vol_type) {
		avsys_error(AVAUDIO, "volume type does not match (%d, %d)in %s\n", table[vol_type].type, vol_type, __func__);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	setting->vol_type = vol_type;
	setting->dev_type = dev_type;
	setting->max_len = table[vol_type].devices[dev_type].max_len;
	setting->table = (avsys_audio_volume_t *)(((int)table[vol_type].devices[dev_type].gain) - (int)table);

	avsys_info(AVAUDIO, "vol %d, dev%d table setted.\n", vol_type, dev_type);
	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_logical_volume_update_table(int dev_type, avsys_audio_volume_setting_t *setting)
{
	void **data = NULL;
	avsys_logical_gain_t *table = NULL;
	data = (void **)&table;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}

	avsys_info(AVAUDIO, "Update Logical gain table\n");
	avsys_info(AVAUDIO, "\t dev_type %d\n", dev_type);

	if (setting == NULL) {
		avsys_error(AVAUDIO, "setting is null in %s\n", __func__);
		return AVSYS_STATE_ERR_NULL_POINTER;
	}
	if (table[setting->vol_type].type != setting->vol_type) {
		avsys_error(AVAUDIO, "volume type does not match in %s\n", __func__);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	if (dev_type < 0 || dev_type >= AVSYS_AUDIO_LVOL_DEV_TYPE_MAX) {
		avsys_error(AVAUDIO, "Input param wrong. Please check params\tdevicetype %d\n", dev_type);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}
	setting->dev_type = dev_type;
	setting->max_len = table[setting->vol_type].devices[dev_type].max_len;
	setting->table = (avsys_audio_volume_t *)(((int)table[setting->vol_type].devices[dev_type].gain) - (int)table);
	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_logical_volume_convert(avsys_audio_volume_t *level, avsys_audio_volume_t *converted, avsys_audio_volume_setting_t *setting)
{
	void **data = NULL;
	avsys_logical_gain_t *table = NULL;
	data = (void **)&table;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}

	if (level == NULL) {
		avsys_error(AVAUDIO, "level is null in %s\n", __func__);
		return AVSYS_STATE_ERR_NULL_POINTER;
	}
	if (level->level[AVSYS_AUDIO_CHANNEL_LEFT] < 0) {
		avsys_error(AVAUDIO, "negative volume level (left) in %s : %d\n", __func__, level->level[AVSYS_AUDIO_CHANNEL_LEFT]);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}
	if (level->level[AVSYS_AUDIO_CHANNEL_RIGHT] < 0) {
		avsys_error(AVAUDIO, "negative volume level (right) in %s : %d\n", __func__, level->level[AVSYS_AUDIO_CHANNEL_RIGHT]);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	if (setting->max_len <= level->level[AVSYS_AUDIO_CHANNEL_LEFT] ||
		setting->max_len <= level->level[AVSYS_AUDIO_CHANNEL_RIGHT] ||
		0 > level->level[AVSYS_AUDIO_CHANNEL_LEFT] ||
		0 > level->level[AVSYS_AUDIO_CHANNEL_RIGHT])
	{
		avsys_error(AVAUDIO, "Input param wrong. Please check params\n\t level %d %d\n", level->level[AVSYS_AUDIO_CHANNEL_LEFT], level->level[AVSYS_AUDIO_CHANNEL_RIGHT]);

		return AVSYS_STATE_ERR_INVALID_VALUE;
	}

	converted->level[AVSYS_AUDIO_CHANNEL_LEFT] = ((avsys_audio_volume_t *)((int)table + (int)setting->table))[level->level[AVSYS_AUDIO_CHANNEL_LEFT]].level[AVSYS_AUDIO_CHANNEL_LEFT];
	converted->level[AVSYS_AUDIO_CHANNEL_RIGHT] = ((avsys_audio_volume_t *)((int)table + (int)setting->table))[level->level[AVSYS_AUDIO_CHANNEL_RIGHT]].level[AVSYS_AUDIO_CHANNEL_RIGHT];

	avsys_warning(AVAUDIO, "Volume converted vol_type %d dev_type %d\n", setting->vol_type, setting->dev_type);
	avsys_warning(AVAUDIO, "\tL: %d to %d\n", level->level[AVSYS_AUDIO_CHANNEL_LEFT], converted->level[AVSYS_AUDIO_CHANNEL_LEFT]);
	avsys_warning(AVAUDIO, "\tR: %d to %d\n", level->level[AVSYS_AUDIO_CHANNEL_RIGHT], converted->level[AVSYS_AUDIO_CHANNEL_RIGHT]);

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_logical_volume_init(void)
{
	void *data = NULL;
	avsys_audio_create_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME);
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, &data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}
	memcpy(data, g_volume_table, sizeof(avsys_logical_gain_t) * AVSYS_AUDIO_LVOL_GAIN_TYPE_MAX);
	if (AVSYS_FAIL(avsys_audio_load_volume_from_file())) {
		avsys_error(AVAUDIO, "Loading volume table from file failed. use default table\n");
	}
	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_logical_volume_set_to_table(int gain_type, int dev_type, int step, int lv, int rv)
{
	void **data = NULL;
	avsys_logical_gain_t *table = NULL;
	data = (void **)&table;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}

	table[gain_type].devices[dev_type].gain[step].level[0] = lv;
	table[gain_type].devices[dev_type].gain[step].level[1] = rv;
	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_logical_volume_get_from_table(int gain_type, int dev_type, int step, int *lv, int *rv)
{
	void **data = NULL;
	avsys_logical_gain_t *table = NULL;
	data = (void **)&table;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_LVOLUME, data))) {
		avsys_error(AVAUDIO, "attach shared memory failed\n");
		return AVSYS_STATE_ERR_ALLOCATION;
	}

	*lv = table[gain_type].devices[dev_type].gain[step].level[0];
	*rv = table[gain_type].devices[dev_type].gain[step].level[1];
	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_load_volume_from_file()
{
	FILE *fp = NULL;
	int result = AVSYS_STATE_SUCCESS;
	char strBuffer[128];

	fp = fopen(VOLUME_FILE_PATH, "r");
	if (fp == NULL) {
		printf("Loading volume table from file failed\n");
		return AVSYS_STATE_ERR_INTERNAL;
	}

	memset(strBuffer, '\0', sizeof(strBuffer));
	while (fgets(strBuffer, sizeof(strBuffer), fp) != NULL) {
		char *pEnd = NULL;
		char *pStart = NULL;
		int volumetable = 0;
		int device = 0;
		int level = 0;
		int lvalue = 0, rvalue = 0;
		char parseBuf[10] = "";

		/* remove newline */
		if (strBuffer[strlen(strBuffer) - 1] == '\n')
			strBuffer[strlen(strBuffer) - 1] = '\0';
		if (strBuffer[strlen(strBuffer) - 1] == '\r')
			strBuffer[strlen(strBuffer) - 1] = '\0';
		if (strBuffer[0] == '#')
			continue;

		pStart = strBuffer;
		pEnd = strstr(pStart, ":");
		if (pEnd) {
			memset(parseBuf, '\0', sizeof(parseBuf));
			memcpy(parseBuf, pStart, pEnd - pStart);
			volumetable = atoi(parseBuf);
		} else {
			result = AVSYS_STATE_ERR_INTERNAL;
			break;
		}

		pStart = ++pEnd;
		pEnd = strstr(pStart, ":");
		if (pEnd) {
			memset(parseBuf, '\0', sizeof(parseBuf));
			memcpy(parseBuf, pStart, pEnd - pStart);
			if (strcmp(parseBuf, "SPK") == 0)
				device = 0;
			else if (strcmp(parseBuf, "HEADSET") == 0)
				device = 1;
			else if (strcmp(parseBuf, "BTHEADSET") == 0)
				device = 2;
		} else {
			result = AVSYS_STATE_ERR_INTERNAL;
			break;
		}

		pStart = ++pEnd;
		pEnd = strstr(pStart, ":");
		if (pEnd) {
			memset(parseBuf, '\0', sizeof(parseBuf));
			memcpy(parseBuf, pStart, pEnd - pStart);
			level = atoi(parseBuf);
		} else {
			result = AVSYS_STATE_ERR_INTERNAL;
			break;
		}

		pStart = ++pEnd;
		if (pEnd) {
			pEnd = strstr(pStart, ":");
			memset(parseBuf, '\0', sizeof(parseBuf));
			memcpy(parseBuf, pStart, pEnd - pStart);
			lvalue = atoi(parseBuf);
		} else {
			result = AVSYS_STATE_ERR_INTERNAL;
			break;
		}

		pStart = ++pEnd;
		rvalue = atoi(pStart);
		avsys_audio_set_volume_table(volumetable, device, level, lvalue, rvalue);
	}
	fclose(fp);
	//avsys_info(AVAUDIO,"Load volume table from file success\n");
	return result;
}
