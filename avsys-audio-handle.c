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
#include <unistd.h>

#include "avsys-common.h"
#include "avsys-audio-shm.h"
#include "avsys-audio-sync.h"
#include "avsys-debug.h"
#include "avsys-audio-handle.h"
#include "avsys-audio-shm.h"
#include "avsys-audio-path.h"
#include "avsys-audio-logical-volume.h"
#include "avsys-audio-pactrl.h"

#define DEFAULT_VOLUME_SYSTEM 5
#define DEFAULT_VOLUME_NOTIFICATION 7
#define DEFAULT_VOLUME_ALARM 7
#define DEFAULT_VOLUME_RINGTONE 13
#define DEFAULT_VOLUME_MEDIA 7
#define DEFAULT_VOLUME_CALL 7
#define DEFAULT_VOLUME_VOIP 7
#define DEFAULT_VOLUME_FIXED 0
#define DEFAULT_VOLUME_JAVA 11

#define AVSYS_GET_SHM(SHM,ERROR) do { \
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_HANDLE, (void **)SHM))) { \
		avsys_error(AVAUDIO,"avsys_audio_get_shm() failed in %s\n", __func__); \
		return ERROR; \
	} \
} while (0)

#define AVSYS_LOCK_SYNC() do { \
	if (AVSYS_FAIL(avsys_audio_lock_sync(AVSYS_AUDIO_SYNC_IDEN_HANDLE))) { \
		avsys_error(AVAUDIO,"avsys_audio_lock_sync() failed in %s\n", __func__); \
		return AVSYS_STATE_ERR_INTERNAL; \
	} \
} while (0)

#define AVSYS_UNLOCK_SYNC() do { \
	if (AVSYS_FAIL(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_HANDLE))) { \
		avsys_error(AVAUDIO,"avsys_audio_unlock_sync() failed in %s\n", __func__); \
		return AVSYS_STATE_ERR_INTERNAL; \
	} \
} while (0)

static int g_default_volume[AVSYS_AUDIO_VOLUME_TYPE_MAX] = {
	DEFAULT_VOLUME_SYSTEM,			/* AVSYS_AUDIO_VOLUME_TYPE_SYSTEM */
	DEFAULT_VOLUME_NOTIFICATION,	/* AVSYS_AUDIO_VOLUME_TYPE_NOTIFICATION */
	DEFAULT_VOLUME_ALARM,			/* AVSYS_AUDIO_VOLUME_TYPE_ALARM */
	DEFAULT_VOLUME_RINGTONE,		/* AVSYS_AUDIO_VOLUME_TYPE_RINGTONE */
	DEFAULT_VOLUME_MEDIA,			/* AVSYS_AUDIO_VOLUME_TYPE_MEDIA */
	DEFAULT_VOLUME_CALL,			/* AVSYS_AUDIO_VOLUME_TYPE_CALL */
	DEFAULT_VOLUME_VOIP,			/* AVSYS_AUDIO_VOLUME_TYPE_VOIP */
	DEFAULT_VOLUME_FIXED,			/* AVSYS_AUDIO_VOLUME_TYPE_FIXED */
	DEFAULT_VOLUME_JAVA,			/* AVSYS_AUDIO_VOLUME_TYPE_EXT_SYSTEM_JAVA */
};

