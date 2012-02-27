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

#ifndef __AVSYS_COMMON_H__
#define __AVSYS_COMMON_H__

#include <stdlib.h>
#include <sys/syscall.h>

#ifdef __cplusplus
    extern "C" {
#endif
#define EXPORT_API __attribute__((__visibility__("default")))

#define AVSYS_MAGIC_START   "TSVA"
#define AVSYS_MAGIC_END     "NEVA"

#define AVSYS_KEY_PREFIX_AUDIO 0x10
#define AVSYS_KEY_PREFIX_GEN(X,Y) ((X) + (Y))

typedef struct {
    char    *key_path;
    int     key_prefix;
    int     size;
} avsys_shm_param_t;

typedef struct {
    char    *key_path;
    int     key_prefix;
} avsys_sync_param_t;


#define LOCK_TIMEOUT_SEC 6

/* get thread id : Not implemented 'gettid' at system libs. */
#define avsys_gettid() (long int)syscall(__NR_gettid)

/* memory */
void * avsys_malloc(size_t size);
void avsys_free(void *ptr);
#define avsys_mem_check(ptr) ( (memcmp(AVSYS_MAGIC_START, ((char*)(ptr))-8, 4) == 0) && \
                                                (memcmp(AVSYS_MAGIC_END, ((char*)(ptr))+(*((int*)((ptr)-4))), 4) == 0) )

/* shared memory */
int avsys_create_shm(const avsys_shm_param_t* param);
int avsys_remove_shm(const avsys_shm_param_t* param);
void* avsys_get_shm(const avsys_shm_param_t* param);

/* Sync object */
int avsys_create_sync(const avsys_sync_param_t *param);
int avsys_remove_sync(const avsys_sync_param_t *param);
int avsys_lock_sync(const avsys_sync_param_t *param);
int avsys_unlock_sync(const avsys_sync_param_t *param);

/* Privilege */
int avsys_check_root_privilege(void);

#ifdef __cplusplus
	}
#endif

#endif /* __AVSYS_COMMON_H__ */
