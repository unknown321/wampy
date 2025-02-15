#ifndef WAMPY_EDITOR_H
#define WAMPY_EDITOR_H

#include "../src/dac/cxd3778gf_common.h"
#include "../src/dac/cxd3778gf_table.h"
#include "implot/implot.h"

#define BUTTON_RED ImVec4(0.98f, 0.27f, 0.26f, 0.4f)
#define GOLD_DONATE ImVec4(0.91f, 0.72f, 0.25f, 1.0f) // #ebb943

enum ETableType {
    ETableType_UNKNOWN = 0,
    ETableType_VOLUME = 1,
    ETableType_DSD = 2,
    ETableType_TONE = 3,
};

extern const char *tableNames[4];

class editor {
  public:
    int curveElementCount = 1;
    int curveYLimit = 255;

    double valuesX[CODEC_RAM_SIZE]{0};
    master_volume masterVolume{};
    master_volume_dsd masterVolumeDsd{};
    tone_control toneControl{};
    int height = 800;
    int width = 600;
    ETableType tableType = ETableType_UNKNOWN;
    std::string name;
    std::string legendName;

    double valuesyCopy[CODEC_RAM_SIZE]{0};

    bool soundEffectOn = true;
    int MasterVolumeTableType = MASTER_VOLUME_TABLE_SMASTER_SE_HG;
    MASTER_VOLUME_VALUE MasterVolumeValueType = MASTER_VOLUME_VALUE_HPOUT;

    int toneControlTableType = TONE_CONTROL_TABLE_SAMP_GENERAL_HP;
    int MasterVolumeDSDTableType = MASTER_VOLUME_TABLE_SMASTER_SE_DSD64_HG;

    double masterVolumeDSDValues[MASTER_VOLUME_TABLE_MAX + 1][MASTER_VOLUME_MAX + 1]{0};
    double masterVolumeValues[2][MASTER_VOLUME_TABLE_MAX + 1][MASTER_VOLUME_VALUE_MAX][MASTER_VOLUME_MAX + 1 + 1]{0};
    double toneControlValues[TONE_CONTROL_TABLE_MAX + 1][CODEC_RAM_SIZE]{0};
    double noValues = 0;

    double masterVolumeValueBuffer[MASTER_VOLUME_MAX + 1 + 1]{0};
    double masterVolumeDSDValueBuffer[MASTER_VOLUME_MAX + 1]{0};
    double toneControlValueBuffer[CODEC_RAM_SIZE]{0};

    double *target = &noValues;

    ImPlotCond cond = ImPlotCond_Once;

    void CurveEditor();

    void MasterVolOpts();

    void DSDOpts();

    void ToneOpts();

    void MasterToDouble();

    void MasterFromDouble();

    void DSDToDouble();

    void DSDFromDouble();

    void ToneToDouble();

    void ToneFromDouble();
};

#endif // WAMPY_EDITOR_H
