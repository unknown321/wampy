/*
 * Copyright 2013,2014,2015,2016,2017 Sony Corporation
 */
/*
 * cxd3778gf_table.c
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

#include "cxd3778gf_table.h"
#include "cxd3778gf_common.h"
#include "fstream"
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <libgen.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#define TABLE_SIZE_OUTPUT_VOLUME (sizeof(struct cxd3778gf_master_volume) * (MASTER_VOLUME_MAX + 1) * (MASTER_VOLUME_TABLE_MAX + 1) * 2)
#define TABLE_SIZE_OUTPUT_VOLUME_DSD (sizeof(unsigned int) * (MASTER_VOLUME_TABLE_MAX + 1) * (MASTER_VOLUME_MAX + 1))
#define TABLE_SIZE_DEVICE_GAIN (sizeof(struct cxd3778gf_device_gain) * (INPUT_DEVICE_MAX + 1))
#define TABLE_SIZE_TONE_CONTROL (sizeof(unsigned char) * (TONE_CONTROL_TABLE_MAX + 1) * CODEC_RAM_SIZE)

struct port_info {
    const char *name;
    void *entry;
    int node;
    int size;
    unsigned char *table;
    int columns; /* for debug */
    int rows;    /* for debug */
    int width;   /* for debug */
};

int do_mkdir1(const char *path, mode_t mode) {
    struct stat st {};
    int status = 0;

    if (stat(path, &st) != 0) {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            status = -1;
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        status = -1;
    }

    return (status);
}

