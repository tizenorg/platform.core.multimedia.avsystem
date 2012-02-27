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

#ifndef __AVSYS_AUDIO_ASCENARIO_H__
#define __AVSYS_AUDIO_ASCENARIO_H__

typedef enum {
	ASCN_RESET_NONE = 0,
	ASCN_RESET_ALL,
	ASCN_RESET_PLAYBACK,
	ASCN_RESET_CAPTURE,
	ASCN_RESET_MODEM,
}AscnResetType;

#define ASCN_STR_RESET					"reset"
#define ASCN_STR_RESET_PLAYBACK			"reset_playback"
#define ASCN_STR_RESET_CAPTURE			"reset_capture"
#define ASCN_STR_PLAYBACK_MUTE			"mute_playback"
#define ASCN_STR_PLAYBACK_UNMUTE		"unmute_playback"
#define ASCN_CODEC_DISABLE_ON_SUSPEND	"codec_disable_on_suspend"

#define IN	0
#define OUT	16

#define INPUT_CH_0			((1 << (0 + IN)))	/* Main Mic.         0x00000001 */
#define INPUT_CH_1			((1 << (1 + IN)))	/* Sub Mic.          0x00000002 */
#define INPUT_CH_2			((1 << (2 + IN)))	/* Stereo Mic. ,     0x00000004 */
#define INPUT_CH_3			((1 << (3 + IN)))	/* Ear Mic. ,        0x00000008 */
#define INPUT_CH_4			((1 << (4 + IN)))	/* BT Mic.           0x00000010 */
#define INPUT_CH_5			((1 << (5 + IN)))	/* AP                0x00000020 */
#define INPUT_CH_6			((1 << (6 + IN)))	/* CP                0x00000040 */
#define INPUT_CH_7			((1 << (7 + IN)))	/* FM Radio          0x00000080 */
#define INPUT_CH_8			((1 << (8 + IN)))	/* Reserved */
#define INPUT_CH_9			((1 << (9 + IN)))	/* Reserved */

#define OUTPUT_CH_0			((1 << (0 + OUT)))	/* Headset (Earphone)    0x00010000 */
#define OUTPUT_CH_1			((1 << (1 + OUT)))	/* Left Speaker ,        0x00020000 */
#define OUTPUT_CH_2			((1 << (2 + OUT)))	/* Right Speaker ,       0x00040000 */
#define OUTPUT_CH_3			((1 << (3 + OUT)))	/* Stereo Speaker ,      0x00080000 */
#define OUTPUT_CH_4			((1 << (4 + OUT)))	/* Receiver (Mono) ,     0x00100000 */
#define OUTPUT_CH_5			((1 << (5 + OUT)))	/* BT Headset            0x00200000 */
#define OUTPUT_CH_6			((1 << (6 + OUT)))	/* CP                    0x00400000 */
#define OUTPUT_CH_7			((1 << (7 + OUT)))	/* AP                    0x00800000 */
#define OUTPUT_CH_8			((1 << (8 + OUT)))	/* Gain */
#define OUTPUT_CH_9			((1 << (9 + OUT)))	/* Video call gain */
#define OUTPUT_CH_10		((1 << (10 + OUT)))	/* Video call gain */
#define OUTPUT_CH_11		((1 << (11 + OUT)))	/* Reserved */
#define OUTPUT_CH_12		((1 << (12 + OUT)))	/* Reserved */
#define OUTPUT_CH_13		((1 << (13 + OUT)))	/* Call alert Gain */

#define INPUT_MAIN_MIC		(INPUT_CH_0)
#define INPUT_SUB_MIC		(INPUT_CH_1)
#define INPUT_STEREO_MIC	(INPUT_CH_2)
#define INPUT_EAR_MIC		(INPUT_CH_3)
#define INPUT_BT_MIC		(INPUT_CH_4)
#define INPUT_AP			(INPUT_CH_5)
#define INPUT_CP			(INPUT_CH_6)
#define INPUT_FMRADIO		(INPUT_CH_7)

#define OUTPUT_HEADSET		(OUTPUT_CH_0)
#define OUTPUT_LEFT_SPK		(OUTPUT_CH_1)
#define OUTPUT_RIGHT_SPK	(OUTPUT_CH_2)
#define OUTPUT_STEREO_SPK	(OUTPUT_CH_3)
#define OUTPUT_RECV			(OUTPUT_CH_4)
#define OUTPUT_BT_HEADSET	(OUTPUT_CH_5)
#define OUTPUT_CP			(OUTPUT_CH_6)
#define OUTPUT_AP			(OUTPUT_CH_7)

#define GAIN_MODE			(OUTPUT_CH_8)
#define GAIN_VIDEO_CALL		(OUTPUT_CH_9)
#define GAIN_VOICE_CALL		(OUTPUT_CH_10)
#define GAIN_CALLALERT		(OUTPUT_CH_13)

/* function prototype */
int avsys_audio_ascn_bulk_set(int * bulk, int bulk_cnt, AscnResetType clear);
int avsys_audio_ascn_single_set(char * str);

#endif /* __AVSYS_AUDIO_ASCENARIO_H__ */
