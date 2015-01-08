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

/**
 * This file defines the debug function of AV System.
 *
 * @file		avsys_debug.h
 * @version	0.1
 */

#ifndef __AVSYS_DEBUG_H__
#define __AVSYS_DEBUG_H__

#ifdef __USE_LOGMANAGER__
#include <stdio.h>
#include <mm_log.h>
#define AVAUDIO  LOG_AVSYSTEM
#else
#define AVAUDIO
#endif

#ifdef __DEBUG_MODE__
#ifdef __USE_LOGMANAGER__

#define log_assert_rel(condition) \
	do { \
		if(!(condition)) { \
			mm_log_by_owner(0, LOG_FATAL, "Assertion Fail", NULL); \
			abort(); \
		} \
	} while(0)

#define avsys_debug_r(owner, msg, args...)		mm_log_by_owner( owner, LOG_DEBUG, msg, ##args )
#define avsys_info_r(owner, msg, args...)		mm_log_by_owner( owner, LOG_INFO, msg, ##args )
#define avsys_warning_r(owner, msg, args...)	mm_log_by_owner( owner, LOG_WARN, msg, ##args )
#define avsys_error_r(owner, msg, args...)		mm_log_by_owner( owner, LOG_ERROR, msg, ##args )
#define avsys_critical_r(owner, msg, args...)	mm_log_by_owner( owner, LOG_FATAL, msg, ##args )
#define avsys_assert_r(condition)				log_assert_rel(( condition ))

#define avsys_debug(owner, msg, args...)			mm_log_by_owner( owner, LOG_DEBUG, msg, ##args )
#define avsys_info(owner, msg, args...)			mm_log_by_owner( owner, LOG_INFO, msg, ##args )
#define avsys_warning(owner, msg, args...)		mm_log_by_owner( owner, LOG_WARN, msg, ##args )
#define avsys_error(owner, msg, args...)		mm_log_by_owner( owner, LOG_ERROR, msg, ##args )
#define avsys_critical(owner, msg, args...)		mm_log_by_owner( owner, LOG_FATAL, msg, ##args )
#define avsys_assert(condition)					log_assert_rel( (condition) )

#else	/* __USE_LOGMANAGER__ */

#define avsys_debug_r(owner, msg, args...)		fprintf(stderr, msg, ##args)
#define avsys_info_r(owner, msg, args...)		fprintf(stderr, msg, ##args)
#define avsys_warning_r(owner, msg, args...)	fprintf(stderr, msg, ##args)
#define avsys_error_r(owner, msg, args...)		fprintf(stderr, msg, ##args)
#define avsys_critical_r(owner, msg, args...)	fprintf(stderr, msg, ##args)
#define avsys_assert_r(condition)				(condition)

#define avsys_debug(owner, msg, args...)			fprintf(stderr, msg, ##args)
#define avsys_info(owner, msg, args...)			fprintf(stderr, msg, ##args)
#define avsys_warning(owner, msg, args...)		fprintf(stderr, msg, ##args)
#define avsys_error(owner, msg, args...)		fprintf(stderr, msg, ##args)
#define avsys_critical(owner, msg, args...)		fprintf(stderr, msg, ##args)
#define avsys_assert(condition)					(condition)

#endif	/* __USE_LOGMANAGER__ */

#else	/* __DEBUG_MODE__ */

#define avsys_debug_r(owner, msg, args...)
#define avsys_info_r(owner, msg, args...)
#define avsys_warning_r(owner, msg, args...)
#define avsys_error_r(owner, msg, args...)
#define avsys_critical_r(owner, msg, args...)
#define avsys_assert_r(condition)				(condition)

#define avsys_debug(owner, msg, args...)
#define avsys_info(owner, msg, args...)
#define avsys_warning(owner, msg, args...)
#define avsys_error(owner, msg, args...)
#define avsys_critical(owner, msg, args...)
#define avsys_assert(condition)					(condition)

#endif  /* __DEBUG_MODE__ */

#endif /* __AVSYS_DEBUG_H__ */
