#ifndef WAMPY_SKIN_H
#define WAMPY_SKIN_H

#include "Version.h"
#include "cassette/cassette.h"
#include "config.h"
#include "connector/hagoromoToString.h"
#include "dac/dac.h"
#include "digital_clock/digital_clock.h"
#include "implot.h"
#include "skinVariant.h"
#include "sound_settings/sound_settings.h"
#include "w1/w1.h"
#include "winamp/winamp.h"
#include <thread>

#define SETTINGS_FONT_SIZE 25

enum SettingsTab {
    SkinOpts = 0,
    Misc = 1,
    TabLicense3rd = 3,
    TabLicense = 4,
    TabWebsite = 5,
    TabWalkmanOne = 6,
    TabSoundSettings = 7,
    TabCurveEditor = 8,
    TabEQ = 9,
    TabEQOld = 10,
    TabDAC = 11,
    TabFM = 12,
};

enum EActiveFilterTab {
    ActiveFilterTab_Invalid = 0,
    ActiveFilterTab_DynamicNormalizer = 1,
    ActiveFilterTab_Eq6Band = 2,
    ActiveFilterTab_Vpt = 3,
    ActiveFilterTab_DCPhaseLinearizer = 5,
    ActiveFilterTab_Vinylizer = 6,
    ActiveFilterTab_Eq10Band = 7,
    ActiveFilterTab_EqTone = 8,
    ActiveFilterTab_Donate = 9,
    ActiveFilterTab_Misc = 10,
};

#define PLEASANT_GREEN ImVec4(0.26f, 0.98f, 0.37f, 0.4f)     // #42fa5f
#define GOLD_HIRES_WALKMAN ImVec4(0.75f, 0.64f, 0.39f, 1.0f) // #c0a565
#define BUTTON_RED ImVec4(0.98f, 0.27f, 0.26f, 0.4f)
#define GOLD_DONATE ImVec4(0.91f, 0.72f, 0.25f, 1.0f) // #ebb943

#define EFFECT_CHANGE_QUEUE_TEXT "Change enqueued"

struct Skin {
    bool *render{};
    ImFont *FontRegular{};
    SkinList skinListWinamp{};
    SkinList reelListCassette{};
    SkinList tapeListCassette{};
    std::vector<std::string> winampSkinDirectories{};
    std::vector<std::string> cassetteTapeDirectories{};
    std::vector<std::string> cassetteReelDirectories{};

    std::vector<std::string> masterVolumeTableDirectories{};
    std::vector<std::string> masterVolumeTableDSDDirectories{};
    std::vector<std::string> toneControlTableDirectories{};

    W1::W1Options w1Options{};
    W1::WalkmanOneOptions walkmanOneOptions{};
    int selectedSkinIdx{};       // winamp skin idx
    std::string loadStatusStr{}; // skin loading status in settings
    Connector *connector{};
    bool needLoad{};
    bool *hold_toggled = nullptr;
    int *hold_value = nullptr;
    bool *power_pressed = nullptr;
    int displaySettings = 0;
    SettingsTab displayTab = SettingsTab::SkinOpts;

    ESkinVariant activeSkinVariant = EMPTY;
    int activeSettingsTab{};
    Winamp::Winamp winamp{};
    Cassette::Cassette cassette{};
    DigitalClock::DigitalClock digitalClock{};

    AppConfig::AppConfig *config{};
    bool onlyFont{};

    bool needRestart{};