EXPORT_API
int avsys_audio_handle_init(void)
{
	int i, err = 0;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;

	/* Check root user */
	err = avsys_check_root_privilege();
	if (AVSYS_FAIL(err)) {
		return err;
	}

	if (AVSYS_FAIL(avsys_audio_create_sync(AVSYS_AUDIO_SYNC_IDEN_HANDLE))) {
		avsys_error(AVAUDIO, "avsys_audio_create_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}
	if (AVSYS_FAIL(avsys_audio_create_shm(AVSYS_AUDIO_SHM_IDEN_HANDLE))) {
		avsys_error(AVAUDIO, "avsys_audio_create_shm() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	temp = &control;
	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);
	if (control == NULL) {
		avsys_error(AVAUDIO, "control is null in %s\n", __func__);
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	/* init allocted bits */
	control->allocated = 0;
	control->handle_amp = 0;
	memcpy(control->volume_value, (int *)g_default_volume, sizeof(int) * AVSYS_AUDIO_VOLUME_TYPE_MAX);
	control->ext_device_amp = AVSYS_AUDIO_HANDLE_EXT_DEV_NONE;
	control->primary_volume_pid = 0;
	control->primary_volume_type = -1;
	for (i = 0; i < AVSYS_AUDIO_HANDLE_MAX; i++) {
		control->handle_priority[i] = AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_0;
	}

	for (i = 0; i < AVSYS_AUDIO_LOCK_SLOT_MAX; i++) {
		control->handlelock_pid[i] = -1;
	}
	return AVSYS_STATE_SUCCESS;
}

EXPORT_API
int avsys_audio_handle_fini(void)
{

	AVSYS_LOCK_SYNC();

	if (AVSYS_FAIL(avsys_audio_remove_shm(AVSYS_AUDIO_SHM_IDEN_HANDLE))) {
		avsys_error(AVAUDIO, "avsys_audio_remove_shm() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	AVSYS_UNLOCK_SYNC();

	if (AVSYS_FAIL(avsys_audio_remove_sync(AVSYS_AUDIO_SYNC_IDEN_HANDLE))) {
		avsys_error(AVAUDIO, "avsys_audio_remove_sync() failed in %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	return AVSYS_STATE_SUCCESS;
}

EXPORT_API
int avsys_audio_handle_reset(int *volume_value)
{
	int i = 0, err = 0;
	long long int flag = 0x01;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	int * volumes;
	temp = &control;

	/* Check root user */
	err = avsys_check_root_privilege();
	if (AVSYS_FAIL(err)) {
		return err;
	}

	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);
	if (control == NULL) {
		avsys_error(AVAUDIO, "control is null in %s\n", __func__);
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	volumes = (volume_value) ? volume_value : (int *)g_default_volume;

	for (i = 0; i < AVSYS_AUDIO_HANDLE_MAX; i++) {
		if (control->allocated & (flag << i)) {	/* allocated condition */
			if (AVSYS_FAIL(avsys_check_process(control->handles[i].pid))) {
				/* process dead */
				if (AVSYS_FAIL(avsys_audio_handle_free(i))) {
					avsys_error(AVAUDIO, "Cleanup handle %d failed\n", i);
				}
			}
		}
	}

#ifdef USE_HIBERNATION
	while (control->allocated) {
		if (++i > 5)
			break;
		avsys_warning(AVAUDIO, "(%d)Waiting...0.5 sec for resume from hibernation\n", i);
		printf("(%d)Waiting...0.5 sec for resume from hibernation\n", i);
		usleep(500);
	}
#endif

	AVSYS_LOCK_SYNC();
	if (volume_value == NULL) {
		control->allocated = 0;
		control->handle_amp = 0;
	}
	memcpy(control->volume_value, volumes, sizeof(int) * AVSYS_AUDIO_VOLUME_TYPE_MAX);
	control->ext_device_amp = AVSYS_AUDIO_HANDLE_EXT_DEV_NONE;
	control->primary_volume_pid = 0;
	control->primary_volume_type = -1;
	for (i = 0; i < AVSYS_AUDIO_HANDLE_MAX; i++) {
		control->handle_priority[i] = AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_0;
	}
	/* Clear semaphore condition */
	for (i = 0; i < AVSYS_AUDIO_LOCK_SLOT_MAX; i++) {
		control->handlelock_pid[i] = -1;
	}

	AVSYS_UNLOCK_SYNC();

	return AVSYS_STATE_SUCCESS;
}

EXPORT_API
int avsys_audio_handle_rejuvenation(void)
{
	int i = 0;
	long long int flag = 0x01;
	int dead_handle = 0x00;

	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	int dead_pids[AVSYS_AUDIO_HANDLE_MAX];

	temp = &control;

	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);
	if (control == NULL) {
		avsys_error(AVAUDIO, "control is null in %s\n", __func__);
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	AVSYS_LOCK_SYNC();

	/* Clear semaphore condition */

	for (i = 0; i < AVSYS_AUDIO_HANDLE_MAX; i++) {
		dead_pids[i] = -1;
		if (control->allocated & (flag << i)) {	/* allocated condition */
			/* check pid of handle... still alive? */
			if (AVSYS_FAIL(avsys_check_process(control->handles[i].pid))) {
				avsys_error(AVAUDIO, "handle %d is dead\n", i);
				dead_handle |= (flag << i);
				dead_pids[i] = control->handles[i].pid;
			}
		}
	}

	AVSYS_UNLOCK_SYNC();

	avsys_warning(AVAUDIO, "dead_handle : 0x%0X\n", dead_handle);
	/* Cleanup dead handle... */
	for (i = 0; i < AVSYS_AUDIO_HANDLE_MAX; i++) {
		if (dead_handle & (flag << i)) {
			/* set priority of dead handle as lowest */
			control->handle_priority[i] = AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_0;
			/* free this handle */
			avsys_error(AVAUDIO, "Cleanup handle %d...\n", i);
			if (AVSYS_FAIL(avsys_audio_handle_free(i)))
				avsys_error(AVAUDIO, "Cleanup handle %d failed\n", i);
		}
	}
	if (dead_handle) {
		char high_priority_exist = 0;
		AVSYS_LOCK_SYNC();
		for (i = 0; i < AVSYS_AUDIO_HANDLE_MAX; i++) {
			if (control->handle_priority[i] > AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_0) {
				high_priority_exist = 1;
				break;
			}
		}

		for (i = 0; i < AVSYS_AUDIO_HANDLE_MAX; i++) {
			if (control->handle_priority[i] == AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_0) {
				if (high_priority_exist) {
					/* sink input mute immediately */
					if (control->handles[i].mute == AVSYS_AUDIO_UNMUTE) {
						if (AVSYS_FAIL(avsys_audio_pa_ctrl_mute_by_index(control->handles[i].stream_index, AVSYS_AUDIO_MUTE))) {
							avsys_error(AVAUDIO, "set sink input mute for %d failed\n", control->handles[i].stream_index);
						} else {
							avsys_warning(AVAUDIO, "set sink input mute for %d success\n", control->handles[i].stream_index);
						}
						control->handles[i].mute = AVSYS_AUDIO_MUTE;
					}
				} else {
					/* sink input unmute immediately */
					if (control->handles[i].mute == AVSYS_AUDIO_MUTE) {
						if (AVSYS_FAIL(avsys_audio_pa_ctrl_mute_by_index(control->handles[i].stream_index, AVSYS_AUDIO_UNMUTE))) {
							avsys_error(AVAUDIO, "set sink input unmute for %d failed\n", control->handles[i].stream_index);
						} else {
							avsys_warning(AVAUDIO, "set sink input unmute for %d success\n", control->handles[i].stream_index);
						}
						control->handles[i].mute = AVSYS_AUDIO_UNMUTE;
					}
				}

			} else	{ /* this is high priority case */
				/* sink input unmute immediately */
				if (control->handles[i].mute == AVSYS_AUDIO_MUTE) {
					if (AVSYS_FAIL(avsys_audio_pa_ctrl_mute_by_index(control->handles[i].stream_index, AVSYS_AUDIO_UNMUTE))) {
						avsys_error(AVAUDIO, "set sink input unmute for %d failed\n", control->handles[i].stream_index);
					} else {
						avsys_warning(AVAUDIO, "set sink input unmute for %d success\n", control->handles[i].stream_index);
					}
					control->handles[i].mute = AVSYS_AUDIO_UNMUTE;
				}
			}
		}

		AVSYS_UNLOCK_SYNC();
	}

	return AVSYS_STATE_SUCCESS;
}

EXPORT_API
int avsys_audio_handle_dump(void)
{
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	char *vol_str[] = { "System", "Notification", "Alarm", "Ringtone", "Media", "Call", "Fixed", "Java", "Media-HL" };
	char *dev_str[] = { "Speaker", "Headset", "BTHeadset" };
	int i = 0;
	long long int flag = 0x01;

	temp = &control;
	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);
	if (control == NULL) {
		avsys_error(AVAUDIO, "control is null in %s\n", __func__);
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	fprintf(stdout, "======================================================================\n");
	fprintf(stdout, "                      Opened Handles Information                      \n");
	fprintf(stdout, "======================================================================\n");
	fprintf(stdout, " Avsystem Handle alloc  : %016x\n", control->allocated);
	for (i = 0; i < AVSYS_AUDIO_LOCK_SLOT_MAX; i++) {
		if (control->handlelock_pid[i] > 0) {
			fprintf(stdout, " Handle Lock PIDs       : %d\n", control->handlelock_pid[i]);
		}
	}
	fprintf(stdout, "----------------------------------------------------------------------\n");
	for (i = 0; i < AVSYS_AUDIO_HANDLE_MAX; i++) {
		if (control->allocated & (flag << i)) {	/* allocated condition */
			fprintf(stdout, " Avsystem Handle ID    : %2d\n", i);
			fprintf(stdout, " Run Process ID        : 0x%08X (%d)\n", control->handles[i].pid, control->handles[i].pid);
			fprintf(stdout, " Run Thread ID         : 0x%08X (%d)\n", control->handles[i].tid, control->handles[i].tid);
			fprintf(stdout, " Open Mode             : %2d\n", control->handles[i].mode);
			fprintf(stdout, " Format                : %2d\n", control->handles[i].format);
			fprintf(stdout, " Channels              : %2d\n", control->handles[i].channels);
			fprintf(stdout, " Samplerate            : %2d\n", control->handles[i].samplerate);
			fprintf(stdout, " Priority              : %2d\n", control->handle_priority[i]);
			if(control->handles[i].mode == AVSYS_AUDIO_MODE_OUTPUT || control->handles[i].mode == AVSYS_AUDIO_MODE_OUTPUT_CLOCK ||
					control->handles[i].mode == AVSYS_AUDIO_MODE_OUTPUT_VIDEO || control->handles[i].mode == AVSYS_AUDIO_MODE_OUTPUT_LOW_LATENCY
					|| control->handles[i].mode == AVSYS_AUDIO_MODE_OUTPUT_AP_CALL) {
				int vol_conf_type = AVSYS_AUDIO_VOLUME_CONFIG_TYPE(control->handles[i].gain_setting.volume_config);
				if (control->handles[i].dirty_volume) {
					fprintf(stdout, " Dirty volume          : %s\n", vol_str[vol_conf_type]);
				} else {
					fprintf(stdout, " Volume Type           : %s\n", vol_str[vol_conf_type]);
				}
				fprintf(stdout, " Target device         : %s\n", dev_str[control->handles[i].gain_setting.dev_type]);
				fprintf(stdout, " Maximum Level         : %2d\n", control->handles[i].gain_setting.max_level);
				fprintf(stdout, " UI setted volume      : L:%3d R:%3d\n", control->handles[i].setting_vol.level[0], control->handles[i].setting_vol.level[1]);
				fprintf(stdout, " Real working volume   : L:%3d R:%3d\n", control->handles[i].working_vol.level[0], control->handles[i].working_vol.level[1]);
			}
			fprintf(stdout, " ----------------------------------------------------------------------\n");
		}
	}

	fprintf(stdout, " ----------------------------------------------------------------------\n");
	fprintf(stdout, " External dev amp      : 0x%08X\n", control->ext_device_amp);
	fprintf(stdout, " External dev status   : 0x%08X\n", control->ext_device_status);
	if (control->primary_volume_type >= 0) {
		fprintf(stdout, " Primary Volume type   : %s\n", vol_str[control->primary_volume_type]);
	}
	fprintf(stdout, " Volume [System]       : %2d ", control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_SYSTEM]);
	for (i = 0; i < control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_SYSTEM]; i++)
		fprintf(stdout, "+");
	fprintf(stdout, "\n");
	fprintf(stdout, " Volume [Notification] : %2d ", control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_NOTIFICATION]);
	for (i = 0; i < control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_NOTIFICATION]; i++)
		fprintf(stdout, "+");
	fprintf(stdout, "\n");
	fprintf(stdout, " Volume [Alarm]        : %2d ", control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_ALARM]);
	for (i = 0; i < control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_ALARM]; i++)
		fprintf(stdout, "+");
	fprintf(stdout, "\n");
	fprintf(stdout, " Volume [Ringtone]     : %2d ", control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_RINGTONE]);
	for (i = 0; i < control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_RINGTONE]; i++)
		fprintf(stdout, "+");
	fprintf(stdout, "\n");
	fprintf(stdout, " Volume [Media]        : %2d ", control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_MEDIA]);
	for (i = 0; i < control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_MEDIA]; i++)
		fprintf(stdout, "+");
	fprintf(stdout, "\n");
	fprintf(stdout, " Volume [Call]         : %2d ", control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_CALL]);
	for (i = 0; i < control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_CALL]; i++)
		fprintf(stdout, "+");
	fprintf(stdout, "\n");
	fprintf(stdout, " Volume [VOIP]         : %2d ", control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_VOIP]);
	for (i = 0; i < control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_VOIP]; i++)
		fprintf(stdout, "+");
	fprintf(stdout, "\n");
	fprintf(stdout, " Volume [Android]      : %2d ", control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_EXT_SYSTEM_ANDROID]);
	for (i = 0; i < control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_EXT_SYSTEM_ANDROID]; i++)
		fprintf(stdout, "+");
	fprintf(stdout, "\n");
	fprintf(stdout, " Volume [Java]         : %2d ", control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_EXT_SYSTEM_JAVA]);
	for (i = 0; i < control->volume_value[AVSYS_AUDIO_VOLUME_TYPE_EXT_SYSTEM_JAVA]; i++)
		fprintf(stdout, "+");
	fprintf(stdout, "\n");
	fprintf(stdout, "======================================================================\n");

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_handle_alloc(int *handle)
{
	long long int flag = 0x01;
	int i;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	temp = &control;

	avsys_info(AVAUDIO, "%s\n", __func__);
	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);

	AVSYS_LOCK_SYNC();
	for (i = 0; i < AVSYS_AUDIO_HANDLE_MAX; i++) {
		if ((control->allocated & flag) == 0) {	/* alloc condition */
			control->allocated |= flag;
			break;
		} else {
			flag <<= 1;
		}
	}

	AVSYS_UNLOCK_SYNC();

	if (i == AVSYS_AUDIO_HANDLE_MAX) {
		*handle = -1;
		return AVSYS_STATE_ERR_RANGE_OVER;
	} else {
		avsys_info(AVAUDIO, "handle allocated %d\n", i);
		memset(&control->handles[i], 0, sizeof(avsys_audio_handle_t));
		control->handles[i].pid = getpid();
		control->handles[i].tid = avsys_gettid();
		*handle = i;
		return AVSYS_STATE_SUCCESS;
	}
}

