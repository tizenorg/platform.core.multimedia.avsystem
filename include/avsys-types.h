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

#ifndef	__AVSYS_TYPES_H__
#define	__AVSYS_TYPES_H__

#ifdef __cplusplus
	extern "C" {
#endif

/**
 * Type definition of av system handle.
 */
typedef void *avsys_handle_t;

#define AVSYS_INVALID_HANDLE			NULL
#define AVSYS_AUDIO_INVALID_HANDLE	(-1)

#ifdef __cplusplus
	}
#endif

#endif	/* __AVSYS_TYPES_H__ */
