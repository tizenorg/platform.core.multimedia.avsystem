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

#ifndef __AVSYS_AUDIO_H__
#define __AVSYS_AUDIO_H__

#include "avsys-types.h"
#include "avsys-error.h"

#ifdef __cplusplus
	extern "C" {
#endif

/**
	@addtogroup AVSYSTEM
	@{

	@par
	This part describes the interface with audio input/output.
 */

#define AVSYS_AUDIO_VOLUME_MAX_MULTIMEDIA	16
#define AVSYS_AUDIO_VOLUME_MAX_BASIC		8
#define AVSYS_AUDIO_VOLUME_MAX_SINGLE		1

/**
 * Enumerations for audio mode
 */
enum avsys_audio_mode_t {
	AVSYS_AUDIO_MODE_OUTPUT,				/**< Output mode of handle */
	AVSYS_AUDIO_MODE_OUTPUT_CLOCK,			/**< Output mode of gst audio only mode */
	AVSYS_AUDIO_MODE_OUTPUT_VIDEO,			/**< Output mode of gst video mode */
	AVSYS_AUDIO_MODE_OUTPUT_LOW_LATENCY,	/**< Output mode for low latency play mode. typically for game */
	AVSYS_AUDIO_MODE_INPUT,					/**< Input mode of handle */
	AVSYS_AUDIO_MODE_INPUT_HIGH_LATENCY,	/**< Input mode for high latency capture mode. */
	AVSYS_AUDIO_MODE_INPUT_LOW_LATENCY,		/**< Input mode for low latency capture mode. typically for VoIP */
	AVSYS_AUDIO_MODE_CALL_OUT,				/**< for voice call establish */
	AVSYS_AUDIO_MODE_CALL_IN,				/**< for voice call establish */
	AVSYS_AUDIO_MODE_OUTPUT_AP_CALL,		/**< for VT call on thin modem */
	AVSYS_AUDIO_MODE_INPUT_AP_CALL,			/**< for VT call on thin modem */
	AVSYS_AUDIO_MODE_NUM,					/**< Number of mode */
};

/**
 * Enumerations for audio format
 */
enum avsys_audio_format_t {
	AVSYS_AUDIO_FORMAT_UNKNOWN	= -1,					/**< Invalid audio format */
	AVSYS_AUDIO_FORMAT_8BIT,							/**< Unsigned 8Bit */
	AVSYS_AUDIO_FORMAT_16BIT,							/**< Signed 16bit Little Endian */
	AVSYS_AUDIO_FORMAT_MIN = AVSYS_AUDIO_FORMAT_8BIT,	/**< Minimum value 8-bit integer per sample */
	AVSYS_AUDIO_FORMAT_MAX = AVSYS_AUDIO_FORMAT_16BIT,	/**< Maximum value 16-bit integer per sample */
};


/*
 * Enums for volume types
 */
enum avsys_audio_volume_type_t {
	AVSYS_AUDIO_VOLUME_TYPE_SYSTEM,
	AVSYS_AUDIO_VOLUME_TYPE_NOTIFICATION,
	AVSYS_AUDIO_VOLUME_TYPE_ALARM,
	AVSYS_AUDIO_VOLUME_TYPE_RINGTONE,
	AVSYS_AUDIO_VOLUME_TYPE_MEDIA,
	AVSYS_AUDIO_VOLUME_TYPE_CALL,
	AVSYS_AUDIO_VOLUME_TYPE_FIXED,
	AVSYS_AUDIO_VOLUME_TYPE_EXT_SYSTEM_JAVA,
	AVSYS_AUDIO_VOLUME_TYPE_MEDIA_HL,
	AVSYS_AUDIO_VOLUME_TYPE_NUM,
	AVSYS_AUDIO_VOLUME_TYPE_MAX = AVSYS_AUDIO_VOLUME_TYPE_NUM,
	AVSYS_AUDIO_VOLUME_TYPE_EXT_SYSTEM_ANDROID = AVSYS_AUDIO_VOLUME_TYPE_FIXED,
};

enum avsys_audio_priority_t {
	AVSYS_AUDIO_PRIORITY_0 = 0,	/*< deprecated. use AVSYS_AUDIO_PRIORITY_NORMAL or AVSYS_AUDIO_PRIORITY_SOLO instead */
	AVSYS_AUDIO_PRIORITY_NORMAL = AVSYS_AUDIO_PRIORITY_0,
	AVSYS_AUDIO_PRIORITY_SOLO,
	AVSYS_AUDIO_PRIORITY_SOLO_WITH_TRANSITION_EFFECT,
	AVSYS_AUDIO_PRIORITY_MAX,
};

enum avsys_audio_device_t {
	AVSYS_AUDIO_DEVICE_TYPE_SPK = 0,
	AVSYS_AUDIO_DEVICE_TYPE_RECV,
	AVSYS_AUDIO_DEVICE_TYPE_HEADSET,
	AVSYS_AUDIO_DEVICE_TYPE_HANDSFREE,
	AVSYS_AUDIO_DEVICE_TYPE_BT,
	AVSYS_AUDIO_DEVICE_TYPE_NUM,
	AVSYS_AUDIO_DEVICE_TYPE_MAX = AVSYS_AUDIO_DEVICE_TYPE_NUM,
};

/**
 * Enumerations for audio channel
 */
enum avsys_audio_channel_t {
	AVSYS_AUDIO_CHANNEL_LEFT,			/**< front-left channel */
	AVSYS_AUDIO_CHANNEL_RIGHT,			/**< front-righth channel */
	AVSYS_AUDIO_CHANNEL_NUM,			/**< Number of channels */
};

/**
 * Enumerations for audio mute condition
 */
enum avsys_audio_mute_t {
	AVSYS_AUDIO_UNMUTE = 0,				/**< Unmute state */
	AVSYS_AUDIO_MUTE,					/**< Mute state */
	AVSYS_AUDIO_UNMUTE_NOLOCK,			/** < Unmute without lock */
	AVSYS_AUDIO_MUTE_NOLOCK,			/** < Mute without lock */
};

/**
 * Enumerations for priority command
 */
enum avsys_audio_priority_cmd_t {
	AVSYS_AUDIO_SET_PRIORITY = 0,		/**< Command set priority */
	AVSYS_AUDIO_UNSET_PRIORITY,			/**< Command unset priority */
};

/**
 * Structure for sound device parameters.
 */
typedef struct {
	int		mode;						/**< Open mode (In/Out) */
	int		priority;					/**< Priority of sound handle */
	int		channels;					/**< Number of channels */
	int		samplerate;					/**< Sampling rate */
	int		format;						/**< Sampling format */
	int		handle_route;				/**< Handle route information. refer. avsys_audio_handle_route_t */
	int		vol_type;					/**< volume type */
	int		allow_mix;
} avsys_audio_param_t;


/**
 * Structure for volume information.
 */
typedef struct {
	int	level[AVSYS_AUDIO_CHANNEL_NUM];	/**< Array of volume level for each channel */
} avsys_audio_volume_t;

/* NEW PATH DEFINE */
 /**
  *
  */
enum avsys_audio_path_ex {
	AVSYS_AUDIO_PATH_EX_NONE = 0,
	AVSYS_AUDIO_PATH_EX_SPK,
	AVSYS_AUDIO_PATH_EX_RECV,
	AVSYS_AUDIO_PATH_EX_HEADSET,
	AVSYS_AUDIO_PATH_EX_BTHEADSET,
	AVSYS_AUDIO_PATH_EX_A2DP,
	AVSYS_AUDIO_PATH_EX_HANDSFREE,
	AVSYS_AUDIO_PATH_EX_HDMI,
	AVSYS_AUDIO_PATH_EX_DOCK,
	AVSYS_AUDIO_PATH_EX_USBAUDIO,
	AVSYS_AUDIO_PATH_EX_OUTMAX,
	AVSYS_AUDIO_PATH_EX_MIC = 1,
	AVSYS_AUDIO_PATH_EX_HEADSETMIC,
	AVSYS_AUDIO_PATH_EX_BTMIC,
	AVSYS_AUDIO_PATH_EX_FMINPUT,
	AVSYS_AUDIO_PATH_EX_HANDSFREEMIC,
	AVSYS_AUDIO_PATH_EX_INMAX,
};

enum avsys_audio_gain_ex {
	AVSYS_AUDIO_GAIN_EX_KEYTONE = 0,
	AVSYS_AUDIO_GAIN_EX_RINGTONE,
	AVSYS_AUDIO_GAIN_EX_ALARMTONE,
	AVSYS_AUDIO_GAIN_EX_CALLTONE,
	AVSYS_AUDIO_GAIN_EX_AUDIOPLAYER,
	AVSYS_AUDIO_GAIN_EX_VIDEOPLAYER,
	AVSYS_AUDIO_GAIN_EX_VOICECALL,
	AVSYS_AUDIO_GAIN_EX_VIDEOCALL,
	AVSYS_AUDIO_GAIN_EX_FMRADIO,
	AVSYS_AUDIO_GAIN_EX_VOICEREC,
	AVSYS_AUDIO_GAIN_EX_CAMCORDER,
	AVSYS_AUDIO_GAIN_EX_CAMERA,
	AVSYS_AUDIO_GAIN_EX_GAME,
	AVSYS_AUDIO_GAIN_EX_MAX,
	AVSYS_AUDIO_GAIN_EX_PDA_PLAYBACK = AVSYS_AUDIO_GAIN_EX_KEYTONE,
};

typedef enum {
	AVSYS_AUDIO_EXT_DEVICE_FMRADIO = 0,
	AVSYS_AUDIO_EXT_DEVICE_NUM
}avsysaudio_ext_device_t;


typedef enum {
	AVSYS_AUDIO_ROUTE_POLICY_DEFAULT,
	AVSYS_AUDIO_ROUTE_POLICY_IGNORE_A2DP,
	AVSYS_AUDIO_ROUTE_POLICY_HANDSET_ONLY,
	AVSYS_AUDIO_ROUTE_POLICY_MAX,
}avsys_audio_route_policy_t; /* system global configuration */

enum avsys_audio_handle_route_t{
	AVSYS_AUDIO_HANDLE_ROUTE_FOLLOWING_POLICY,
	AVSYS_AUDIO_HANDLE_ROUTE_HANDSET_ONLY,
}; /* custom routing per handle */

typedef enum {
	AVSYS_AUDIO_ROUTE_DEVICE_UNKNOWN = -1,
	AVSYS_AUDIO_ROUTE_DEVICE_HANDSET,
	AVSYS_AUDIO_ROUTE_DEVICE_BLUETOOTH,
	AVSYS_AUDIO_ROUTE_DEVICE_EARPHONE,
	AVSYS_AUDIO_ROUTE_DEVICE_NUM,
}avsys_audio_playing_devcie_t;/* routing device */

/* path option */
#define AVSYS_AUDIO_PATH_OPTION_NONE			0x00000000	/*!< Sound path option none */
#define AVSYS_AUDIO_PATH_OPTION_DUAL_OUT		0x00000002	/*!< SPK or Recv with headset sound path. used for Ringtone or Alarm */
#define AVSYS_AUDIO_PATH_OPTION_VOICECALL_REC	0x00000010	/*!< Voice call recording path option */
#define AVSYS_AUDIO_PATH_OPTION_USE_SUBMIC		0x00000020	/*!< Use sub-mic when call or recording */
#define AVSYS_AUDIO_PATH_OPTION_USE_STEREOMIC	0x00000040	/*!< Use stereo mic when recording */
#define AVSYS_AUDIO_PATH_OPTION_FORCED			0x01000000	/*!< Forced sound path setting. only for booting animation */


/**
 * This function make instance for audio system, and retreives handle.
 *
 * @param	param		[in]	Parameters of audio system.
 * @param	phandle		[out]	Handle of audio system.
 * @param	size		[out]	Recomended buffer size.
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 *			on failure.
 * @remark
 * @see
 */
int avsys_audio_open(avsys_audio_param_t *param, avsys_handle_t *phandle, int *size);

/**
 * This function is to close sound handle and release allocated resources.
 *
 * @param	handle		[in]	Handle of audio system
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see
 */
int avsys_audio_close(avsys_handle_t handle);

/**
 * This function is to stop playback stream immediately. this drops all buffer remaining data.
 *
 * @param	handle		[in]	Playback handle of audio system
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see
 */
int avsys_audio_flush(avsys_handle_t handle);

/**
 * This function is to stop playback stream after all remaining buffer data played.
 *
 * @param	handle		[in]	Playback handle of audio system
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see
 */
int avsys_audio_drain(avsys_handle_t handle);

/**
 * This function is turn on speaker amp.
 *
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see
 */
int avsys_audio_ampon(void) __attribute__((deprecated));

/**
 * This function is turn off speaker amp.
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see
 */
int avsys_audio_ampoff(void) __attribute__((deprecated));

/**
 * This function is to read pcm data from sound handle.
 *
 * @param	handle		[in]	Handle of audio system
 * @param	buf			[in]	Buffer of audio data to read
 * @param	size		[in]	Size of buffer
 *
 * @return	This function returns number of bytes read, or negative value with
 *			error code on failure.
 * @remark
 * @see
 */
int avsys_audio_read(avsys_handle_t handle, void *buf, int size);

/**
 * This function is to write audio data to sound handle.
 *
 * @param	handle		[in]	Handle of audio system
 * @param	buf			[in]	Buffer of audio data to write
 * @param	size		[in]	Size of buffer
 *
 * @return	This function returns number of bytes written, or negative value
 *			with error code on failure.
 * @remark
 * @see
 */
int avsys_audio_write(avsys_handle_t handle, void *buf, int size);

int avsys_audio_set_volume_table(int gain_type, int dev_type, int step, int lv, int rv);
int avsys_audio_get_volume_table(int gain_type, int dev_type, int step, int *lv, int *rv);

int avsys_audio_set_volume_fadeup(avsys_handle_t handle);

/**
 * This function is to get volume max.
 *
 * @param	vol_type		[in]	Type of volume table
 * @param	max			[out]	number of volume steps
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see
 */
int avsys_audio_get_volume_max_ex(int volume_table, int *max_step);

/**
 * This function is to set relative mute of sound handle.
 *
 * @param	handle		[in]	Handle of audio system
 * @param	mute		[in]	Mute information to set
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 *  mute is AVSYS_AUDIO_MUTE : mute
 *  mute is AVSYS_AUDIO_UNMUTE : unmute
 * @see
 */
int avsys_audio_set_mute(avsys_handle_t handle, int mute);


/**
 * This function is to set mute of sound handle with fade out effect.
 *
 * @param	handle		[in]	Handle of audio system
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see
 */
int avsys_audio_set_mute_fadedown(avsys_handle_t handle);

/**
 * This function is to get relative mute of sound handle.
 *
 * @param	handle		[in]	Handle of audio system
 * @param	pmute		[out] Pointer to mute information to retrieve
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 *  pmute is AVSYS_AUDIO_MUTE : mute
 *  pmute is AVSYS_AUDIO_UNMUTE : unmute
 * @see	avsys_audio_mute_t
 */
int avsys_audio_get_mute(avsys_handle_t handle, int* pmute);


/**
 * This function is to set amp on external device
 *
 * @param	device		[in]	External audio device type
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 *  mute is AVSYS_AUDIO_MUTE : mute
 *  mute is AVSYS_AUDIO_UNMUTE : unmute
 * @see avsysaudio_ext_device_t
 */
int avsys_audio_ext_device_ampon(avsysaudio_ext_device_t device);

/**
 * This function is to set amp off external device
 *
 * @param	device		[in]	External audio device type
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 *  pmute is AVSYS_AUDIO_MUTE : mute
 *  pmute is AVSYS_AUDIO_UNMUTE : unmute
 * @see avsysaudio_ext_device_t
 */
int avsys_audio_ext_device_ampoff(avsysaudio_ext_device_t device);

/**
 * This function is to set status of external device
 *
 * @param	device		[in]	External audio device type
 * @param	onoff		[in]	1 for on , 0 for off
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see avsysaudio_ext_device_t
 */
int avsys_audio_set_ext_device_status(avsysaudio_ext_device_t device_type, int onoff);

/**
 * This function is to get status of external device
 *
 * @param	device		[in]	External audio device type
 * @param	onoff		[out]	1 for on , 0 for off
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see avsys_audio_set_ext_device_status
 */
int avsys_audio_get_ext_device_status(avsysaudio_ext_device_t device_type, int *onoff);
/**
 * This function is to set sound path of sound device.
 *
 * @param	path		[in]	value of sound gain
 * @param	output		[in]	value of output device
 * @param	input		[in]	value of input device
 * @param	option		[in]	value of sound path option
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 *			AVSYS_STATE_ERR_RANGE_OVER : input path value is invalid range
 *			AVSYS_STATE_ERR_INTERNAL   : internal device error
 * @remark
 * @see
 */
int avsys_audio_set_path_ex(int gain, int out, int in, int option);

/**
 * This function is to get sound path of sound device.
 *
 * @param	path		[out]	value of sound gain
 * @param	output		[out]	value of output device
 * @param	input		[out]	value of input device
 * @param	option		[out]	value of sound path option
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 *			AVSYS_STATE_ERR_INTERNAL   : internal device error
 * @remark
 * @see
 */
int avsys_audio_get_path_ex(int *gain, int *out, int *in, int *option);

/**
 * This function is to set relative global mute of sound device.
 *
 * @param	mute		[in]	Global mute information to retreive
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 *  mute is avsys_audio_mute_disable : unmute
 *  mute is avsys_audio_mute_enable  : mute
 *  others     : ignore
 * @see avsys_audio_mute_t
 */
int avsys_audio_set_global_mute(int mute);

/**
 * This function is to get relative global mute of sound device.
 *
 * @param	pmute		[out]	Pointer to global mute information to retreive
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * *pbmute is avsys_audio_mute_disable or avsys_audio_mute_enable
 * @see
 */
int avsys_audio_get_global_mute(int* pmute);

/**
 * This function is to get number of remaining audio frames in hw buffer.
 *
 * @param	handle		[in]	handle to get delay frames
 * @param	frame_delay	[out]	value of number of remaining audio frames
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see
 */
int avsys_audio_delay(avsys_handle_t handle, int *frame_delay);

/**
 * This function is to reset audio stream. (drops all remaining audio frames in  device buffer)
 *
 * @param	handle		[in]	handle to reset
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see
 */
int avsys_audio_reset(avsys_handle_t handle);

/**
 * This function is to get period time and buffer time of audio stream.
 *
 * @param	handle		[in]	handle to get period & buffer time
 * @param	period_time	[out]	value of period time in microsecond.
 * @param	buffer_time	[out]	value of buffer time in microsecond.
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see
 */
int avsys_audio_get_period_buffer_time(avsys_handle_t handle, unsigned int *period_time, unsigned int *buffer_time);

/**
 * This function is to cork stream.
 *
 * @param	handle		[in]	handle to cork
 * @param	cork		[in]	cork=1, uncork=0
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see
 */
int avsys_audio_cork (avsys_handle_t handle, int cork);

/**
 * This function is to check whether stream is corked or not.
 *
 * @param	handle		[in]	handle to cork
 * @param	is_corked	[out]	corked is 1, otherwise 0
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see
 */
int avsys_audio_is_corked (avsys_handle_t handle, int *is_corked);

/**
 * This function is to get audio capturing status of system.
 *
 * @param	on_capture	[out]	capture status.
 * 								zero if there is no capture instance.
 * 								positive value if there is capture instance
 *
 * @return	This function returns AVSYS_STATE_SUCCESS on success, or negative
 *			value with error code.
 * @remark
 * @see
 */
int avsys_audio_get_capture_status(int *on_capture);

int avsys_audio_earjack_manager_init(int *earjack_type, int *waitfd);
int avsys_audio_earjack_manager_wait(int waitfd, int *current_earjack_type, int *new_earjack_type, int *need_mute);
int avsys_audio_earjack_manager_process(int new_earjack_type);
int avsys_audio_earjack_manager_deinit(int waitfd);
int avsys_audio_earjack_manager_get_type(void);
int avsys_audio_earjack_manager_unlock(void);

int avsys_audio_set_route_policy(avsys_audio_route_policy_t route);
int avsys_audio_get_route_policy(avsys_audio_route_policy_t *route);
int avsys_audio_get_current_playing_volume_type(int *volume_type);
int avsys_audio_set_volume_by_type(const int volume_type, const int volume_value);
int avsys_audio_set_primary_volume(const int pid, const int type);
int avsys_audio_clear_primary_volume(const int pid);
int avsys_audio_hibernation_reset(int *vol);
/**
	@}
 */

#ifdef __cplusplus
	}
#endif

#endif /* __AVSYS_AUDIO_H__ */
