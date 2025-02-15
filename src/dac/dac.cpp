#include "dac.h"
#include "cxd3778gf_table.h"
#include <fstream>

#define TABLE_SIZE_OUTPUT_VOLUME (sizeof(struct cxd3778gf_master_volume) * (MASTER_VOLUME_MAX + 1) * (MASTER_VOLUME_TABLE_MAX + 1) * 2)
#define TABLE_SIZE_OUTPUT_VOLUME_DSD (sizeof(unsigned int) * (MASTER_VOLUME_TABLE_MAX + 1) * (MASTER_VOLUME_MAX + 1))
#define TABLE_SIZE_DEVICE_GAIN (sizeof(struct cxd3778gf_device_gain) * (INPUT_DEVICE_MAX + 1))
#define TABLE_SIZE_TONE_CONTROL (sizeof(unsigned char) * (TONE_CONTROL_TABLE_MAX + 1) * CODEC_RAM_SIZE)

namespace Dac {

#ifdef DESKTOP
    std::string volumeTableOutPath = "/tmp/out_ovt";
    std::string volumeTableDSDOutPath = "/tmp/out_ovt_dsd";
    std::string toneControlOutPath = "/tmp/tct";
#else
    std::string volumeTableOutPath = "/proc/icx_audio_cxd3778gf_data/ovt";
    std::string volumeTableDSDOutPath = "/proc/icx_audio_cxd3778gf_data/ovt_dsd";
    std::string toneControlOutPath = "/proc/icx_audio_cxd3778gf_data/tct";
#endif

    std::map<int, std::string> TableTypeToString = {
        {TABLE_ID_MASTER_VOLUME, "Master volume"},
        //        {TABLE_ID_DEVICE_GAIN, "Device gain"}, // used for audio recording
        {TABLE_ID_TONE_CONTROL, "Tone control"},
        {TABLE_ID_MASTER_VOLUME_DSD, "Master volume (DSD)"},
    };

    // BTL here refers to the Balanced output on the players that have it, "Bridge Tied Load"
    std::map<int, std::string> MasterVolumeTableTypeToString = {
        {MASTER_VOLUME_TABLE_OFF, "No output device (OFF)"},
        {MASTER_VOLUME_TABLE_SMASTER_SE_LG, "Headphones, low gain (SMASTER_SE_LG)"},
        {MASTER_VOLUME_TABLE_SMASTER_SE_HG, "Headphones, high gain (SMASTER_SE_HG)"},
        {MASTER_VOLUME_TABLE_SMASTER_SE_NC, "Headphones, noise cancel ON (SMASTER_SE_NC)"},
        {MASTER_VOLUME_TABLE_SMASTER_SE_AM, "Headphones, ambient ON (SMASTER_SE_AM)"},
        {MASTER_VOLUME_TABLE_SMASTER_BTL_50, "Headphones, balanced output, low gain (SMASTER_BTL_50)"},
        {MASTER_VOLUME_TABLE_SMASTER_BTL_100, "Headphones, balanced output, high gain (SMASTER_BTL_100)"},
        {MASTER_VOLUME_TABLE_CLASSAB, "Headphones, ?, (CLASSAB)"},
        {MASTER_VOLUME_TABLE_CLASSAB_NC, "Headphones, ?, noise cancel ON (CLASSAB_NC)"},
        {MASTER_VOLUME_TABLE_CLASSAB_AM, "Headphones, ?, ambient ON (CLASSAB_AM)"},
        {MASTER_VOLUME_TABLE_LINE, "Line output (LINE)"},
        {MASTER_VOLUME_TABLE_FIXEDLINE, "Fixed line output (FIXEDLINE)"},
        {MASTER_VOLUME_TABLE_SMASTER_SE_DSD64_LG, "DSD64 low gain (SMASTER_SE_DSD64_LG)"},  // sample rate 88200
        {MASTER_VOLUME_TABLE_SMASTER_SE_DSD64_HG, "DSD64 high gain (SMASTER_SE_DSD64_HG)"}, // sample rate 176400
        {MASTER_VOLUME_TABLE_SMASTER_SE_DSD128_LG, "DSD128 low gain (SMASTER_SE_DSD128_LG)"},
        {MASTER_VOLUME_TABLE_SMASTER_SE_DSD128_HG, "DSD128 high gain (SMASTER_SE_DSD128_HG)"},
        {MASTER_VOLUME_TABLE_SMASTER_SE_DSD256_LG, "DSD256 low gain (SMASTER_SE_DSD256_LG)"},
        {MASTER_VOLUME_TABLE_SMASTER_SE_DSD256_HG, "DSD256 high gain (SMASTER_SE_DSD256_HG)"},
        {MASTER_VOLUME_TABLE_SMASTER_BTL_50_DSD64, "DSD64, balanced output, low gain (SMASTER_BTL_50_DSD64)"},
        {MASTER_VOLUME_TABLE_SMASTER_BTL_100_DSD64, "DSD64, balanced output, high gain (SMASTER_BTL_100_DSD64)"},
        {MASTER_VOLUME_TABLE_SMASTER_BTL_50_DSD128, "DSD128, balanced output, low gain (SMASTER_BTL_50_DSD128)"},
        {MASTER_VOLUME_TABLE_SMASTER_BTL_100_DSD128, "DSD128, balanced output, high gain (SMASTER_BTL_100_DSD128)"},
        {MASTER_VOLUME_TABLE_SMASTER_BTL_50_DSD256, "DSD256, balanced output, low gain (SMASTER_BTL_50_DSD256)"},
        {MASTER_VOLUME_TABLE_SMASTER_BTL_100_DSD256, "DSD256, balanced output, high gain (SMASTER_BTL_100_DSD256)"},
        {MASTER_VOLUME_TABLE_LINE_DSD64, "DSD64, line output (LINE_DSD64)"},
        {MASTER_VOLUME_TABLE_LINE_DSD128, "DSD128, line output (LINE_DSD128)"},
        {MASTER_VOLUME_TABLE_LINE_DSD256, "DSD256, line output (LINE_DSD256)"},
    };

