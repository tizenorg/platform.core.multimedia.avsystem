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
#include <unistd.h>
#include <alloca.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <avsys-audio.h>

#define VERSION	(0001)

void usage(char *name)
{
	printf("Usage: %s [OPTION]... [FILE]...\n\n", name);
	printf("-h\t\thelp\n");
	printf("-p\t\tplay raw file (default mode 0) \n");
	printf("-c\t\tcapture audio to raw file (default mode 0)\n");
	printf("-m\t\tmode\n");
	printf("\t\tmode 0 : S16LE / Stereo / 44100Hz\n");
	printf("\t\tmode 1 : S16LE / Mono / 8000Hz\n");
	printf("\t\tmode 2 : S16LE / Mono / 22050Hz\n");
	printf("\t\tmode 3 : U8 / Mono / 8000Hz\n");
	printf("-r\t\trouting\n");
	printf("\t\trouting 0 : following policy\n");
	printf("\t\trouting 1 : forced set to alsa\n");
	printf("\n");
	return;
}

enum {
	OP_NONE = -1,
	OP_PLAYBACK,
	OP_CAPTURE,
};

#define MODE_MIN 0
#define MODE_MAX 3

static struct sigaction sigterm_action; /* Backup pointer of SIGTERM signal handler */
int g_interrupted;

int __make_param(int op, int mode, int routing, avsys_audio_param_t *param)
{
	if (!param)
		return -1;

	if (op == OP_PLAYBACK)
		param->mode = AVSYS_AUDIO_MODE_OUTPUT;
	else if (op == OP_CAPTURE)
		param->mode = AVSYS_AUDIO_MODE_INPUT;
	else
		return -1;

	param->priority = AVSYS_AUDIO_PRIORITY_NORMAL;
	param->vol_type = AVSYS_AUDIO_VOLUME_TYPE_SYSTEM;
	param->bluetooth = routing;

	switch (mode) {
	case 0:
		param->format = AVSYS_AUDIO_FORMAT_16BIT;
		param->samplerate = 44100;
		param->channels = 2;
		break;
	case 1:
		param->format = AVSYS_AUDIO_FORMAT_16BIT;
		param->samplerate = 8000;
		param->channels = 1;
		break;
	case 2:
		param->format = AVSYS_AUDIO_FORMAT_16BIT;
		param->samplerate = 22050;
		param->channels = 1;
		break;
	case 3:
		param->format = AVSYS_AUDIO_FORMAT_8BIT;
		param->samplerate = 8000;
		param->channels = 1;
		break;
	default:
		return -1;
		break;
	}
	return 0;
}
int _playback(int mode, int routing, char *filename)
{
	int res = 0;
	avsys_audio_param_t param;
	avsys_handle_t handle = -1;
	int recommended_period_size = 0;
	int fd = -1;
	char *pcmbuf = NULL;
	int size = 0;

	if (!filename)
		return -1;

	memset(&param, sizeof(avsys_audio_param_t), '\0');

	if (__make_param(OP_PLAYBACK, mode, routing, &param)) {
		printf("Can not make audio parameter\n");
		return -1;
	}

	res = avsys_audio_open(&param, &handle, &recommended_period_size);
	if (res != 0) {
		printf("Can not open handle 0x%x\n", res);
		goto FAIL;
	}

	pcmbuf = alloca(recommended_period_size);
	if (!pcmbuf)
		goto FAIL;

	fd = open(filename, O_RDONLY);
	if (fd == -1)
		goto FAIL;

	while (((size = read(fd, pcmbuf, recommended_period_size)) > 0) && !g_interrupted) {
		if (AVSYS_FAIL(avsys_audio_write(handle, pcmbuf, recommended_period_size))) {
			printf("Oops!! audio play fail\n");
			break;
		}
	}

	close(fd);
	avsys_audio_close(handle);
	return 0;
FAIL:
	if (handle != -1)
		avsys_audio_close(handle);

	if (fd != -1)
		close(fd);

	return res;
}

int _capture(int mode, int routing, char *filename)
{
	int res = 0;
	avsys_audio_param_t param;
	avsys_handle_t handle = -1;
	int recommended_period_size = 0;
	int fd = -1;
	char *pcmbuf = NULL;
	int size = 0;
	char namebuffer[64]= {0,};

	if (!filename)
		return -1;
	else
		snprintf(namebuffer, sizeof(namebuffer)-1, "%s.raw_%1d", filename, mode);

	printf("[%s] real filename :%s\n", __func__, namebuffer);

	memset(&param, sizeof(avsys_audio_param_t), '\0');

	if (__make_param(OP_CAPTURE, mode, routing, &param)) {
		printf("Can not make audio parameter\n");
		return -1;
	}

	res = avsys_audio_open(&param, &handle, &recommended_period_size);
	if (res != 0) {
		printf("Can not open handle 0x%x\n", res);
		goto FAIL;
	}

	pcmbuf = alloca(recommended_period_size);
	if (!pcmbuf) {
		printf("Can not alloca pcm buffer\n");
		goto FAIL;
	}

	fd = open(namebuffer, O_WRONLY | O_CREAT);
	if (fd == -1) {
		printf("Can not open file %s, %s\n", namebuffer, strerror(errno));
		goto FAIL;
	}

	while (((size = avsys_audio_read(handle, pcmbuf, recommended_period_size)) > 0) && !g_interrupted) {
		if (-1 == write(fd, pcmbuf, size))
			break;
	}

	close(fd);
	avsys_audio_close(handle);
	return 0;
FAIL:
	printf("[%s] FAIL\n",__func__);
	if (handle != -1)
		avsys_audio_close(handle);

	if (fd != -1)
		close(fd);

	return res;
}

void sig_handler(int signo)
{
	g_interrupted = 1;
}

int main(int argc, char *argv[])
{
	int opt = 0;
	int operation = OP_NONE;
	int mode = 0;
	int tmp = 0;
	int routing = 0;
	struct sigaction action;

	while ((opt = getopt(argc, argv, "hpcm:r:")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			return 0;
			break;
		case 'p':
			if (operation != OP_NONE) {
				usage(argv[0]);
				return 0;
			}
			operation = OP_PLAYBACK;
			break;
		case 'c':
			if (operation != OP_NONE) {
				usage(argv[0]);
				return 0;
			}
			operation = OP_CAPTURE;
			break;
		case 'm':
			tmp = atoi(optarg);
			if (tmp < MODE_MIN || tmp > MODE_MAX) {
				usage(argv[0]);
				printf("MESSAGE : Unsupported mode number\n");
				return -1;
			}
			mode = tmp;
			break;
		case 'r':
			tmp = atoi(optarg);
			if (tmp < 0 || tmp > 1) {
				usage(argv[0]);
				printf("MESSAGE : Unsupported mode number\n");
				return -1;
			}
			routing = tmp;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	action.sa_handler = sig_handler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGINT, &action, &sigterm_action);

	if (operation == OP_NONE) {
		usage(argv[0]);
		printf("MESSAGE : Operation is not determined\n");
		return -1;
	} else if (operation == OP_PLAYBACK) {
		char *name = NULL;
		name = argv[optind++];
		printf("op %u, mode %u, routing %d, name %s\n", operation, mode, routing, name);
		_playback(mode, routing, name);
	} else if (operation == OP_CAPTURE) {
		char *name = NULL;
		name = argv[optind++];
		if (strlen(name) < 2) {
			printf("Insufficient filename length\n");
			return -2;
		}
		printf("op %u, mode %u, routing %d, name %s\n", operation, mode, routing, name);
		_capture(mode, routing, name);
	}

	return 0;
}