int avsys_audio_handle_free(int handle)
{
	long long int flag = 0x01;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	temp = &control;

	avsys_info(AVAUDIO, "%s, handle=[%d]\n", __func__, handle);

	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);

	flag <<= handle;

	AVSYS_LOCK_SYNC();
	if (control->allocated & flag) {	/* find condition */
		control->allocated &= ~flag;
		/* clear handle mute field */
		if (control->handle_amp & flag) {
			control->handle_amp &= ~flag;
		}
		AVSYS_UNLOCK_SYNC();
		return AVSYS_STATE_SUCCESS;
	} else {
		AVSYS_UNLOCK_SYNC();
		return AVSYS_STATE_ERR_INVALID_VALUE;
	}
}

int avsys_audio_handle_get_ptr(int handle, avsys_audio_handle_t **ptr, const int mode)
{
	long long int flag = 0x01;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	int ret = AVSYS_STATE_SUCCESS;

	//avsys_info(AVAUDIO, "%s handle %d\n", __func__, handle);

	if (handle < 0 || handle >= AVSYS_AUDIO_HANDLE_MAX) {
		*ptr = NULL;
		return AVSYS_STATE_ERR_INVALID_HANDLE;
	}

	if (mode < 0 || mode >= HANDLE_PTR_MODE_NUM) {
		*ptr = NULL;
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	temp = &control;
	if (AVSYS_FAIL(avsys_audio_get_shm(AVSYS_AUDIO_SHM_IDEN_HANDLE, (void **)temp))) {
		avsys_error(AVAUDIO, "avsys_audio_get_shm() failed in %s\n", __func__);
		*ptr = NULL;
		return AVSYS_STATE_ERR_INTERNAL;
	}

	flag <<= handle;

	if (mode == HANDLE_PTR_MODE_NORMAL) {
		if (AVSYS_FAIL(avsys_audio_lock_sync(AVSYS_AUDIO_SYNC_IDEN_HANDLE))) {
			avsys_error(AVAUDIO, "avsys_audio_lock_sync() failed in %s\n", __func__);
			*ptr = NULL;
			return AVSYS_STATE_ERR_INTERNAL;
		}
	}

	if (control->allocated & flag) {
		//avsys_info(AVAUDIO, "input handle %d flag %x allocated %x\n", handle, flag, control->allocated);
		*ptr = &(control->handles[handle]);
		ret = AVSYS_STATE_SUCCESS;
	} else {
		*ptr = NULL;
		ret = AVSYS_STATE_ERR_INVALID_VALUE;
	}

	if (mode == HANDLE_PTR_MODE_NORMAL) {
		if (AVSYS_FAIL(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_HANDLE))) {
			avsys_error(AVAUDIO, "avsys_audio_unlock_sync() failed in %s\n", __func__);
			*ptr = NULL;
			return AVSYS_STATE_ERR_INTERNAL;
		}
	}

	return ret;
}