    std::map<int, std::string> ToneControlTableTypeToString = {
        {TONE_CONTROL_TABLE_NO_HP, "NO_HP"},
        {TONE_CONTROL_TABLE_NAMP_GENERAL_HP, "NAMP_GENERAL_HP"},
        {TONE_CONTROL_TABLE_NAMP_NW500N_NCHP, "NAMP_NW500N_NCHP"},
        {TONE_CONTROL_TABLE_NAMP_NW750N_NCHP, "NAMP_NW750N_NCHP"},
        {TONE_CONTROL_TABLE_NAMP_NC31_NCHP, "NAMP_NC31_NCHP"},
        {TONE_CONTROL_TABLE_SAMP_GENERAL_HP, "SAMP_GENERAL_HP"},
        {TONE_CONTROL_TABLE_SAMP_NW500N_NCHP, "SAMP_NW500N_NCHP"},
        {TONE_CONTROL_TABLE_SAMP_NW750N_NCHP, "SAMP_NW750N_NCHP"},
        {TONE_CONTROL_TABLE_SAMP_NC31_NCHP, "SAMP_NC31_NCHP"},
    };

    std::map<int, std::string> MasterVolumeValueTypeToString = {
        {MASTER_VOLUME_VALUE_HPOUT, "Headphones (HpOut)"},
        {MASTER_VOLUME_VALUE_PLAY, "Play"},
        {MASTER_VOLUME_VALUE_LINEIN, "LineIn"},
        {MASTER_VOLUME_VALUE_SDIN1, "SdIn1"},
        {MASTER_VOLUME_VALUE_SDIN2, "SdIn2"},
        {MASTER_VOLUME_VALUE_LINEOUT, "LineOut"},
        {MASTER_VOLUME_VALUE_CMX1_500, "CMX1_500"},
        {MASTER_VOLUME_VALUE_CMX2_500, "CMX2_500"},
        {MASTER_VOLUME_VALUE_CMX1_750, "CMX1_750"},
        {MASTER_VOLUME_VALUE_CMX2_750, "CMX2_750"},
        {MASTER_VOLUME_VALUE_CMX1_31, "CMX1_31"},
        {MASTER_VOLUME_VALUE_CMX2_31, "CMX2_31"},
        {MASTER_VOLUME_VALUE_HPOUT2_CTRL3, "HpOut2_CTRL3"},
    };

