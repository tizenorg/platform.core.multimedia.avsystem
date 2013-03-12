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

#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "avsys-common.h"
#include "avsys-audio-sync.h"
#include "avsys-error.h"
#include "avsys-debug.h"
#include "avsys-audio-shm.h"
#include "avsys-audio-path.h"

static avsys_sync_param_t g_presettings[AVSYS_AUDIO_SYNC_IDEN_CNT] = {
	{"audio_handle_lock", AVSYS_KEY_PREFIX_GEN(AVSYS_KEY_PREFIX_AUDIO, AVSYS_AUDIO_SYNC_IDEN_HANDLE)},
	{"audio_path_lock", AVSYS_KEY_PREFIX_GEN(AVSYS_KEY_PREFIX_AUDIO, AVSYS_AUDIO_SYNC_IDEN_PATH)},
};

int avsys_audio_create_sync(const avsys_audio_sync_iden_t iden)
{
	if (iden >= AVSYS_AUDIO_SYNC_IDEN_CNT || 0 > iden)
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	return avsys_create_sync(&g_presettings[iden]);
}

int avsys_audio_remove_sync(const avsys_audio_sync_iden_t iden)
{
	if (iden >= AVSYS_AUDIO_SYNC_IDEN_CNT || 0 > iden)
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	return avsys_remove_sync(&g_presettings[iden]);
}

int avsys_audio_lock_sync(const avsys_audio_sync_iden_t iden)
{
	pid_t pid = -1;
	int index = 0;
	int err = AVSYS_STATE_SUCCESS;

	if (iden >= AVSYS_AUDIO_SYNC_IDEN_CNT || 0 > iden) {
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}
	//avsys_info(AVAUDIO,"lock[%d]\n",(int)iden);

	err = avsys_lock_sync(&g_presettings[iden]);

	pid = getpid();
	if ((iden == AVSYS_AUDIO_SYNC_IDEN_PATH) && (err == AVSYS_STATE_SUCCESS)) {
		avsys_audio_path_ex_info_t *control = NULL;
		avsys_audio_path_ex_info_t **temp = NULL;

		temp = &control;
		if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_PATH, (void **)temp))) {
			avsys_error_r(AVAUDIO, "avsys_audio_get_shm() for path failed in %s\n", __func__);
			return AVSYS_STATE_ERR_INTERNAL;
		}

		do {
			/* Add mark */
			if (control->pathlock_pid[index] < 0) {	/* find empty slot */
				control->pathlock_pid[index] = pid;
				break;
			}
			index++;
			if (index == AVSYS_AUDIO_LOCK_SLOT_MAX) {
				int i;
				avsys_critical(AVAUDIO, "path lock pid slot is full. print stored pid before clear all slot\n");
				/* print and cleanup all stored pid */
				for (i = 0; i < AVSYS_AUDIO_LOCK_SLOT_MAX; i++) {
					avsys_critical(AVAUDIO, "path lock pid : %d\n", control->pathlock_pid[i]);
					control->pathlock_pid[i] = -1;
				}
				control->pathlock_pid[0] = pid; /* add current pid */
			}
		} while (index < AVSYS_AUDIO_LOCK_SLOT_MAX);
	} else if ((iden == AVSYS_AUDIO_SYNC_IDEN_HANDLE) && (err == AVSYS_STATE_SUCCESS)) {
		avsys_audio_handle_info_t *control = NULL;
		avsys_audio_handle_info_t **temp = NULL;

		temp = &control;
		if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_HANDLE, (void **)temp))) {
			avsys_error(AVAUDIO, "avsys_audio_get_shm() for handle failed in %s\n", __func__);
			return AVSYS_STATE_ERR_INTERNAL;
		}

		do {
			/* Add mark */
			if (control->handlelock_pid[index] < 0) {	/* find empty slot */
				control->handlelock_pid[index] = pid;
				break;
			}
			index++;
			if (index == AVSYS_AUDIO_LOCK_SLOT_MAX) {
				int i;
				avsys_critical(AVAUDIO, "handle lock pid slot is full. print stored pid before clear all slot\n");
				/* print and cleanup all stored pid */
				for (i = 0; i < AVSYS_AUDIO_LOCK_SLOT_MAX; i++) {
					avsys_critical(AVAUDIO, "handle lock pid : %d\n", control->handlelock_pid[i]);
					control->handlelock_pid[i] = -1;
				}
				control->handlelock_pid[0] = pid; /*add current pid */
			}
		} while (index < AVSYS_AUDIO_LOCK_SLOT_MAX);
	}
	return err;
}

