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

#include "avsys-common.h"
#include "avsys-debug.h"
#include "avsys-error.h"

#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define RETRY_EINTR
#include <fcntl.h>
#include <semaphore.h>

void * avsys_malloc(size_t size)
{
	char *allocated;
	allocated = (char*) malloc(size + 12);

	if(!allocated) {
		return NULL;
	}
	memset(allocated, 0, size+8);
	memcpy(allocated, AVSYS_MAGIC_START, 4);
	(*(int*)(allocated+4)) = size;
	memcpy(allocated+size+8, AVSYS_MAGIC_END, 4);

	return (void*)(allocated + 8);
}

void avsys_free(void *ptr)
{
	char *allocated = (char*)ptr;
	if(allocated) {
		if(avsys_mem_check(allocated)) {
			free(allocated-8);
		}
	}
}

int avsys_create_shm(const avsys_shm_param_t* param)
{
	int shmid = -1;
	int *segptr = NULL;
	key_t key;

	if (!param) {
		return AVSYS_STATE_ERR_NULL_POINTER;
	}
	if (param->size < 1 || param->key_path == NULL || access(param->key_path, R_OK) != 0) {
		return AVSYS_STATE_ERR_INVALID_VALUE;
	}

	key = ftok(param->key_path, param->key_prefix);

	if ((shmid = shmget(key, param->size, IPC_CREAT|IPC_EXCL|0666)) == -1) {
		if (errno == EEXIST) {
			avsys_error(AVAUDIO,"Already initialized.\n");
			if ((shmid = shmget(key, param->size, 0)) == -1) {
				avsys_error(AVAUDIO, "Initialization fail.\n");
			} else {
				segptr = shmat(shmid, 0, 0);
				avsys_assert_r(segptr != NULL);
			}
		} else {
			if(errno == EACCES)
				avsys_error_r(AVAUDIO, "Require ROOT permission.\n");
			else if(errno == ENOMEM)
				avsys_critical_r(AVAUDIO, "System memory is empty.\n");
			else if(errno == ENOSPC)
				avsys_critical_r(AVAUDIO, "Resource is empty.\n");
		}
	} else {
		shmctl(shmid, SHM_LOCK, 0);
		segptr = shmat(shmid, 0, 0);
		avsys_assert_r(segptr != NULL);
		memset((void*)segptr, 0, param->size);
	}

	if (shmid != -1) {
		return AVSYS_STATE_SUCCESS;
	} else {
		return AVSYS_STATE_ERR_INTERNAL;
	}
}

int avsys_remove_shm(const avsys_shm_param_t* param)
{
	int shmid = -1;
	key_t key;

	if (!param) {
		return AVSYS_STATE_ERR_NULL_POINTER;
	}
	if (param->size < 1 || param->key_path == NULL || access(param->key_path, R_OK) != 0) {
		return AVSYS_STATE_ERR_INVALID_VALUE;
	}

	key = ftok(param->key_path, param->key_prefix);

	if ((shmid = shmget(key, param->size, 0)) == -1) {
		if(errno == ENOENT)
			avsys_error_r(AVAUDIO, "Not initialized.\n");
		else if(errno == EACCES)
			avsys_error_r(AVAUDIO, "Require ROOT permission.\n");
		else if(errno == ENOSPC)
			avsys_critical_r(AVAUDIO, "Resource is empty.\n");
	} else {
		avsys_assert_r(shmctl(shmid, IPC_RMID, 0) == 0);
	}

	if (shmid != -1) {
		return AVSYS_STATE_SUCCESS;
	} else {
		return AVSYS_STATE_ERR_INTERNAL;
	}
}

void* avsys_get_shm(const avsys_shm_param_t* param)
{
	int shmid = -1;
	void *ptr = NULL;
	key_t key;

	if (!param) {
		return NULL;
	}
	if (param->size < 1 || param->key_path == NULL || access(param->key_path, R_OK) != 0) {
		return NULL;
	}

	key = ftok(param->key_path, param->key_prefix);

	if ((shmid = shmget(key, param->size, 0)) == -1) {
		if(errno == ENOENT)
			avsys_error_r(AVAUDIO, "Not initialized.\n");
		else if(errno == EACCES)
			avsys_error_r(AVAUDIO, "Require ROOT permission.\n");
		else if(errno == ENOSPC)
			avsys_critical_r(AVAUDIO, "Resource is empty.\n");
		return NULL;
	}

	ptr = shmat(shmid, 0, 0);
	if(ptr == (void*)-1) {
		if(errno == EACCES)
			avsys_error_r(AVAUDIO,"no permission\n");
		else if(errno == EINVAL)
			avsys_error_r(AVAUDIO,"invalid shmid\n");
		else if(errno == ENOMEM)
			avsys_error_r(AVAUDIO,"can not allocate memory\n");
		else
			avsys_error_r(AVAUDIO,"shmat() failed %d\n", errno);
		ptr = NULL;
	}
	return ptr;
}

int avsys_create_sync(const avsys_sync_param_t *param)
{
	sem_t *sem = NULL;
	sem = sem_open(param->key_path, O_CREAT, 0666, 1);
	if (sem == SEM_FAILED) {
		switch(errno)
		{
		case EACCES:
			avsys_error(AVAUDIO,
					"The semaphore already exist, but caller does not have permission %s\n",param->key_path);
			break;
		case ENOMEM:
			avsys_error(AVAUDIO,"Insufficient memory in %s (%d)\n",__func__,__LINE__);
			break;
		case ENFILE:
			avsys_error(AVAUDIO,"Too many open files in system %s (%d)\n",__func__,__LINE__);
			break;
		default:
			avsys_critical(AVAUDIO, "Semaphore create fail! (name:%s, errno %d)\n", param->key_path, errno);
			break;
		}
		return AVSYS_STATE_ERR_INTERNAL;
	}
	return AVSYS_STATE_SUCCESS;
}

