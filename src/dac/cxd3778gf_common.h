/*
 * cxd3778gf_common.h
 *
 * CXD3778GF CODEC driver
 *
 * Copyright 2013-2016, 2017 Sony Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _CXD3778GF_COMMON_HEADER_
#define _CXD3778GF_COMMON_HEADER_

/* #define TRACE_PRINT_ON */
/* #define DEBUG_PRINT_ON */

#define CODEC_RAM_WORD_SIZE 5
#define CODEC_RAM_SIZE (CODEC_RAM_WORD_SIZE * 32 * 2)

/* for 44.1kHz */
#define CODEC_RAM_441_AREA 0x00 /* off       */
/* for 48kHz */
#define CODEC_RAM_480_AREA 0x20 /* air plane */

/***************/
/* definitions */
/***************/

/* basic */
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#define OFF 0
#define ON 1
#define LOW 0
#define HIGH 1

/* nc headphone type */
#define NCHP_TYPE_NW750N 0
#define NCHP_TYPE_NC31 1
#define NCHP_TYPE_NW500N 2
#define NCHP_TYPE_OTHER 3
#define NCHP_TYPE_MAX 3

/* jack mode */
#define JACK_MODE_HEADPHONE 0
#define JACK_MODE_ANTENNA 1
#define JACK_MODE_MAX 1

/* tone control */
#define TONE_CONTROL_NON_HP 0
#define TONE_CONTROL_GENERAL_HP 1
#define TONE_CONTROL_NC_HP 2 /* add NCHP_TYPE_XX to use */
#define TONE_CONTROL_MAX (2 + NCHP_TYPE_MAX)

/* noise cancel mode */
#define NOISE_CANCEL_MODE_OFF 0
#define NOISE_CANCEL_MODE_AIRPLANE 1
#define NOISE_CANCEL_MODE_TRAIN 2
#define NOISE_CANCEL_MODE_OFFICE 3
#define NOISE_CANCEL_MODE_AINC 4
#define NOISE_CANCEL_MODE_AMBIENT1 5
#define NOISE_CANCEL_MODE_AMBIENT2 6
#define NOISE_CANCEL_MODE_AMBIENT3 7
#define NOISE_CANCEL_MODE_MAX 7

/* noise cancel active */
#define NC_NON_ACTIVE 0
#define NC_ACTIVE 1
#define AMBIENT 2
#define NC_ACTIVE_MAX 2

/* input device */
#define INPUT_DEVICE_NONE 0
#define INPUT_DEVICE_TUNER 1
#define INPUT_DEVICE_MIC 2
#define INPUT_DEVICE_LINE 3
#define INPUT_DEVICE_DIRECTMIC 4
#define INPUT_DEVICE_MAX 4

/* output device */
#define OUTPUT_DEVICE_NONE 0
#define OUTPUT_DEVICE_HEADPHONE 1
#define OUTPUT_DEVICE_LINE 2
#define OUTPUT_DEVICE_SPEAKER 3
#define OUTPUT_DEVICE_FIXEDLINE 4
#define OUTPUT_DEVICE_MAX 4

/* headphone amp */
#define HEADPHONE_AMP_NORMAL 0
#define HEADPHONE_AMP_SMASTER_SE 1
#define HEADPHONE_AMP_SMASTER_BTL 2
#define HEADPHONE_AMP_MAX 2

/* headphone smaster_gain_mode */
#define HEADPHONE_SMASTER_SE_GAIN_MODE_NORMAL 0
#define HEADPHONE_SMASTER_SE_GAIN_MODE_HIGH 1
#define HEADPHONE_SMASTER_SE_GAIN_MODE_MAX 1

#define HEADPHONE_SMASTER_BTL_GAIN_MODE_NORMAL 0
#define HEADPHONE_SMASTER_BTL_GAIN_MODE_HIGH 1
#define HEADPHONE_SMASTER_BTL_GAIN_MODE_MAX 1
/* jack status */
#define JACK_STATUS_SE_NONE 0
#define JACK_STATUS_SE_3PIN 1
#define JACK_STATUS_SE_4PIN 2
#define JACK_STATUS_SE_5PIN 3
#define JACK_STATUS_SE_ANTENNA 4
#define JACK_STATUS_SE_MAX 4

#define JACK_STATUS_BTL_NONE 0
#define JACK_STATUS_BTL 1
#define JACK_STATUS_BTL_MAX 1

/* master volume */
#define MASTER_VOLUME_MIN 0
#define MASTER_VOLUME_MAX 120

/* lr balance volume */
#define L_BALANCE_VOLUME_MAX 88
#define R_BALANCE_VOLUME_MAX 88

/* master gain */
#define MASTER_GAIN_MIN 0
#define MASTER_GAIN_MAX 30

/* base noise cancel gain index */
#define BASE_NOISE_CANCEL_GAIN_INDEX_MAX 50

/* user noise cancel / ambient gain index */
#define USER_DNC_GAIN_INDEX_DEFAULT 15
#define USER_DNC_GAIN_INDEX_MAX 30

/* external OSC */
#define EXTERNAL_OSC_441 0
#define EXTERNAL_OSC_480 1
#define EXTERNAL_OSC_KEEP 2
#define EXTERNAL_OSC_OFF 3

/* PCM or DSD mode */
#define PCM_MODE 0
#define DSD_MODE 1

/* headphone detect mode */
#define HEADPHONE_DETECT_POLLING 0
#define HEADPHONE_DETECT_SELECT 1
#define HEADPHONE_DETECT_INTERRUPT 2
#define HEADPHONE_DETECT_MAX 2

/* dnc block inactive mode */
#define DNC_INACTIVE_OFF 0
#define DNC1_INACTIVE 1
#define DNC2_INACTIVE 2
#define DNC_INACTIVE_MAX 2

/* PCM or DSD mode */
#define PLAYBACK_LATENCY_NORMAL 0
#define PLAYBACK_LATENCY_LOW 1
#define PLAYBACK_LATENCY_MAX 2

/* DSD timed mute params */
enum MUTE_ID {
    MUTE_ID_DEFAULT = 0,
    MUTE_ID_DSD064,
    MUTE_ID_DSD128,
    MUTE_ID_DSD256,
    MUTE_ID_MAX,
};
extern int cxd3778gf_timed_mute_ms[MUTE_ID_MAX];

/* basic macro */
#define minimum(_a, _b) ((_a) < (_b) ? (_a) : (_b))
#define maximum(_a, _b) ((_a) > (_b) ? (_a) : (_b))
// #define absolute(_a) ((_a) >= 0 ? (_a) : (-(_a)))

/***********/
/* headers */
/***********/

#endif