int avsys_audio_unlock_sync(const avsys_audio_sync_iden_t iden)
{
	pid_t pid = -1;
	int index = 0;
	int err = AVSYS_STATE_SUCCESS;

	if (iden >= AVSYS_AUDIO_SYNC_IDEN_CNT || 0 > iden) {
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}
	//avsys_info(AVAUDIO,"unlock[%d]\n",(int)iden);

	err = avsys_unlock_sync(&g_presettings[iden]);

	pid = getpid();
	if ((iden == AVSYS_AUDIO_SYNC_IDEN_PATH) && (err == AVSYS_STATE_SUCCESS)) {
		avsys_audio_path_ex_info_t *control = NULL;
		avsys_audio_path_ex_info_t **temp = NULL;

		temp = &control;
		if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_PATH, (void **)temp))) {
			avsys_error_r(AVAUDIO, "avsys_audio_get_shm() failed in %s\n", __func__);
			return AVSYS_STATE_ERR_INTERNAL;
		}

		do {
			/* Remove mark */
			if (control->pathlock_pid[index] == pid) {	/* find empty slot */
				control->pathlock_pid[index] = -1;
				break;
			}
			index++;
			if (index == AVSYS_AUDIO_LOCK_SLOT_MAX)
				avsys_error(AVAUDIO, "Can not find pid (%d) in path_lock_pid slot\n", pid);
		} while (index < AVSYS_AUDIO_LOCK_SLOT_MAX);
	} else if ((iden == AVSYS_AUDIO_SYNC_IDEN_HANDLE) && (err == AVSYS_STATE_SUCCESS)) {
		avsys_audio_handle_info_t *control = NULL;
		avsys_audio_handle_info_t **temp = NULL;

		temp = &control;
		if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_HANDLE, (void **)temp))) {
			avsys_error(AVAUDIO, "avsys_audio_get_shm() for handle failed in %s\n", __func__);
			return AVSYS_STATE_ERR_INTERNAL;
		}

		do {
			/* Add mark */
			if (control->handlelock_pid[index] == pid) {	/* find empty slot */
				control->handlelock_pid[index] = -1;
				break;
			}
			index++;
			if (index == AVSYS_AUDIO_LOCK_SLOT_MAX)
				avsys_error(AVAUDIO, "Can not find pid (%d) in handlelock_pid slot\n", pid);
		} while (index < AVSYS_AUDIO_LOCK_SLOT_MAX);
	}

	return err;
}

/* find pid information in proc filesystem */
/* if pid exist, return AVSYS_STATE_SUCCESS */
/* if pid does not exist, return AVSYS_STATE_ERR_ALLOCATION */
int avsys_check_process(int check_pid)
{
	DIR *dir = NULL;
	char check_path[128] = "";
	int exist = AVSYS_STATE_SUCCESS;

	memset(check_path, '\0', sizeof(check_path));
	snprintf(check_path, sizeof(check_path) - 1, "/proc/%d", check_pid);

	dir = opendir(check_path);
	if (dir == NULL) {
		switch (errno) {
		case ENOENT:
			avsys_error(AVAUDIO, "pid %d does not exist anymore\n", check_pid);
			exist = AVSYS_STATE_ERR_ALLOCATION;
			break;
		case EACCES:
			avsys_error(AVAUDIO, "Permission denied\n");
			break;
		case EMFILE:
			avsys_error(AVAUDIO, "Too many file descriptors in use by process\n");
			break;
		case ENFILE:
			avsys_error(AVAUDIO, "Too many files are currently open in the system\n");
			break;
		default:
			avsys_error(AVAUDIO, "Other error : %d\n", errno);
			break;
		}
	} else {
		avsys_warning(AVAUDIO, "pid : %d still alive\n", check_pid);
		if (-1 == closedir(dir)) {
			avsys_error(AVAUDIO, "[%s] closedir failed with errno : %d\n", __func__, errno);
		}
	}
	return exist;
}

int avsys_audio_dump_sync(void)
{
	pid_t pid = -1;
	int index = 0;
	int err = AVSYS_STATE_SUCCESS;
	int i = 0;
	int sem_value = 0;

	fprintf(stdout, "Dump sync : Start\n");
	for (i = 0; i < AVSYS_AUDIO_SYNC_IDEN_CNT; i++) {
		err = avsys_dump_sync(&g_presettings[i]);
	}
	fprintf(stdout, "Dump sync : End\n");
	return err;
}