int avsys_audio_handle_release_ptr(int handle, const int mode)
{
	long long int flag = 0x01;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	int ret = AVSYS_STATE_SUCCESS;

	//avsys_info(AVAUDIO, "%s handle %d\n", __func__, handle);

	if (handle < 0 || handle >= AVSYS_AUDIO_HANDLE_MAX) {
		return AVSYS_STATE_ERR_INVALID_HANDLE;
	}

	if (mode < 0 || mode >= HANDLE_PTR_MODE_NUM) {
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	temp = &control;
	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);

	flag <<= handle;

	//avsys_info(AVAUDIO, "input handle %d flag %x allocated %x\n", handle, flag, control->allocated);
	if (mode == HANDLE_PTR_MODE_NORMAL) {
		AVSYS_LOCK_SYNC();
	}

	if (control->allocated & flag) {
		ret = AVSYS_STATE_SUCCESS;
	} else {
		ret = AVSYS_STATE_ERR_INVALID_VALUE;
	}

	if (mode == HANDLE_PTR_MODE_NORMAL) {
		AVSYS_UNLOCK_SYNC();
	}

	return ret;
}

int avsys_audio_handle_set_mute(int handle, int mute)
{
	long long int flag = 0x01;
	int result = AVSYS_STATE_SUCCESS;
	int path_mute = 0;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	avsys_audio_handle_t *ptr = NULL;
	temp = &control;

	avsys_info(AVAUDIO, "%s_%d_%d\n", __func__, handle, mute);
	if (mute > AVSYS_AUDIO_MUTE || mute < AVSYS_AUDIO_UNMUTE) {
		avsys_error_r(AVAUDIO, "input parameter range error : mute %d\n", mute);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);

	flag <<= handle;

	AVSYS_LOCK_SYNC();

	if (control->allocated & flag) {	/* find condition */
		/* update mute information for input handle parameter */
		ptr = &(control->handles[handle]);
		ptr->mute = mute;
		avsys_audio_pa_ctrl_mute_by_index(ptr->stream_index, mute);

		/* update handle amp information */
		if (mute == AVSYS_AUDIO_UNMUTE) {
#ifdef _VERBOSE_
			if (control->handle_amp & flag)
				avsys_warning(AVAUDIO, "handle 0x%x already powered\n");
			else
				control->handle_amp |= flag;
#else
			if (!(control->handle_amp & flag))
				control->handle_amp |= flag;

			/* reset fadedown mute */
			ptr->fadeup_vol = 0;
#endif
		} else if (mute == AVSYS_AUDIO_MUTE) {
			/* clear handle amp field */
			if (control->handle_amp & flag)
				control->handle_amp &= ~flag;
#ifdef _VERBOSE_
			else
				avsys_warning(AVAUDIO, "handle 0x%x already off\n");
#endif
		}

		result = AVSYS_STATE_SUCCESS;
	} else {
		avsys_warning(AVAUDIO, "[%s] handle %d does not allocated\n", __func__, handle);
		if (AVSYS_FAIL(avsys_audio_unlock_sync(AVSYS_AUDIO_SYNC_IDEN_HANDLE))) {
			avsys_error(AVAUDIO, "avsys_audio_unlock_sync() failed in %s\n", __func__);
			return AVSYS_STATE_ERR_INTERNAL;
		}
		return AVSYS_STATE_ERR_INVALID_VALUE;
	}

	if (control->handle_amp | control->ext_device_amp)
		path_mute = AVSYS_AUDIO_UNMUTE;
	else
		path_mute = AVSYS_AUDIO_MUTE;

	AVSYS_UNLOCK_SYNC();

	if (AVSYS_FAIL(avsys_audio_path_ex_set_mute(path_mute))) {
		avsys_error_r(AVAUDIO, "Path mute control failed. %s_%d\n", __func__, path_mute);
		result = AVSYS_STATE_ERR_IO_CONTROL;
	}
	return result;
}