    void printTableValue(master_volume *t, MASTER_VOLUME_VALUE valType, const std::string &outfile) {
        std::fstream f;
        if (!outfile.empty()) {
            f.open(outfile, std::ios::trunc | std::ios::out);
            f << "valueid " << MasterVolumeValueTypeToString.at(valType) << "\n";
        }

        int soundEffect = SOUND_EFFECT_ON;
        //    maxGain = 2;
        for (int tableID = 0; tableID <= MASTER_VOLUME_TABLE_MAX; tableID++) {
            int counter = 0;
            for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
                unsigned char val;

                switch (valType) {
                case MASTER_VOLUME_VALUE_LINEIN:
                    val = t->v[soundEffect][tableID][i].linein;
                    break;
                case MASTER_VOLUME_VALUE_SDIN1:
                    val = t->v[soundEffect][tableID][i].sdin1;
                    break;
                case MASTER_VOLUME_VALUE_SDIN2:
                    val = t->v[soundEffect][tableID][i].sdin2;
                    break;
                case MASTER_VOLUME_VALUE_PLAY:
                    val = t->v[soundEffect][tableID][i].play;
                    break;
                case MASTER_VOLUME_VALUE_LINEOUT:
                    val = t->v[soundEffect][tableID][i].lineout;
                    break;
                case MASTER_VOLUME_VALUE_HPOUT:
                    val = t->v[soundEffect][tableID][i].hpout;
                    break;
                case MASTER_VOLUME_VALUE_CMX1_500:
                    val = t->v[soundEffect][tableID][i].cmx1_500;
                    break;
                case MASTER_VOLUME_VALUE_CMX2_500:
                    val = t->v[soundEffect][tableID][i].cmx2_500;
                    break;
                case MASTER_VOLUME_VALUE_CMX1_750:
                    val = t->v[soundEffect][tableID][i].cmx1_750;
                    break;
                case MASTER_VOLUME_VALUE_CMX2_750:
                    val = t->v[soundEffect][tableID][i].cmx2_750;
                    break;
                case MASTER_VOLUME_VALUE_CMX1_31:
                    val = t->v[soundEffect][tableID][i].cmx1_31;
                    break;
                case MASTER_VOLUME_VALUE_CMX2_31:
                    val = t->v[soundEffect][tableID][i].cmx2_31;
                    break;
                case MASTER_VOLUME_VALUE_MAX:
                case MASTER_VOLUME_VALUE_HPOUT2_CTRL3:
                    val = t->v[soundEffect][tableID][i].hpout2_ctrl3;
                    break;
                }

                counter++;

                //                if (val != 0) {
                printf("vol: %03d | table: %02d | val: %02d\n", i, tableID, val);
                //                        printf("vol: %03d | gain: %02d | table: %02d | val: %02d\n", i, gainID, tableID, val);
                if (!outfile.empty()) {
                    f << tableID << " " << (int)val << "\n";
                    //                        f << "foo[" << counter << "]=ImVec2(" << counter << "," << (int)val << ");\n";
                }
                //                }
            }
        }

        if (!outfile.empty()) {
            f.close();
        }
    }

    std::string getStatus(std::vector<directoryEntry> *volumeTableFiles, const std::string &outPath) {
        std::fstream f(outPath, std::ios::in | std::ios::binary);
        std::string currentVT((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        f.close();

        auto vth = std::hash<std::string>{}(currentVT);
        std::string res = "?";
        DLOG("current is %d len, hash %d\n", currentVT.length(), vth);

        for (const auto &entry : *volumeTableFiles) {
            f.open(entry.fullPath, std::ios::in | std::ios::binary);
            std::string c((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            f.close();

#ifndef DESKTOP
            // device driver does not return checksum
            if (c.length() > 0) {
                c.erase(c.length() - CHECKSUM_SIZE, CHECKSUM_SIZE);
            }
#endif
            auto h = std::hash<std::string>{}(c);
            //            DLOG("checking %s, hash %d\n", entry.fullPath.c_str(), h);
            if (h == vth) {
                res = entry.name;
                break;
            }
        }

        return res;
    }
} // namespace Dac
