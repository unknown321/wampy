/*
 * Copyright 2013,2014,2015,2016,2017 Sony Corporation
 */
/*
 * cxd3778gf_table.h
 *
 * CXD3778GF CODEC driver
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

#ifndef _CXD3778GF_TABLE_HEADER_
#define _CXD3778GF_TABLE_HEADER_

#include "cxd3778gf_common.h"
#include <cstdio>
#include <string>

enum MASTER_VOLUME_VALUE {
    MASTER_VOLUME_VALUE_MIN = 0,
    MASTER_VOLUME_VALUE_LINEIN = 0,
    MASTER_VOLUME_VALUE_SDIN1 = 1,
    MASTER_VOLUME_VALUE_SDIN2,
    MASTER_VOLUME_VALUE_PLAY,
    MASTER_VOLUME_VALUE_LINEOUT,
    MASTER_VOLUME_VALUE_HPOUT,
    MASTER_VOLUME_VALUE_CMX1_500,
    MASTER_VOLUME_VALUE_CMX2_500,
    MASTER_VOLUME_VALUE_CMX1_750,
    MASTER_VOLUME_VALUE_CMX2_750,
    MASTER_VOLUME_VALUE_CMX1_31,
    MASTER_VOLUME_VALUE_CMX2_31,
    MASTER_VOLUME_VALUE_HPOUT2_CTRL3,
    MASTER_VOLUME_VALUE_MAX,
};

struct cxd3778gf_master_volume {
    unsigned char linein;
    unsigned char sdin1;
    unsigned char sdin2;
    unsigned char play;
    unsigned char lineout;
    unsigned char hpout;
    unsigned char cmx1_500;
    unsigned char cmx2_500;
    unsigned char cmx1_750;
    unsigned char cmx2_750;
    unsigned char cmx1_31;
    unsigned char cmx2_31;
    unsigned char hpout2_ctrl3;
};

struct cxd3778gf_device_gain {
    unsigned char pga;
    unsigned char adc;
};

struct cxd3778gf_deq_coefficient {
    unsigned char b0[3];
    unsigned char b1[3];
    unsigned char b2[3];
    unsigned char a1[3];
    unsigned char a2[3];
};

#define TABLE_ID_MASTER_VOLUME 0
#define TABLE_ID_DEVICE_GAIN 1
#define TABLE_ID_TONE_CONTROL 2

#define TABLE_ID_MASTER_VOLUME_DSD 3 // :unknown

#define MASTER_VOLUME_TABLE_OFF 0
#define MASTER_VOLUME_TABLE_SMASTER_SE_LG 1 // low gain
#define MASTER_VOLUME_TABLE_SMASTER_SE_HG 2 // high gain
#define MASTER_VOLUME_TABLE_SMASTER_SE_NC 3 // noise cancel
#define MASTER_VOLUME_TABLE_SMASTER_SE_AM 4 // ambient
#define MASTER_VOLUME_TABLE_SMASTER_BTL_50 5
#define MASTER_VOLUME_TABLE_SMASTER_BTL_100 6
#define MASTER_VOLUME_TABLE_CLASSAB 7
#define MASTER_VOLUME_TABLE_CLASSAB_NC 8
#define MASTER_VOLUME_TABLE_CLASSAB_AM 9
#define MASTER_VOLUME_TABLE_LINE 10
#define MASTER_VOLUME_TABLE_FIXEDLINE 11
#define MASTER_VOLUME_TABLE_SMASTER_SE_DSD64_LG 12
#define MASTER_VOLUME_TABLE_SMASTER_SE_DSD64_HG 13
#define MASTER_VOLUME_TABLE_SMASTER_SE_DSD128_LG 14
#define MASTER_VOLUME_TABLE_SMASTER_SE_DSD128_HG 15
#define MASTER_VOLUME_TABLE_SMASTER_SE_DSD256_LG 16
#define MASTER_VOLUME_TABLE_SMASTER_SE_DSD256_HG 17
#define MASTER_VOLUME_TABLE_SMASTER_BTL_50_DSD64 18
#define MASTER_VOLUME_TABLE_SMASTER_BTL_100_DSD64 19
#define MASTER_VOLUME_TABLE_SMASTER_BTL_50_DSD128 20
#define MASTER_VOLUME_TABLE_SMASTER_BTL_100_DSD128 21
#define MASTER_VOLUME_TABLE_SMASTER_BTL_50_DSD256 22
#define MASTER_VOLUME_TABLE_SMASTER_BTL_100_DSD256 23
#define MASTER_VOLUME_TABLE_LINE_DSD64 24
#define MASTER_VOLUME_TABLE_LINE_DSD128 25
#define MASTER_VOLUME_TABLE_LINE_DSD256 26
#define MASTER_VOLUME_TABLE_MAX 26

#define TONE_CONTROL_TABLE_NO_HP 0
#define TONE_CONTROL_TABLE_NAMP_GENERAL_HP 1
#define TONE_CONTROL_TABLE_NAMP_NW500N_NCHP 2
#define TONE_CONTROL_TABLE_NAMP_NW750N_NCHP 3
#define TONE_CONTROL_TABLE_NAMP_NC31_NCHP 4
#define TONE_CONTROL_TABLE_SAMP_GENERAL_HP 5
#define TONE_CONTROL_TABLE_SAMP_NW500N_NCHP 6
#define TONE_CONTROL_TABLE_SAMP_NW750N_NCHP 7
#define TONE_CONTROL_TABLE_SAMP_NC31_NCHP 8
#define TONE_CONTROL_TABLE_MAX 8

#define SOUND_EFFECT_ON 1
#define SOUND_EFFECT_OFF 0

extern struct cxd3778gf_master_volume cxd3778gf_master_volume_table[2][MASTER_VOLUME_TABLE_MAX + 1][MASTER_VOLUME_MAX + 1];
extern unsigned char cxd3778gf_master_gain_table[MASTER_GAIN_MAX + 1];
extern struct cxd3778gf_device_gain cxd3778gf_device_gain_table[INPUT_DEVICE_MAX + 1];
extern unsigned char cxd3778gf_tone_control_table[(TONE_CONTROL_TABLE_MAX + 1)][CODEC_RAM_SIZE];
extern unsigned int cxd3778gf_master_volume_dsd_table[MASTER_VOLUME_TABLE_MAX + 1][MASTER_VOLUME_MAX + 1];

int checksum(const unsigned char *buf, int size, unsigned int *sum, unsigned int *xr);

#define CHECKSUM_SIZE 8

class TableLike {
  public:
    virtual int ToFile(const std::string &path) = 0;
    virtual int Apply(const std::string &path) = 0;
    virtual void Reset() = 0;
};

class master_volume : public TableLike {
  public:
    cxd3778gf_master_volume v[2][MASTER_VOLUME_TABLE_MAX + 1][MASTER_VOLUME_MAX + 1]{};
    uint sum{};
    uint xr{};

    uint GetValue(int tableIndex, int volumeTable, int volume, MASTER_VOLUME_VALUE valueType);
    void SetValue(int tableIndex, int volumeTable, int volume, MASTER_VOLUME_VALUE valueType, uint value);
    int FromFile(const std::string &path);
    int ToFile(const std::string &path) override;
    int Apply(const std::string &path) override;
    void Reset() override;
};

class master_volume_dsd : public TableLike {
  public:
    unsigned int v[MASTER_VOLUME_TABLE_MAX + 1][MASTER_VOLUME_MAX + 1]{};
    uint sum{};
    uint xr{};

    int FromFile(const std::string &path);
    int ToFile(const std::string &path) override;
    int Apply(const std::string &path) override;
    void Reset() override;
};

class tone_control : public TableLike {
  public:
    unsigned char v[TONE_CONTROL_TABLE_MAX + 1][CODEC_RAM_SIZE]{};
    uint sum{};
    uint xr{};

    int FromFile(const std::string &path);
    int ToFile(const std::string &path) override;
    int Apply(const std::string &path) override;
    void Reset() override;
};

#endif
