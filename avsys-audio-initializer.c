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
#include <unistd.h>
#include <signal.h>

#include "include/avsys-error.h"
#include "include/avsys-debug.h"

#include "include/avsys-audio-handle.h"
#include "include/avsys-audio-path.h"
#include "include/avsys-audio-logical-volume.h"

#define OP_NONE -1
#define OP_USAGE 0
#define OP_INIT 1
#define OP_UNINIT 2
#define OP_RESET 3
#define OP_DUMP 4
#define OP_DAEMON 5
#define OP_AMP 6
#define OP_SHUTDOWN 8
#define OP_REJUVE 9

static int usage(int argc, char *argv[]);
static int get_options(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	int operation = OP_NONE;
	int result = 0;
	int mute = 1;
	pid_t pid;
	mode_t old_umask = 0;

	static char *str_errormsg[] = {
		"Operation is success.",
		"Handle Init Fail",
		"Path Init Fail",
		"Handle Fini Fail",
		"Path Fini Fail",
		"Handle Reset Fail",
		"Path Reset Fail",
		"Handle Dump Fail",
		"Path Dump Fail",
		"Global Mute Fail",
		"Handle Rejuvenation Fail",
		"Vconf Get Value Fail",
		"Sync Dump Fail",
	};

	operation = get_options(argc, argv);

	switch (operation) {
	case OP_INIT:
		old_umask = umask(0);
		fprintf(stderr, "old umask was [%o]\n", old_umask);
		result = avsys_audio_handle_init();
		if (AVSYS_FAIL(result)) {
			result = 1;
			umask(old_umask);
			fprintf(stderr, "set umask to old value\n");
			break;
		}
		result = avsys_audio_path_ex_init();
		if (AVSYS_FAIL(result)) {
			result = 2;
			umask(old_umask);
			fprintf(stderr, "set umask to old value\n");
			break;
		}
		result = avsys_audio_logical_volume_init();
		if (AVSYS_FAIL(result)) {
			result = 2;
			umask(old_umask);
			fprintf(stderr, "set umask to old value\n");
			break;
		}
		umask(old_umask);
		fprintf(stderr, "set umask to old value\n");
		break;

	case OP_UNINIT:
		result = avsys_audio_handle_fini();
		if (AVSYS_FAIL(result)) {
			result = 3;
			break;
		}

		result = avsys_audio_path_ex_fini();
		if (AVSYS_FAIL(result)) {
			result = 4;
			break;
		}
		break;
	case OP_RESET:
		result = avsys_audio_handle_reset(NULL);
		if (AVSYS_FAIL(result)) {
			result = 5;
			break;
		}

		result = avsys_audio_path_ex_reset(0);
		if (AVSYS_FAIL(result)) {
			result = 6;
			break;
		}
		break;
	case OP_DUMP:
		result = avsys_audio_handle_dump();
		if (AVSYS_FAIL(result)) {
			result = 7;
			break;
		}
		result = avsys_audio_path_ex_dump();
		if (AVSYS_FAIL(result)) {
			result = 8;
			break;
		}

		result = avsys_audio_dump_sync();
		if (AVSYS_FAIL(result)) {
			result = 12;
			break;
		}
		break;
	case OP_DAEMON:
		result = 0;
		break;
	case OP_USAGE:
		break;
	case OP_SHUTDOWN:
		result = avsys_audio_path_ex_set_mute(mute);
		if (AVSYS_FAIL(result)) {
			result = 9;
		}
		break;
	case OP_REJUVE:
		result = avsys_audio_handle_rejuvenation();
		if (AVSYS_FAIL(result)) {
			result = 10;
		}
		break;
	default:
		fprintf(stderr, "Unknown operation\n");
		break;
	}

	if (result != 0) {
		fprintf(stderr, "%s\n", str_errormsg[result]);
		return 1;
	} else
		return 0;
}

static int get_options(int argc, char *argv[])
{
	int ch = 0;
	int operation = OP_NONE;

	if (argc != 2)
		return usage(argc, argv);

	while ((ch = getopt(argc, argv, "iurtdmjhap")) != EOF) {
		switch (ch) {
		case 'i':
			if (operation != OP_NONE)
				return usage(argc, argv);
			operation = OP_INIT;
			break;
		case 'u':
			if (operation != OP_NONE)
				return usage(argc, argv);
			operation = OP_UNINIT;
			break;
		case 'r':
			if (operation != OP_NONE)
				return usage(argc, argv);
			operation = OP_RESET;
			break;
		case 'd':
			if (operation != OP_NONE)
				return usage(argc, argv);
			operation = OP_DUMP;
			break;
		case 'h':
			if (operation != OP_NONE)
				return usage(argc, argv);
			operation = OP_DAEMON;
			break;
		case 'm':
			if (operation != OP_NONE)
				return usage(argc, argv);
			operation = OP_SHUTDOWN;
			break;
		case 'j':
			if (operation != OP_NONE)
				return usage(argc, argv);
			operation = OP_REJUVE;
			break;
		default:
			return usage(argc, argv);
		}
		if (optind != argc)
			return usage(argc, argv);
	}
	return operation;
}

static int usage(int argc, char *argv[])
{
	fprintf(stderr, "Usage : %s option\n", argv[0]);
	fprintf(stderr, "[OPTIONS]\n");
	fprintf(stderr, "  -i : Initialize audio system\n");
	fprintf(stderr, "  -u : Uninitialize audio system\n");
	fprintf(stderr, "  -r : Reset audio system\n");
	fprintf(stderr, "  -d : Dump audio system\n");
	fprintf(stderr, "  -m : Global mute\n");
	fprintf(stderr, "  -j : Handle rejuvenation\n");
	return OP_USAGE;
}