int avsys_audio_handle_ext_dev_set_mute(avsysaudio_ext_device_t device_type, int mute)
{
	int bit = 0;
	int result = AVSYS_STATE_SUCCESS;
	int path_mute = 0;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	temp = &control;

	avsys_info(AVAUDIO, "%s_%d_%d\n", __func__, (int)device_type, mute);
	if (mute > AVSYS_AUDIO_MUTE || mute < AVSYS_AUDIO_UNMUTE) {
		avsys_error_r(AVAUDIO, "input parameter range error : mute %d\n", mute);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	switch (device_type) {
	case AVSYS_AUDIO_EXT_DEVICE_FMRADIO:
		bit = AVSYS_AUDIO_HANDLE_EXT_DEV_FMRADIO;
		break;
	default:
		avsys_error(AVAUDIO, "Unknown device type %d\n", (int)device_type);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);

	AVSYS_LOCK_SYNC();

	if (mute == AVSYS_AUDIO_MUTE) {
		control->ext_device_amp |= bit;
	} else {
		control->ext_device_amp &= ~bit;
	}

	if (control->handle_amp | control->ext_device_amp)
		path_mute = AVSYS_AUDIO_UNMUTE;
	else
		path_mute = AVSYS_AUDIO_MUTE;

	AVSYS_UNLOCK_SYNC();

	if (AVSYS_FAIL(avsys_audio_path_ex_set_mute(path_mute))) {
		avsys_error_r(AVAUDIO, "Path mute control failed. %s_%d\n", __func__, path_mute);
		result = AVSYS_STATE_ERR_IO_CONTROL;
	}
	return result;
}

int avsys_audio_handle_ext_dev_status(avsysaudio_ext_device_t device_type, int *onoff)
{
	int result = AVSYS_STATE_SUCCESS;
	int ext_dev_state = 0;

	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	temp = &control;

	avsys_info(AVAUDIO, "%s\n", __func__);

	if (onoff == NULL) {
		avsys_error(AVAUDIO, "Null pointer input parameter\n");
		return AVSYS_STATE_ERR_NULL_POINTER;
	}

	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);

	AVSYS_LOCK_SYNC();

	switch (device_type) {
	case AVSYS_AUDIO_EXT_DEVICE_FMRADIO:
		if (control->ext_device_status & AVSYS_AUDIO_HANDLE_EXT_DEV_FMRADIO) {
			ext_dev_state = 1;
		} else {
			ext_dev_state = 0;
		}
		*onoff = ext_dev_state;
		break;
	default:
		avsys_error(AVAUDIO, "Invalid device type %d\n", device_type);
		result = AVSYS_STATE_ERR_INTERNAL;
		break;
	}

	AVSYS_UNLOCK_SYNC();
	return result;
}

int avsys_audio_handle_ext_dev_status_update(avsysaudio_ext_device_t device_type, int onoff)
{
	int bit = 0;
	int result = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	temp = &control;

	avsys_info(AVAUDIO, "%s_%d_%d\n", __func__, (int)device_type, onoff);

	switch (device_type) {
	case AVSYS_AUDIO_EXT_DEVICE_FMRADIO:
		bit = AVSYS_AUDIO_HANDLE_EXT_DEV_FMRADIO;
		break;
	default:
		avsys_error(AVAUDIO, "Unknown device type %d\n", (int)device_type);
		return AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);

	AVSYS_LOCK_SYNC();

	if (onoff > 0) {
		control->ext_device_status |= bit;
	} else if (onoff == 0) {
		control->ext_device_status &= ~bit;
	} else {
		avsys_error(AVAUDIO, "[%s] Unknown parameter %d. To nothing\n", __func__, onoff);
	}

	AVSYS_UNLOCK_SYNC();
	return result;
}