int avsys_remove_sync(const avsys_sync_param_t *param)
{
	int err = 0;

	err = sem_unlink(param->key_path);
	if (err == -1) {
		avsys_critical(AVAUDIO, "Semaphore destroy Fail! (name:%s, errno %d)\n", param->key_path, errno);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	return AVSYS_STATE_SUCCESS;
}

int avsys_lock_sync(const avsys_sync_param_t *param)
{
	sem_t *sem = NULL;
	int ret;
	int err = AVSYS_STATE_SUCCESS;
	struct timespec wait_time;

	sem = sem_open(param->key_path, O_CREAT, 0666, 1);
	if (sem == SEM_FAILED) {
		avsys_critical(AVAUDIO, "Semaphore open Fail! (name:%s, errno %d)\n", param->key_path, errno);
		return AVSYS_STATE_ERR_INTERNAL;
	}
retry_lock:
	wait_time.tv_sec = (long int)(time(NULL)) + LOCK_TIMEOUT_SEC;
	wait_time.tv_nsec = 0;
	ret = sem_timedwait(sem, &wait_time);
	if(ret == -1) {
		switch(errno)
		{
		case EINTR:
			avsys_critical(AVAUDIO, "Lock RETRY LOCK\n");
			goto retry_lock;
			break;
		case EINVAL:
			avsys_critical(AVAUDIO, "Invalid semaphore\n");
			err = AVSYS_STATE_ERR_INTERNAL;
			break;
		case EAGAIN:
			avsys_critical(AVAUDIO, "EAGAIN\n");
			err = AVSYS_STATE_ERR_INTERNAL;
			break;
		case ETIMEDOUT:
			avsys_critical(AVAUDIO, "sem_wait leached %d seconds timeout.\n", LOCK_TIMEOUT_SEC);
			{
				/* Recovery of sem_wait lock....in abnormal condition */
				int sem_value = -1;
				if (0 == sem_getvalue(sem, &sem_value)) {
					avsys_critical(AVAUDIO,"%s sem value is %d\n",param->key_path, sem_value);
					if (sem_value == 0) {
						ret = sem_post(sem);
						if (ret == -1) {
							avsys_critical(AVAUDIO,"sem_post error %s : %d\n", param->key_path, sem_value);
						} else {
							avsys_critical_r(AVAUDIO,"lock recovery success...try lock again\n");
							goto retry_lock;
						}
					} else {
						avsys_critical(AVAUDIO,"sem value is not 0. but failed sem_timedwait so retry.. : %s\n",param->key_path);
						usleep(5);
						goto retry_lock;
					}
				} else {
					avsys_critical(AVAUDIO,"sem_getvalue failed : %s\n",param->key_path);
				}
			}
			err = AVSYS_STATE_ERR_INTERNAL;
			break;
		}
	}
	sem_close(sem);
	return err;
}

int avsys_unlock_sync(const avsys_sync_param_t *param)
{
	sem_t *sem = NULL;
	int ret;
	int err = AVSYS_STATE_SUCCESS;

	sem = sem_open(param->key_path, O_CREAT, 0666, 1);
	if (sem == SEM_FAILED) {
		avsys_critical(AVAUDIO, "Semaphore open Fail! (name:%s, errno %d)\n", param->key_path, errno);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	ret = sem_post(sem);
	if (ret == -1) {
		avsys_critical(AVAUDIO, "UNLOCK FAIL\n");
		err = AVSYS_STATE_ERR_INTERNAL;
	}

	sem_close(sem);
	return err;
}

int avsys_check_root_privilege()
{
	uid_t uid;
	uid = getuid();
	if(0 != uid) {
		/* code from man page */
		struct passwd pwd;
		struct passwd *result;
		char *buf;
		size_t bufsize;
		int s = 0;

		bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
		if (bufsize == -1)		  /* Value was indeterminate */
		   bufsize = 16384;		/* Should be more than enough */

		buf = malloc(bufsize);
		if (buf == NULL) {
		   perror("malloc");
		} else {
			s= getpwuid_r (uid, &pwd, buf, bufsize, &result);
			if (result == NULL) {
				if (s == 0)
				   printf("Not found\n");
				else {
				   errno = s;
				   perror("getpwnam_r");
				}
			} else {
				avsys_error_r(AVAUDIO,"super user privilege check failed (%s)\n", pwd.pw_name);
			}
			free (buf);
		}
		return AVSYS_STATE_ERR_PRIVILEGE;
	}
	return AVSYS_STATE_SUCCESS;
}

int avsys_dump_sync (const avsys_sync_param_t *param)
{
	int err = AVSYS_STATE_SUCCESS;

	sem_t *sem = NULL;
	int ret;
	int sem_value = -1;

	sem = sem_open(param->key_path, O_CREAT, 0666, 1);
	if (sem == SEM_FAILED) {
		avsys_critical(AVAUDIO, "Semaphore open Fail! (name:%s, errno %d)\n", param->key_path, errno);
		return AVSYS_STATE_ERR_INTERNAL;
	}

	if(0 == sem_getvalue(sem, &sem_value)) {
		fprintf (stdout, " * [%d] sem value for [%s]\n", sem_value, param->key_path);
	} else {
		avsys_critical(AVAUDIO,"sem_getvalue failed : %s\n",param->key_path);
	}

	sem_close(sem);
	return err;
}