    std::string license{};
    std::string license3rd{};

#ifdef DESKTOP
    std::string licensePath = "../LICENSE";
    std::string license3rdPath = "../LICENSE_3rdparty";
    std::string qrPath = "../qr.bmp";
    std::string qrDonatePath = "../qrDonate.bmp";
    std::string soundSettingsPathSystemWampy = "../ss/system/";
    std::string soundSettingsPathSystemHagoromo = "../ss/system/";
    std::string soundSettingsPathUser = "../ss/user/";
#else
    std::string licensePath = "/system/vendor/unknown321/usr/share/wampy/doc/LICENSE";
    std::string license3rdPath = "/system/vendor/unknown321/usr/share/wampy/doc/LICENSE_3rdparty";
    std::string qrPath = "/system/vendor/unknown321/usr/share/wampy/qr.bmp";
    std::string qrDonatePath = "/system/vendor/unknown321/usr/share/wampy/qrDonate.bmp";
    std::string soundSettingsPathUser = "/contents/wampy/sound_settings/";
    std::string soundSettingsPathSystemWampy = "/system/vendor/unknown321/usr/share/wampy/sound_settings/";
    std::string soundSettingsPathSystemHagoromo = "/system/usr/share/audio_dac/";
#endif
    GLuint qrTexture{};
    GLuint qrDonateTexture{};
    float qrSide{};
    bool isWalkmanOne{};

    int zoomZero = 0;

    std::string systemMark = "Ⓢ ";
    std::string systemMarkHagoromo = "Ⓗ ";

    std::vector<directoryEntry> masterVolumeFiles;
    int masterVolumeFileSelected = 0;
    bool soundEffectOn = true;
    int MasterVolumeTableType = MASTER_VOLUME_TABLE_SMASTER_SE_HG;
    int MasterVolumeValueType = MASTER_VOLUME_VALUE_HPOUT;
    master_volume masterVolume{};
    ImVec2 masterVolumeValues[2][MASTER_VOLUME_TABLE_MAX + 1][MASTER_VOLUME_VALUE_MAX][MASTER_VOLUME_MAX + 1 + 1] = {
        {{{{-FLT_MIN, -FLT_MIN}}}}};
    ImVec2 masterVolumeValueBuffer[MASTER_VOLUME_MAX + 1 + 1] = {{0, 0}};
    std::string statusStringMasterVolume;

    std::vector<directoryEntry> masterVolumeDSDFiles;
    int masterVolumeDSDFileSelected;
    master_volume_dsd masterVolumeDSD{};
    ImVec2 masterVolumeDSDValues[MASTER_VOLUME_TABLE_MAX + 1][MASTER_VOLUME_MAX + 1] = {{{-FLT_MIN, -FLT_MIN}}};
    ImVec2 masterVolumeDSDValueBuffer[MASTER_VOLUME_MAX + 1] = {{0, 0}};
    int MasterVolumeDSDTableType = MASTER_VOLUME_TABLE_SMASTER_SE_DSD64_HG;
    std::string statusStringMasterVolumeDSD;

    std::vector<directoryEntry> toneControlFiles;
    int toneControlFileSelected;
    tone_control toneControl{};
    ImVec2 toneControlValues[TONE_CONTROL_TABLE_MAX + 1][CODEC_RAM_SIZE] = {{{-FLT_MIN, -FLT_MIN}}};
    ImVec2 toneControlValueBuffer[CODEC_RAM_SIZE] = {{0, 0}};
    int toneControlTableType = TONE_CONTROL_TABLE_SAMP_GENERAL_HP;
    std::string statusStringToneControl;

    std::string statusMasterVTFile;
    std::string statusMasterVTDSDFile;
    std::string statusToneControlFile;
    std::string deviceProduct;
    std::string deviceModelID;
    std::string deviceRegionID;
    std::string deviceRegionStr;
    std::pair<std::string, std::string> deviceAudioInUse;
    std::string deviceAudioSampleRate;
    std::string minCpuFreq;
    std::string curCpuFreq;
    std::string freqStr;

    float *curveEditorTarget = (float *)masterVolumeValues[int(soundEffectOn)][MasterVolumeTableType][MasterVolumeValueType];
    float curveYLimit;
    int curveElementCount = 121;
    double valuesx[CODEC_RAM_SIZE]{0};
    double valuesy[CODEC_RAM_SIZE]{0};
    double zoomLimitXL = 0;
    double zoomLimitXR = 0;
    double zoomLimitYT = 0;
    double zoomLimitYB = 0;

    bool llusbdacLoaded;
    std::string llusbdacStatus;

    std::string bookmarkExportStatus;
    std::string logCleanupStatus;
    std::string logCleanupButtonLabel;
    std::string refreshStatus;
    std::string charactersInDB;