int avsys_audio_handle_current_playing_volume_type(int *type)
{
	int result = AVSYS_STATE_SUCCESS;
	int i = 0;
	char used_table[AVSYS_AUDIO_VOLUME_TYPE_MAX] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	char capture_used = 0;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	temp = &control;

	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);

	AVSYS_LOCK_SYNC();

	for (i = 0; i < AVSYS_AUDIO_HANDLE_MAX; i++) {
		long long int flag = 0x01;
		flag <<= i;
		if (control->allocated & flag) {
			int vol_conf_type = AVSYS_AUDIO_VOLUME_CONFIG_TYPE(control->handles[i].gain_setting.volume_config);
			if(control->handles[i].mode == AVSYS_AUDIO_MODE_OUTPUT ||
					control->handles[i].mode == AVSYS_AUDIO_MODE_OUTPUT_CLOCK ||
					control->handles[i].mode == AVSYS_AUDIO_MODE_OUTPUT_VIDEO ||
					control->handles[i].mode == AVSYS_AUDIO_MODE_OUTPUT_LOW_LATENCY ) {
				used_table[vol_conf_type] = 1;
			}
			else if(control->handles[i].mode == AVSYS_AUDIO_MODE_INPUT ||
					control->handles[i].mode == AVSYS_AUDIO_MODE_INPUT_HIGH_LATENCY ||
					control->handles[i].mode == AVSYS_AUDIO_MODE_INPUT_LOW_LATENCY) {
				capture_used = 1;
			}
		}
	}
	if (control->ext_device_status & AVSYS_AUDIO_HANDLE_EXT_DEV_FMRADIO) {
		used_table[AVSYS_AUDIO_VOLUME_TYPE_MEDIA] = 1;
	}

	avsys_warning(AVAUDIO,"Call[%d] VOIP[%d] Ringtone[%d] Media[%d] Alarm[%d] Notification[%d] System[%d] Android[%d] Java[%d] Capture[%d]\n",
			used_table[AVSYS_AUDIO_VOLUME_TYPE_CALL],
			used_table[AVSYS_AUDIO_VOLUME_TYPE_VOIP],
			used_table[AVSYS_AUDIO_VOLUME_TYPE_RINGTONE],
			used_table[AVSYS_AUDIO_VOLUME_TYPE_MEDIA],
			used_table[AVSYS_AUDIO_VOLUME_TYPE_ALARM],
			used_table[AVSYS_AUDIO_VOLUME_TYPE_NOTIFICATION],
			used_table[AVSYS_AUDIO_VOLUME_TYPE_SYSTEM],
			used_table[AVSYS_AUDIO_VOLUME_TYPE_EXT_SYSTEM_ANDROID],
			used_table[AVSYS_AUDIO_VOLUME_TYPE_EXT_SYSTEM_JAVA],
			capture_used);

	if (control->primary_volume_pid > 2 && AVSYS_FAIL(avsys_check_process(control->primary_volume_pid))) {
		avsys_warning(AVAUDIO, "Primary volume set pid does not exist anymore. clean primary volume\n");
		control->primary_volume_type = -1;
		control->primary_volume_pid = 0;
	}

	if (control->primary_volume_type != -1) {
		*type = control->primary_volume_type;
		avsys_warning(AVAUDIO, "Primary volume is %d\n", control->primary_volume_type);
	} else if (used_table[AVSYS_AUDIO_VOLUME_TYPE_CALL]) {
		*type = AVSYS_AUDIO_VOLUME_TYPE_CALL;
	} else if (used_table[AVSYS_AUDIO_VOLUME_TYPE_VOIP]) {
		*type = AVSYS_AUDIO_VOLUME_TYPE_VOIP;
	} else if (used_table[AVSYS_AUDIO_VOLUME_TYPE_RINGTONE]) {
		*type = AVSYS_AUDIO_VOLUME_TYPE_RINGTONE;
	} else if (used_table[AVSYS_AUDIO_VOLUME_TYPE_MEDIA]) {
		*type = AVSYS_AUDIO_VOLUME_TYPE_MEDIA;
	} else if (used_table[AVSYS_AUDIO_VOLUME_TYPE_ALARM]) {
		*type = AVSYS_AUDIO_VOLUME_TYPE_ALARM;
	} else if (used_table[AVSYS_AUDIO_VOLUME_TYPE_NOTIFICATION]) {
		*type = AVSYS_AUDIO_VOLUME_TYPE_NOTIFICATION;
	} else if (used_table[AVSYS_AUDIO_VOLUME_TYPE_SYSTEM]) {
		*type = AVSYS_AUDIO_VOLUME_TYPE_SYSTEM;
	} else if (used_table[AVSYS_AUDIO_VOLUME_TYPE_EXT_SYSTEM_JAVA]) {
		*type = AVSYS_AUDIO_VOLUME_TYPE_EXT_SYSTEM_JAVA;
	} else if (capture_used) {
		/* No Playing instance just capture only. */
		result = AVSYS_STATE_ERR_INVALID_MODE;
		avsys_error(AVAUDIO, "Capture handle only...\n");
	} else {
		/* not playing */
		result = AVSYS_STATE_ERR_ALLOCATION;
		avsys_error(AVAUDIO, "There is no running handles...\n");
	}

	AVSYS_UNLOCK_SYNC();

	return result;
}