int mkpath1(const char *path, mode_t mode) {
    char *pp;
    char *sp;
    int status;
    char *copypath = strdup(path);

    status = 0;
    pp = copypath;
    while (status == 0 && (sp = strchr(pp, '/')) != nullptr) {
        if (sp != pp) {
            /* Neither root nor double slash in path */
            *sp = '\0';
            status = do_mkdir1(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = do_mkdir1(path, mode);
    free(copypath);
    return (status);
}

static int check_data(const unsigned char *buf, int size, unsigned int *sum, unsigned int *xr);
static void dump_data(unsigned char *buf, int size, int columns, int rows, int width);

#if 0
/* muting all */
struct cxd3778gf_master_volume cxd3778gf_master_volume_table
								[2]
								[MASTER_VOLUME_TABLE_MAX+1]
								[MASTER_VOLUME_MAX+1]
									= {{{{0x33,0x33,0x33,0x33,0xC0,0xC0,0x00,0x3F,0x00,0x00}}}};
#endif

struct cxd3778gf_master_volume cxd3778gf_master_volume_table[2][MASTER_VOLUME_TABLE_MAX + 1][MASTER_VOLUME_MAX + 1] = {
    {{{0x33, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}}};

unsigned int cxd3778gf_master_volume_dsd_table[MASTER_VOLUME_TABLE_MAX + 1][MASTER_VOLUME_MAX + 1] = {{0x00}};

unsigned char cxd3778gf_master_gain_table[MASTER_GAIN_MAX + 1] = {
    /* 00 */ 0xC0,
    /* 01 */ 0xC5,
    /* 02 */ 0xC8,
    /* 03 */ 0xCC,
    /* 04 */ 0xD0,
    /* 05 */ 0xD3,
    /* 06 */ 0xD6,
    /* 07 */ 0xD9,
    /* 08 */ 0xDC,
    /* 09 */ 0xDF,
    /* 10 */ 0xE2,
    /* 11 */ 0xE4,
    /* 12 */ 0xE6,
    /* 13 */ 0xE8,
    /* 14 */ 0xEA,
    /* 15 */ 0xEC,
    /* 16 */ 0xEE,
    /* 17 */ 0xF0,
    /* 18 */ 0xF2,
    /* 19 */ 0xF3,
    /* 20 */ 0xF4,
    /* 21 */ 0xF5,
    /* 22 */ 0xF6,
    /* 23 */ 0xF7,
    /* 24 */ 0xF8,
    /* 25 */ 0xF9,
    /* 26 */ 0xFA,
    /* 27 */ 0xFB,
    /* 28 */ 0xFC,
    /* 29 */ 0xFE,
    /* 30 */ 0x00,
};

struct cxd3778gf_device_gain cxd3778gf_device_gain_table[INPUT_DEVICE_MAX + 1] = {
    /*    PGA1  ADC1                 */
    {0x00, 0x00}, /* OFF       */
    {0x06, 0x00}, /* TUNER     */
    {0x00, 0x00}, /* MIC       */
    {0xF8, 0x00}, /* LINE      */
    {0x00, 0x00}, /* DIRECTMIC */
};

unsigned char cxd3778gf_tone_control_table[TONE_CONTROL_TABLE_MAX + 1][CODEC_RAM_SIZE] = {{0x00}};

static struct port_info port_info_table[] = {
    {"ovt", nullptr, -1, TABLE_SIZE_OUTPUT_VOLUME, (unsigned char *)cxd3778gf_master_volume_table, 13, 121, 1},
    {"dgt", nullptr, -1, TABLE_SIZE_DEVICE_GAIN, (unsigned char *)cxd3778gf_device_gain_table, 2, 5, 1},
    {"tct", nullptr, -1, TABLE_SIZE_TONE_CONTROL, (unsigned char *)cxd3778gf_tone_control_table, 20, 8, 1},
    {"ovt_dsd", nullptr, -1, TABLE_SIZE_OUTPUT_VOLUME_DSD, (unsigned char *)cxd3778gf_master_volume_dsd_table, 1, 121, 4},
    {"tct_nh", nullptr, -1, CODEC_RAM_SIZE, (unsigned char *)cxd3778gf_tone_control_table[TONE_CONTROL_TABLE_NO_HP], 20, 6, 1},
    {"tct_ng", nullptr, -1, CODEC_RAM_SIZE, (unsigned char *)cxd3778gf_tone_control_table[TONE_CONTROL_TABLE_NAMP_GENERAL_HP], 20, 6, 1},
    {"tct_nnw500",
     nullptr,
     -1,
     CODEC_RAM_SIZE,
     (unsigned char *)cxd3778gf_tone_control_table[TONE_CONTROL_TABLE_NAMP_NW500N_NCHP],
     20,
     6,
     1},
    {"tct_nnw750",
     nullptr,
     -1,
     CODEC_RAM_SIZE,
     (unsigned char *)cxd3778gf_tone_control_table[TONE_CONTROL_TABLE_NAMP_NW750N_NCHP],
     20,
     6,
     1},
    {"tct_nnc31", nullptr, -1, CODEC_RAM_SIZE, (unsigned char *)cxd3778gf_tone_control_table[TONE_CONTROL_TABLE_NAMP_NC31_NCHP], 20, 6, 1},
    {"tct_sg", nullptr, -1, CODEC_RAM_SIZE, (unsigned char *)cxd3778gf_tone_control_table[TONE_CONTROL_TABLE_SAMP_GENERAL_HP], 20, 6, 1},
    {"tct_snw500",
     nullptr,
     -1,
     CODEC_RAM_SIZE,
     (unsigned char *)cxd3778gf_tone_control_table[TONE_CONTROL_TABLE_SAMP_NW500N_NCHP],
     20,
     6,
     1},
    {"tct_snw750",
     nullptr,
     -1,
     CODEC_RAM_SIZE,
     (unsigned char *)cxd3778gf_tone_control_table[TONE_CONTROL_TABLE_SAMP_NW750N_NCHP],
     20,
     6,
     1},
    {"tct_snc31", nullptr, -1, CODEC_RAM_SIZE, (unsigned char *)cxd3778gf_tone_control_table[TONE_CONTROL_TABLE_SAMP_NC31_NCHP], 20, 6, 1},
    {nullptr, nullptr, -1, 0, nullptr, 0, 0, 0}};

static int initialized = FALSE;
static struct mutex *global_mutex = nullptr;
static struct proc_dir_entry *port_directory = nullptr;
static unsigned char *work_buf = nullptr;

static ssize_t write_table(const std::string &data, const char *buf, size_t size, loff_t *pos) {
    int index;
    int count = 0;
    int rv;

    index = 0;

    *pos = *pos + count;

    if (*pos >= port_info_table[index].size + 8) {

        unsigned int sum;
        unsigned int xr;
        rv = check_data(work_buf, port_info_table[index].size, &sum, &xr);
        if (rv < 0) {
            return (-1);
        }

        dump_data(
            (unsigned char *)port_info_table[index].table,
            port_info_table[index].size,
            port_info_table[index].columns,
            port_info_table[index].rows,
            port_info_table[index].width
        );
        //        cxd3778gf_apply_table_change(index);

        printf("done\n");
    }

    return (count);
}

// void read_table(const std::string &data, bool dump, master_volume* m) {
//     int index;
//
//     index = 0;
//
//     if (dump) {
//         printf("target table = %s\n", port_info_table[index].name);
//         dump_data(
//             (unsigned char *)data.c_str(),
//             port_info_table[index].size,
//             port_info_table[index].columns,
//             port_info_table[index].rows,
//             port_info_table[index].width
//         );
//     }
//
//     auto m = master_volume{};
//
//     memcpy(&m.v, data.c_str(), sizeof(m.v));
//     //    memcpy(&m.v, cxd3778gf_master_volume_table, sizeof(cxd3778gf_master_volume_table));
//
//     return m;
// }

int checksum(const unsigned char *buf, int size, unsigned int *sum, unsigned int *xr) {
    *sum = 0;
    *xr = 0;
    int n;

    for (n = 0; n < size; n++) {
        *sum = *sum + buf[n];
        *xr = *xr ^ (buf[n] << (n % 4) * 8);
    }

    //    printf("sum = 0x%08X, xor = 0x%08X\n", sum, xr);

    if (*sum == 0) {
        printf("all zero\n");
        return (-1);
    }

    return 0;
}

static int check_data(const unsigned char *buf, int size, unsigned int *sum, unsigned int *xr) {
    if (checksum(buf, size, sum, xr) != 0) {
        printf("failed to calc checksum\n");
        return -1;
    }

    if (*sum != *(unsigned int *)(buf + size)) {
        printf("sum error %08X,%08X\n", *sum, *(unsigned int *)(buf + size));
        return (-1);
    }

    if (*xr != *(unsigned int *)(buf + size + 4)) {
        printf("xor error %08X,%08X\n", xr, *(unsigned int *)(buf + size + 4));
        return (-1);
    }

    return 0;
}

static void dump_data(unsigned char *buf, int size, int columns, int rows, int width) {
    unsigned char *uc;
    unsigned int *ui;
    int n;
    int m;

    printf("target = %08X, size = %d\n", buf, size);

    if (width == 1) {

        uc = buf;

        for (n = 0; n < size; n = n + columns) {

            printf("%02d:", (n / columns) % rows);

            for (m = 0; m < columns; m++) {

                if (n + m >= size)
                    break;

                printf(" %02X", uc[n + m]);
            }

            printf("\n");
        }
    }

    if (width == 4) {

        ui = (unsigned int *)buf;
        size = size / width;

        for (n = 0; n < size; n = n + columns) {

            printf("%02d:", (n / columns) % rows);

            for (m = 0; m < columns; m++) {

                if (n + m >= size)
                    break;

                printf(" %08X", ui[n + m]);
            }

            printf("\n");
        }
    }
}

unsigned int master_volume::GetValue(int tableIndex, int volumeTable, int volume, MASTER_VOLUME_VALUE valueType) {
    unsigned char val;

    switch (valueType) {
    case MASTER_VOLUME_VALUE_LINEIN:
        val = v[tableIndex][volumeTable][volume].linein;
        break;
    case MASTER_VOLUME_VALUE_SDIN1:
        val = v[tableIndex][volumeTable][volume].sdin1;
        break;
    case MASTER_VOLUME_VALUE_SDIN2:
        val = v[tableIndex][volumeTable][volume].sdin2;
        break;
    case MASTER_VOLUME_VALUE_PLAY:
        val = v[tableIndex][volumeTable][volume].play;
        break;
    case MASTER_VOLUME_VALUE_LINEOUT:
        val = v[tableIndex][volumeTable][volume].lineout;
        break;
    case MASTER_VOLUME_VALUE_HPOUT:
        val = v[tableIndex][volumeTable][volume].hpout;
        break;
    case MASTER_VOLUME_VALUE_CMX1_500:
        val = v[tableIndex][volumeTable][volume].cmx1_500;
        break;
    case MASTER_VOLUME_VALUE_CMX2_500:
        val = v[tableIndex][volumeTable][volume].cmx2_500;
        break;
    case MASTER_VOLUME_VALUE_CMX1_750:
        val = v[tableIndex][volumeTable][volume].cmx1_750;
        break;
    case MASTER_VOLUME_VALUE_CMX2_750:
        val = v[tableIndex][volumeTable][volume].cmx2_750;
        break;
    case MASTER_VOLUME_VALUE_CMX1_31:
        val = v[tableIndex][volumeTable][volume].cmx1_31;
        break;
    case MASTER_VOLUME_VALUE_CMX2_31:
        val = v[tableIndex][volumeTable][volume].cmx2_31;
        break;
    case MASTER_VOLUME_VALUE_HPOUT2_CTRL3:
    case MASTER_VOLUME_VALUE_MAX:
        val = v[tableIndex][volumeTable][volume].hpout2_ctrl3;
        break;
    }

    //    if (val > 0) {
    //        printf("%d %d %d: %d\n", tableIndex, volumeTable, volume, val);
    //    }
    return (unsigned int)val;
}

void master_volume::SetValue(int tableIndex, int volumeTable, int volume, MASTER_VOLUME_VALUE valueType, unsigned int value) {
    if (value > 255) {
        value = 0;
    }

    switch (valueType) {
    case MASTER_VOLUME_VALUE_LINEIN:
        v[tableIndex][volumeTable][volume].linein = (unsigned char)value;
        break;
    case MASTER_VOLUME_VALUE_SDIN1:
        v[tableIndex][volumeTable][volume].sdin1 = (unsigned char)value;
        break;
    case MASTER_VOLUME_VALUE_SDIN2:
        v[tableIndex][volumeTable][volume].sdin2 = (unsigned char)value;
        break;
    case MASTER_VOLUME_VALUE_PLAY:
        v[tableIndex][volumeTable][volume].play = (unsigned char)value;
        break;
    case MASTER_VOLUME_VALUE_LINEOUT:
        v[tableIndex][volumeTable][volume].lineout = (unsigned char)value;
        break;
    case MASTER_VOLUME_VALUE_HPOUT:
        v[tableIndex][volumeTable][volume].hpout = (unsigned char)value;
        break;
    case MASTER_VOLUME_VALUE_CMX1_500:
        v[tableIndex][volumeTable][volume].cmx1_500 = (unsigned char)value;
        break;
    case MASTER_VOLUME_VALUE_CMX2_500:
        v[tableIndex][volumeTable][volume].cmx2_500 = (unsigned char)value;
        break;
    case MASTER_VOLUME_VALUE_CMX1_750:
        v[tableIndex][volumeTable][volume].cmx1_750 = (unsigned char)value;
        break;
    case MASTER_VOLUME_VALUE_CMX2_750:
        v[tableIndex][volumeTable][volume].cmx2_750 = (unsigned char)value;
        break;
    case MASTER_VOLUME_VALUE_CMX1_31:
        v[tableIndex][volumeTable][volume].cmx1_31 = (unsigned char)value;
        break;
    case MASTER_VOLUME_VALUE_CMX2_31:
        v[tableIndex][volumeTable][volume].cmx2_31 = (unsigned char)value;
        break;
    case MASTER_VOLUME_VALUE_HPOUT2_CTRL3:
    case MASTER_VOLUME_VALUE_MAX:
        v[tableIndex][volumeTable][volume].hpout2_ctrl3 = (unsigned char)value;
        break;
    }

    //        printf("set %d for %d %d %d: %d\n", tableIndex, volumeTable, volume, value);
}

int master_volume::ToFile(const std::string &path) {
    char *c = (char *)malloc(path.length() + 1);
    strcpy(c, path.c_str());
    auto d = dirname(c);
    mkpath1(d, 0755);

    std::fstream f;
    f.open(path, std::ios::binary | std::ios::out);

    if (!f.good()) {
        printf("bad file\n");
        return -1;
    }

    f.write((const char *)v, sizeof(v));

    checksum((const unsigned char *)v, sizeof(v), &sum, &xr);

    f.write((const char *)&sum, sizeof(sum));
    f.write((const char *)&xr, sizeof(xr));
    f.close();

    return 0;
}

int master_volume::Apply(const std::string &path) {
    auto f = open(path.c_str(), O_RDWR);
    if (f < 0) {
        printf("cannot open %s: %d\n", path.c_str(), f);
        return f;
    }

    if (write(f, (const char *)v, sizeof(v)) < 0) {
        printf("cannot write to %s\n", path.c_str());
        return -1;
    }

    checksum((const unsigned char *)v, sizeof(v), &sum, &xr);

    if (write(f, (const char *)&sum, sizeof(sum)) < 0) {
        printf("cannot write to %s\n", path.c_str());
        return -1;
    }

    if (write(f, (const char *)&xr, sizeof(xr)) < 0) {
        printf("cannot write to %s\n", path.c_str());
        return -1;
    }

    close(f);

    return 0;
}

void master_volume::Reset() {
    memset(&v, 0, sizeof(v));
    sum = 0;
    xr = 0;
}

int master_volume::FromFile(const std::string &path) {
    std::ifstream f;

    f.open(path, std::ios::binary);

    if (!f.good()) {
        printf("bad file\n");
        return -1;
    }

    std::string buf((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();

    auto length = buf.length();
    if (length != (TABLE_SIZE_OUTPUT_VOLUME + CHECKSUM_SIZE)) {
        printf("invalid volume table size\n");
        return -1;
    }

    memcpy(v, buf.c_str(), sizeof(v));

    if (check_data((const unsigned char *)buf.c_str(), (int)buf.length() - CHECKSUM_SIZE, &sum, &xr) != 0) {
        printf("bad checksum\n");
        return -1;
    }

    return 0;
}

int master_volume::FromBytes(const char *buf, size_t len) {
    if (len != (TABLE_SIZE_OUTPUT_VOLUME + CHECKSUM_SIZE)) {
        printf("invalid table size\n");
        return -1;
    }

    memcpy(v, buf, sizeof(v));

    if (check_data((const unsigned char *)buf, (int)len - CHECKSUM_SIZE, &sum, &xr) != 0) {
        printf("bad checksum\n");
        return -1;
    }
    return 0;
}

void master_volume::ToBytes(void *buf, size_t *len) {
    auto pos = (char *)buf;
    memcpy(pos, v, sizeof(v));
    pos = pos + sizeof(v);

    checksum((const unsigned char *)v, sizeof(v), &sum, &xr);

    memcpy(pos, (const char *)&sum, sizeof(sum));
    pos += sizeof(sum);
    memcpy(pos, (const char *)&xr, sizeof(xr));
    //    pos += sizeof(xr);

    *len = sizeof v + sizeof sum + sizeof xr;
}

int master_volume_dsd::FromFile(const std::string &path) {
    std::ifstream f;

    f.open(path, std::ios::binary);

    if (!f.good()) {
        printf("bad file\n");
        return -1;
    }

    std::string buf((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();

    auto length = buf.length();
    if (length != (TABLE_SIZE_OUTPUT_VOLUME_DSD + CHECKSUM_SIZE)) {
        printf("invalid dsd table size\n");
        return -1;
    }

    memcpy(v, buf.c_str(), sizeof(v));

    if (check_data((const unsigned char *)buf.c_str(), (int)buf.length() - CHECKSUM_SIZE, &sum, &xr) != 0) {
        printf("bad checksum\n");
        return -1;
    }

    return 0;
}

int master_volume_dsd::ToFile(const std::string &path) {
    char *c = (char *)malloc(path.length() + 1);
    strcpy(c, path.c_str());
    auto d = dirname(c);
    mkpath1(d, 0755);

    std::fstream f;

    f.open(path, std::ios::binary | std::ios::out);

    if (!f.good()) {
        printf("bad file\n");
        return -1;
    }

    f.write((const char *)v, sizeof(v));

    checksum((const unsigned char *)v, sizeof(v), &sum, &xr);

    f.write((const char *)&sum, sizeof(sum));
    f.write((const char *)&xr, sizeof(xr));
    f.close();

    return 0;
}

void master_volume_dsd::ToBytes(void *buf, size_t *len) {
    auto pos = (char *)buf;
    memcpy(pos, v, sizeof(v));
    pos = pos + sizeof(v);

    checksum((const unsigned char *)v, sizeof(v), &sum, &xr);

    memcpy(pos, (const char *)&sum, sizeof(sum));
    pos += sizeof(sum);
    memcpy(pos, (const char *)&xr, sizeof(xr));
    pos += sizeof(xr);

    *len = sizeof v + sizeof sum + sizeof xr;
}

int master_volume_dsd::Apply(const std::string &path) {
    auto f = open(path.c_str(), O_RDWR);
    if (f < 0) {
        printf("cannot open %s: %d\n", path.c_str(), f);
        return f;
    }

    if (write(f, (const char *)v, sizeof(v)) < 0) {
        printf("cannot write to %s\n", path.c_str());
        return -1;
    }

    checksum((const unsigned char *)v, sizeof(v), &sum, &xr);

    if (write(f, (const char *)&sum, sizeof(sum)) < 0) {
        printf("cannot write to %s\n", path.c_str());
        return -1;
    }

    if (write(f, (const char *)&xr, sizeof(xr)) < 0) {
        printf("cannot write to %s\n", path.c_str());
        return -1;
    }

    close(f);

    return 0;
}

void master_volume_dsd::Reset() {
    memset(&v, 0, sizeof(v));
    sum = 0;
    xr = 0;
}

int master_volume_dsd::FromBytes(const char *buf, size_t len) {
    if (len != (TABLE_SIZE_OUTPUT_VOLUME_DSD + CHECKSUM_SIZE)) {
        printf("invalid dsd table size\n");
        return -1;
    }

    memcpy(v, buf, sizeof(v));

    if (check_data((const unsigned char *)buf, (int)len - CHECKSUM_SIZE, &sum, &xr) != 0) {
        printf("bad checksum\n");
        return -1;
    }
    return 0;
}

int tone_control::FromFile(const std::string &path) {
    std::ifstream f;

    f.open(path, std::ios::binary);

    if (!f.good()) {
        printf("bad file\n");
        return -1;
    }

    std::string buf((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();

    auto length = buf.length();
    if (length != (TABLE_SIZE_TONE_CONTROL + CHECKSUM_SIZE)) {
        printf("invalid tone control table size\n");
        return -1;
    }

    memcpy(v, buf.c_str(), sizeof(v));

    if (check_data((const unsigned char *)buf.c_str(), (int)buf.length() - CHECKSUM_SIZE, &sum, &xr) != 0) {
        printf("bad checksum\n");
        return -1;
    }

    return 0;
}

int tone_control::ToFile(const std::string &path) {
    char *c = (char *)malloc(path.length() + 1);
    strcpy(c, path.c_str());
    auto d = dirname(c);
    mkpath1(d, 0755);

    std::fstream f;

    f.open(path, std::ios::binary | std::ios::out);

    if (!f.good()) {
        printf("bad file\n");
        return -1;
    }

    f.write((const char *)v, sizeof(v));

    checksum((const unsigned char *)v, sizeof(v), &sum, &xr);

    f.write((const char *)&sum, sizeof(sum));
    f.write((const char *)&xr, sizeof(xr));
    f.close();

    return 0;
}

int tone_control::Apply(const std::string &path) {
    auto f = open(path.c_str(), O_RDWR);
    if (f < 0) {
        printf("cannot open %s: %d\n", path.c_str(), f);
        return f;
    }

    if (write(f, (const char *)v, sizeof(v)) < 0) {
        printf("cannot write to %s\n", path.c_str());
        return -1;
    }

    checksum((const unsigned char *)v, sizeof(v), &sum, &xr);

    if (write(f, (const char *)&sum, sizeof(sum)) < 0) {
        printf("cannot write to %s\n", path.c_str());
        return -1;
    }

    if (write(f, (const char *)&xr, sizeof(xr)) < 0) {
        printf("cannot write to %s\n", path.c_str());
        return -1;
    }

    close(f);

    return 0;
}
void tone_control::Reset() {
    memset(&v, 0, sizeof(v));
    sum = 0;
    xr = 0;
}
int tone_control::FromBytes(const char *buf, size_t len) {
    if (len != (TABLE_SIZE_TONE_CONTROL + CHECKSUM_SIZE)) {
        printf("invalid tone control table size\n");
        return -1;
    }

    memcpy(v, buf, sizeof(v));

    if (check_data((const unsigned char *)buf, (int)len - CHECKSUM_SIZE, &sum, &xr) != 0) {
        printf("bad checksum\n");
        return -1;
    }

    return 0;
}

void tone_control::ToBytes(void *buf, size_t *len) {
    auto pos = (char *)buf;
    memcpy(pos, v, sizeof(v));
    pos = pos + sizeof(v);

    checksum((const unsigned char *)v, sizeof(v), &sum, &xr);

    memcpy(pos, (const char *)&sum, sizeof(sum));
    pos += sizeof(sum);
    memcpy(pos, (const char *)&xr, sizeof(xr));
    pos += sizeof(xr);

    *len = sizeof v + sizeof sum + sizeof xr;
}