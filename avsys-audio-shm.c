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

#include "avsys-audio-shm.h"
#include "avsys-common.h"
#include "avsys-error.h"
#include "avsys-audio-handle.h"
#include "avsys-audio-path.h"
#include "avsys-audio-logical-volume.h"

#define SHM_KEY_PATH "/tmp"

typedef struct {
	avsys_shm_param_t param;
	void *pdst;
} _avsys_audio_shm_control_t;

static _avsys_audio_shm_control_t g_presettings[AVSYS_AUDIO_SHM_IDEN_CNT] = {
	{{SHM_KEY_PATH, AVSYS_KEY_PREFIX_GEN(AVSYS_KEY_PREFIX_AUDIO,AVSYS_AUDIO_SHM_IDEN_HANDLE) + 0x10,sizeof(avsys_audio_handle_info_t)}, NULL},
	{{SHM_KEY_PATH, AVSYS_KEY_PREFIX_GEN(AVSYS_KEY_PREFIX_AUDIO,AVSYS_AUDIO_SHM_IDEN_PATH) + 0x10,sizeof(avsys_audio_path_ex_info_t)}, NULL},
	{{SHM_KEY_PATH, AVSYS_KEY_PREFIX_GEN(AVSYS_KEY_PREFIX_AUDIO,AVSYS_AUDIO_SHM_IDEN_LVOLUME) + 0x10,sizeof(avsys_audio_lvol_info_t)}, NULL},
};

int avsys_audio_create_shm(const avsys_audio_shm_iden_t iden)
{
	if (iden >= AVSYS_AUDIO_SHM_IDEN_CNT || 0 > iden)
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	return avsys_create_shm(&g_presettings[iden].param);
}

int avsys_audio_remove_shm(const avsys_audio_shm_iden_t iden)
{
	if (iden >= AVSYS_AUDIO_SHM_IDEN_CNT || 0 > iden)
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	g_presettings[iden].pdst = NULL;
	return avsys_remove_shm(&g_presettings[iden].param);
}

int avsys_audio_get_shm(const avsys_audio_shm_iden_t iden, void **ptr)
{
	if (iden >= AVSYS_AUDIO_SHM_IDEN_CNT || 0 > iden)
		return AVSYS_STATE_ERR_INVALID_PARAMETER;

	if (!ptr)
		return AVSYS_STATE_ERR_NULL_POINTER;

	if (!g_presettings[iden].pdst) {
		g_presettings[iden].pdst = avsys_get_shm(&g_presettings[iden].param);
		if (!g_presettings[iden].pdst)
			return AVSYS_STATE_ERR_INTERNAL;
	}
	*ptr = (void *)g_presettings[iden].pdst;
	return AVSYS_STATE_SUCCESS;
}