int avsys_audio_handle_update_volume(avsys_audio_handle_t *p, const int volume_config)
{
	int result = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	avsys_audio_volume_t *set_volume = NULL;
	int vol_conf_type = AVSYS_AUDIO_VOLUME_CONFIG_TYPE(volume_config);

	temp = &control;
	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);

	AVSYS_LOCK_SYNC();

	set_volume = &(p->setting_vol);
	set_volume->level[AVSYS_AUDIO_CHANNEL_LEFT] = control->volume_value[vol_conf_type];
	set_volume->level[AVSYS_AUDIO_CHANNEL_RIGHT] = control->volume_value[vol_conf_type];
	result = avsys_audio_logical_volume_convert(set_volume, &(p->working_vol), &(p->gain_setting));

	AVSYS_UNLOCK_SYNC();

	return result;
}

int avsys_audio_handle_update_volume_by_type(const int volume_type, const int volume_value)
{
	int i;
	int result = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	temp = &control;

	avsys_info(AVAUDIO, "%s\n", __func__);
	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);

	AVSYS_LOCK_SYNC();

	control->volume_value[volume_type] = volume_value;

	for (i = 0; i < AVSYS_AUDIO_HANDLE_MAX; i++) {
		int mode;
		avsys_audio_volume_t *set_volume = NULL;
		long long int flag = 0x01;
		int vol_conf_type = AVSYS_AUDIO_VOLUME_CONFIG_TYPE(control->handles[i].gain_setting.volume_config);

		flag <<= i;
		if ((control->allocated & flag) == 0) {
			continue;
		}
		mode = control->handles[i].mode;
		if (mode != AVSYS_AUDIO_MODE_OUTPUT && mode != AVSYS_AUDIO_MODE_OUTPUT_CLOCK
				&& mode != AVSYS_AUDIO_MODE_OUTPUT_VIDEO && mode != AVSYS_AUDIO_MODE_OUTPUT_LOW_LATENCY
				&& mode != AVSYS_AUDIO_MODE_OUTPUT_AP_CALL) {
			continue;
		}

		if (vol_conf_type != volume_type) {
			continue;
		}

		if (control->handles[i].dirty_volume) {
			/* This is volatile volume per handle */
			continue;
		}

		set_volume = &(control->handles[i].setting_vol);
		set_volume->level[AVSYS_AUDIO_CHANNEL_LEFT] = control->volume_value[volume_type];
		set_volume->level[AVSYS_AUDIO_CHANNEL_RIGHT] = control->volume_value[volume_type];
		result = avsys_audio_logical_volume_convert(set_volume, &(control->handles[i].working_vol), &(control->handles[i].gain_setting));
		if (AVSYS_FAIL(result)) {
			avsys_error(AVAUDIO, "Can not set volume for handle %d. Error 0x%x\n", i, result);
			break;
		}
		avsys_warning(AVAUDIO, "stream index %d\n", control->handles[i].stream_index);
		if (AVSYS_FAIL(avsys_audio_pa_ctrl_volume_by_index(control->handles[i].stream_index, control->handles[i].working_vol.level[AVSYS_AUDIO_CHANNEL_LEFT], control->handles[i].channels))) {
			avsys_error(AVAUDIO, "avsys_audio_pa_ctrl_volume_by_index() failed\n");
		}
	}

	AVSYS_UNLOCK_SYNC();

	return result;
}

int avsys_audio_handle_set_primary_volume_type(const int pid, const int type, const int command)
{
	int result = AVSYS_STATE_SUCCESS;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;
	temp = &control;

	avsys_info(AVAUDIO, "%s\n", __func__);
	if (type < AVSYS_AUDIO_VOLUME_TYPE_SYSTEM || type >= AVSYS_AUDIO_VOLUME_TYPE_MAX || type == AVSYS_AUDIO_VOLUME_TYPE_FIXED) {
		avsys_error(AVAUDIO, "Invalid primary type primary type\n");
		return AVSYS_STATE_ERR_INTERNAL;
	}

	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);

	AVSYS_LOCK_SYNC();

	if (command == AVSYS_AUDIO_PRIMARY_VOLUME_SET) {
		if (control->primary_volume_type != -1) {
			avsys_warning(AVAUDIO,"Previous primary volume set by %d to %d\n",
														control->primary_volume_pid,
														control->primary_volume_type
														);
		}
		avsys_warning(AVAUDIO, "Primary Volume Type Set to %d [%d]\n", type, pid);
		control->primary_volume_pid = pid;
		control->primary_volume_type = type;
	} else if (command == AVSYS_AUDIO_PRIMARY_VOLUME_CLEAR) {
		if (pid != control->primary_volume_pid) {
			avsys_error(AVAUDIO, "Primary volume set api pair is not matched [%d] [%d]\n", control->primary_volume_pid, pid);
		} else {
			avsys_warning(AVAUDIO, "Primary Volume Type Clear [%d]\n", pid);
			control->primary_volume_pid = 0;
			control->primary_volume_type = -1;
		}
	} else {
		avsys_error(AVAUDIO, "Unknown Parameter : %d\n", command);
		result = AVSYS_STATE_ERR_INVALID_PARAMETER;
	}

	AVSYS_UNLOCK_SYNC();

	return result;
}

