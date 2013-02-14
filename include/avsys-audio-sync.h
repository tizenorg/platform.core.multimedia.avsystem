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

#ifndef __AVSYS_AUDIO_SYNC_H__
#define __AVSYS_AUDIO_SYNC_H__

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
	AVSYS_AUDIO_SYNC_IDEN_HANDLE,
	AVSYS_AUDIO_SYNC_IDEN_PATH,
	AVSYS_AUDIO_SYNC_IDEN_CNT
} avsys_audio_sync_iden_t;

int avsys_audio_create_sync(const avsys_audio_sync_iden_t iden);
int avsys_audio_remove_sync(const avsys_audio_sync_iden_t iden);
int avsys_audio_lock_sync(const avsys_audio_sync_iden_t iden);
int avsys_audio_unlock_sync(const avsys_audio_sync_iden_t iden);
int avsys_check_process(int check_pid);

#ifdef __cplusplus
	}
#endif

#endif /* __AVSYS_AUDIO_SYNC_H__ */