    std::string prevSong{};
    PlayStateE prevPlayState = PlayStateE::PAUSED;
    std::string eqStatus{};
    bool eqSongExists{};
    bool eqSongDirExists{};

    bool alwaysFalse{};

    std::string activeFilter;
    EActiveFilterTab eActiveFilterTab = ActiveFilterTab_Invalid;
    std::string effectQueueStatus;

    std::map<std::string, EActiveFilterTab> filterToTab = {
        {"dynamicnormalizer", ActiveFilterTab_DynamicNormalizer},
        {"eq6band", ActiveFilterTab_Eq6Band},
        {"vpt", ActiveFilterTab_Vpt},
        {"dcphaselinear", ActiveFilterTab_DCPhaseLinearizer},
        {"vinylizer", ActiveFilterTab_Vinylizer},
        {"eq10band", ActiveFilterTab_Eq10Band},
        {"eqtone", ActiveFilterTab_EqTone},
        {"misc", ActiveFilterTab_Misc},
        {"donate", ActiveFilterTab_Donate},
    };

    ImVec2 *windowOffset;
    ImVec2 screenMode;
    ImVec2 windowSize;

    bool radioAvailable;
    int fmFreq;
    char fmFreqFormat[11];

    struct timespec ssfwUpdateDelay = {0, 500000000};

    void WithWinampSkinDir(const std::string &d);

    void RefreshWinampSkinList();

    void WithCassetteTapeDir(const std::string &d);

    void WithCassetteReelDir(const std::string &d);

    void RefreshCassetteTapeReelLists();

    void WithMasterVolumeTableDirs(const std::string &d);
    void RefreshMasterVolumeTableFiles();

    void WithMasterVolumeTableDSDDirs(const std::string &d);
    void RefreshMasterVolumeTableDSDFiles();

    void WithToneControlTableDirs(const std::string &d);
    void RefreshToneControlFiles();

    void ReloadFont();

    void ReadQR();

    void Load();

    void LoadUpdatedSkin();

    void MasterVolumeTableToImVec2();

    void MasterVolumeDSDTableToImVec2();

    void ToneControlToImVec2();

    void MasterVolumeImVec2ToTable();

    void MasterVolumeDSDImVec2ToTable();

    void ToneControlImVec2ToTable();

    void ToggleAudioAnalyzerOn() const;

    void ToggleAudioAnalyzerOff() const;

    static void ToggleDrawSettings(void *skin, void *);

    static void ToggleDrawEQTab(void *skin, void *);

    static void RandomizeTape(void *skin, void *);

    static void SaveConfig(void *skin);

    void Header();

    void Misc();

    void Website() const;

    void WalkmanOne();

    void WalkmanOneTab();

    void Wee1();

    void ReadLicense();

    void License() const;

    void License3rd() const;

    void DrawSkin();

    void Winamp();

    void Cassette();

    void DigitalClock();

    // some winamp skins are ugly with unreadable text
    // use default imgui style
    void DrawSettings();

    void PreprocessTableFilenames();

    static void CopyTableEntry(TableLike *table, std::vector<directoryEntry> *fileList, int *selectedIndex, const std::string &outDir);

    void TabMasterVolume();

    void TabMasterDSDVolume();

    void TabToneControl();

    void TabSoundStatus();

    void SoundSettingsTab();

    void CurveEditor();

    void Draw();

    void KeyHandler();

    void GetLogsDirSize();

    void SetActiveEqFilter(const std::string &v);

    void TabEQ_Donate() const;

    void TabEQ_Misc();

    void TabEq_EnableDisableFilter();

    void TabEQ_Vinylizer();

    void TabEQ_DCPhase();

    void TabEQ_Vpt();

    void TabEQ_DynamicNormalizer();

    void TabEQ_Eq6Band();

    void TabEQ_Eq10Band();

    void TabEQ_EqTone();

    void TabDac();

    void TabFM();

    void TabEQ();

    void TabEQOld();

    void CalcWindowPos() const;
};

#endif // WAMPY_SKIN_H