int avsys_audio_handle_update_priority(int handle, int priority, int handle_route, int cmd)
{
	long long int flag = 0x01;
	int i = 0;
	char high_priority_exist = 0;
	char transition_effect = 0;
	int lpriority = AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_0;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;

	temp = &control;

	if (cmd < AVSYS_AUDIO_SET_PRIORITY || cmd > AVSYS_AUDIO_UNSET_PRIORITY) {
		avsys_error_r(AVAUDIO, "Invalid command %s\n", __func__);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	if (priority >= AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_MAX || priority < AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_0) {
		avsys_error(AVAUDIO, "input parameter range error : priority %d. set lowest\n", priority);
		lpriority = AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_0;
	} else {
		lpriority = priority;
	}

	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);

	flag <<= handle;

	AVSYS_LOCK_SYNC();

	if (control->allocated & flag) {	/* find condition */
		/* update for allocated handle */
		if (cmd == AVSYS_AUDIO_SET_PRIORITY) {
			control->handle_priority[handle] = lpriority;
		} else if (cmd == AVSYS_AUDIO_UNSET_PRIORITY) {
			control->handle_priority[handle] = AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_0;
			if (lpriority == AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_2) {	/* unset with priority 2 */
				transition_effect = 1;
			}
		}
	} else {
		avsys_warning(AVAUDIO, "[%s] handle %d does not allocated\n", __func__, handle);
		AVSYS_UNLOCK_SYNC();
		return AVSYS_STATE_ERR_INVALID_VALUE;
	}

	for (i = 0; i < AVSYS_AUDIO_HANDLE_MAX; i++) {
		if (control->handle_priority[i] == AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_1) {
			high_priority_exist = 1;
		} else if (control->handle_priority[i] == AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_2) {
			high_priority_exist = 1;
			transition_effect = 1;
		}
	}

	for (i = 0; i < AVSYS_AUDIO_HANDLE_MAX; i++) {
		flag = 0x01;
		flag <<= i;
		if (!(control->allocated & flag))
			continue;

		if (control->handles[i].stream_index == 0) {
			avsys_warning(AVAUDIO, "handle[%d] has stream index 0, skip....(only mono sink-input use 0)\n", i);
			continue;
		}

		if (control->handle_priority[i] == AVSYS_AUDIO_HANDLE_PRIORITY_TYPE_0) {

			if (high_priority_exist) {	/* mute */
				if (transition_effect) {
					/* set fade out */
					if (control->handles[i].setting_vol.level[AVSYS_AUDIO_CHANNEL_LEFT] >= control->handles[i].setting_vol.level[AVSYS_AUDIO_CHANNEL_RIGHT]) {
						control->handles[i].fadeup_vol = (-1) * control->handles[i].setting_vol.level[AVSYS_AUDIO_CHANNEL_LEFT];
					} else {
						control->handles[i].fadeup_vol = (-1) * control->handles[i].setting_vol.level[AVSYS_AUDIO_CHANNEL_RIGHT];
					}
					control->handles[i].fadeup_multiplier = 0;
				} else {
					/* mute immediately */
					if (AVSYS_FAIL(avsys_audio_pa_ctrl_mute_by_index(control->handles[i].stream_index, AVSYS_AUDIO_MUTE))) {
						avsys_error(AVAUDIO, "set sink input mute for %d failed\n", control->handles[i].stream_index);
					} else {
						avsys_warning(AVAUDIO, "set sink input mute for %d success\n", control->handles[i].stream_index);
					}
					control->handles[i].mute = AVSYS_AUDIO_MUTE;
				}
			} else {	/* unmute */
				if (transition_effect) {
					/* set fade in */
					if (control->handles[i].setting_vol.level[AVSYS_AUDIO_CHANNEL_LEFT] >= control->handles[i].setting_vol.level[AVSYS_AUDIO_CHANNEL_RIGHT]) {
						control->handles[i].fadeup_vol = control->handles[i].setting_vol.level[AVSYS_AUDIO_CHANNEL_LEFT];
					} else {
						control->handles[i].fadeup_vol = control->handles[i].setting_vol.level[AVSYS_AUDIO_CHANNEL_RIGHT];
					}
					control->handles[i].fadeup_multiplier = 0;
				} else {
					/* unmute immediately */
					if (control->handles[i].mute != AVSYS_AUDIO_UNMUTE) {
						if (AVSYS_FAIL(avsys_audio_pa_ctrl_mute_by_index(control->handles[i].stream_index, AVSYS_AUDIO_UNMUTE))) {
							avsys_error(AVAUDIO, "set sink input unmute for %d failed\n", control->handles[i].stream_index);
						} else {
							avsys_warning(AVAUDIO, "set sink input unmute for %d success\n", control->handles[i].stream_index);
						}
						control->handles[i].mute = AVSYS_AUDIO_UNMUTE;
					}
				}
			}
		}
	}

	AVSYS_UNLOCK_SYNC();

	if (transition_effect)
		sleep(1);
	else if (!transition_effect && high_priority_exist)
		usleep(20);

	return AVSYS_STATE_SUCCESS;
}

int avsys_audio_handle_current_capture_status(int *on_capture)
{
	int i = 0;
	char capture_used = 0;
	avsys_audio_handle_info_t *control = NULL;
	avsys_audio_handle_info_t **temp = NULL;

	if (!on_capture)
		return AVSYS_STATE_ERR_INVALID_PARAMETER;

	temp = &control;

	AVSYS_GET_SHM(temp,AVSYS_STATE_ERR_INTERNAL);

	AVSYS_LOCK_SYNC();

	for (i = 0; i < AVSYS_AUDIO_HANDLE_MAX; i++) {
		long long int flag = 0x01;
		flag <<= i;
		if (control->allocated & flag) {
			if(control->handles[i].mode == AVSYS_AUDIO_MODE_INPUT ||
					control->handles[i].mode == AVSYS_AUDIO_MODE_INPUT_HIGH_LATENCY ||
					control->handles[i].mode == AVSYS_AUDIO_MODE_INPUT_LOW_LATENCY ||
					control->handles[i].mode == AVSYS_AUDIO_MODE_INPUT_AP_CALL) {
				capture_used = 1;
				break;
			}
		}
	}
	if (capture_used) {
		avsys_info(AVAUDIO, "Audio capture is running\n");
		*on_capture = 1;
	} else {
		*on_capture = 0;
	}

	AVSYS_UNLOCK_SYNC();

	return AVSYS_STATE_SUCCESS;
}
