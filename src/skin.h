#ifndef WAMPY_SKIN_H
#define WAMPY_SKIN_H

#include "Version.h"
#include "cassette/cassette.h"
#include "config.h"
#include "connector/hagoromoToString.h"
#include "dac/dac.h"
#include "digital_clock/digital_clock.h"
#include "imgui_curve.h"
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

    bool needRestartWalkmanOne{};

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

    int zoomDirection = 0;
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

    bool llusbdacLoaded;
    std::string llusbdacStatus;

    std::string bookmarkExportStatus;
    std::string logCleanupStatus;
    std::string logCleanupButtonLabel;
    std::string refreshStatus;
    std::string charactersInDB;

    std::string prevSong{};
    std::string eqStatus{};
    bool eqSongExists{};
    bool eqSongDirExists{};

    bool alwaysFalse{};

    std::string activeFilter;
    EActiveFilterTab eActiveFilterTab = ActiveFilterTab_Invalid;

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

    void WithWinampSkinDir(const std::string &d) { winampSkinDirectories.push_back(d); }

    void RefreshWinampSkinList() {
        DLOG("\n");
        assert(config);
        skinListWinamp.clear();
        for (const auto &d : winampSkinDirectories) {
            listdir(d.c_str(), &skinListWinamp, ".wsz");
        }

        for (int i = 0; i < skinListWinamp.size(); i++) {
            if (skinListWinamp.at(i).name == config->winamp.filename) {
                selectedSkinIdx = i;
                break;
            }
        }
    }

    void WithCassetteTapeDir(const std::string &d) { cassetteTapeDirectories.push_back(d); }

    void WithCassetteReelDir(const std::string &d) { cassetteReelDirectories.push_back(d); }

    void RefreshCassetteTapeReelLists() {
        DLOG("\n");
        assert(config);
        reelListCassette.clear();
        for (const auto &d : cassetteReelDirectories) {
            listdirs(d.c_str(), &reelListCassette);
        }

        tapeListCassette.clear();
        for (const auto &d : cassetteTapeDirectories) {
            listdirs(d.c_str(), &tapeListCassette);
        }
    }

    void WithMasterVolumeTableDirs(const std::string &d) { masterVolumeTableDirectories.push_back(d); };
    void RefreshMasterVolumeTableFiles() {
        DLOG("\n");
        masterVolumeFiles.clear();
        for (const auto &d : masterVolumeTableDirectories) {
            listdir(d.c_str(), &masterVolumeFiles, ".tbl");
        }

        if (masterVolumeFiles.empty()) {
            listdir("/system/usr/share/audio_dac/", &masterVolumeFiles, ".tbl");
        }
    }

    void WithMasterVolumeTableDSDDirs(const std::string &d) { masterVolumeTableDSDDirectories.push_back(d); };
    void RefreshMasterVolumeTableDSDFiles() {
        DLOG("\n");
        masterVolumeDSDFiles.clear();
        for (const auto &d : masterVolumeTableDSDDirectories) {
            listdir(d.c_str(), &masterVolumeDSDFiles, ".tbl");
        }

        if (masterVolumeDSDFiles.empty()) {
            listdir("/system/usr/share/audio_dac/", &masterVolumeDSDFiles, ".tbl");
        }
    }

    void WithToneControlTableDirs(const std::string &d) { toneControlTableDirectories.push_back(d); };
    void RefreshToneControlFiles() {
        DLOG("\n");
        toneControlFiles.clear();
        for (const auto &d : toneControlTableDirectories) {
            listdir(d.c_str(), &toneControlFiles, ".tbl");
        }

        if (toneControlFiles.empty()) {
            listdir("/system/usr/share/audio_dac/", &toneControlFiles, ".tbl");
        }
    }

    void ReloadFont() {
        std::string filepath{};
        switch (activeSkinVariant) {
        case CASSETTE:
            cassette.AddFonts(&FontRegular);
            break;
        case WINAMP:
            winamp.Unload();

            if (!SkinExists(config->winamp.filename, &skinListWinamp, &filepath)) {
                DLOG("no skin %s found in skinlist\n", config->winamp.filename.c_str());
                exit(1);
            }

            break;
        default:
            break;
        }

        config->Save();

        onlyFont = false;
    }

    void ReadQR() {
        Magick::Image i;
        i.read(qrPath);
        i.magick("RGBA");
        i.depth(8);
        Magick::Blob blob;
        i.write(&blob);
        qrSide = (float)i.size().width();
        LoadTextureFromMagic((unsigned char *)blob.data(), &qrTexture, (int)i.size().width(), (int)i.size().height());

        Magick::Image v;
        v.read(qrDonatePath);
        v.magick("RGBA");
        v.depth(8);
        Magick::Blob blob2;
        v.write(&blob2);
        LoadTextureFromMagic((unsigned char *)blob2.data(), &qrDonateTexture, (int)v.size().width(), (int)v.size().height());
    }

    void Load() {
        if (config->activeSkin == EMPTY) {
            DLOG("empty skin\n");
            createDump();
            exit(1);
        }

        DLOG("loading %d\n", config->activeSkin);
        if (onlyFont && activeSkinVariant == CASSETTE) {
            DLOG("reloading font for cassette\n");
            ReloadFont();
            return;
        }

        if (ImGui::GetCurrentContext()->WithinFrameScope) {
            DLOG("cannot load in frame, perhaps this https://github.com/ocornut/imgui/pull/3761 ?\n");
            return;
        }

        if (config->activeSkin == WINAMP) {
            DLOG("loading winamp\n");
            cassette.Unload();
            cassette.active = false;
            digitalClock.Unload();

            std::string filepath{};
            if (!SkinExists(config->winamp.filename, &skinListWinamp, &filepath)) {
                DLOG("no skin %s found in skin list\n", config->winamp.filename.c_str());
            }

            winamp.render = render;
            winamp.skin = (void *)this;
            winamp.WithConfig(&config->winamp);
            winamp.changeSkin(filepath);
            winamp.loadNewSkin(true);
            FontRegular = winamp.GetFont();
            winamp.active = true;
        }

        if (config->activeSkin == CASSETTE) {
            DLOG("loading cassette\n");
            winamp.Unload();
            winamp.active = false;
            digitalClock.Unload();

            cassette.reelList = &reelListCassette;
            cassette.tapeList = &tapeListCassette;
            cassette.render = render;
            cassette.skin = (void *)this;
            cassette.WithConfig(&config->cassette);
            cassette.Load("", &FontRegular);
            cassette.active = true;
        }

        if (config->activeSkin == DIGITAL_CLOCK) {
            DLOG("loading digital clock\n");
            winamp.Unload();
            winamp.active = false;
            cassette.Unload();
            cassette.active = false;

            digitalClock.skin = (void *)this;
            digitalClock.render = render;
            digitalClock.Load(config->digitalClock.color, &FontRegular);
        }

        activeSkinVariant = config->activeSkin;
        activeSettingsTab = activeSkinVariant;

        connector->soundSettings.Update();
        connector->soundSettingsFw.Update();
    }

    void LoadUpdatedSkin() {
        if (!needLoad) {
            return;
        }

        if (config->activeSkin != activeSkinVariant) {
            DLOG("changing active skin type from %d to %d\n", activeSkinVariant, config->activeSkin);
            Load();
            needLoad = false;
            return;
        }

        if (config->activeSkin == WINAMP) {
            winamp.loadNewSkin();
            FontRegular = winamp.GetFont();
            loadStatusStr = "✓ Loaded " + std::string(basename(winamp.GetCurrentSkin().c_str()));

            for (const auto &v : skinListWinamp) {
                if (winamp.GetCurrentSkin() == v.fullPath) {
                    config->winamp.filename = v.name;
                    config->Save();
                    DLOG("config updated\n");
                    break;
                }
            }
        }

        if (config->activeSkin == DIGITAL_CLOCK) {
            digitalClock.LoadNewSkin();
            FontRegular = digitalClock.GetFont();
            config->Save();
        }

        needLoad = false;
    }

    void MasterVolumeTableToImVec2() {
        for (int index = 0; index < 2; index++) {
            for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX; tableID++) {
                for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {

                    for (int valType = MASTER_VOLUME_VALUE_MIN; valType < MASTER_VOLUME_VALUE_MAX; valType++) {
                        auto val = masterVolume.GetValue(index, tableID, i, (MASTER_VOLUME_VALUE)valType);
                        //                        if (val != 0) {
                        //                            DLOG("Got %d -> %f, %f\n", val, (float)val, float(val));
                        //                        }
                        masterVolumeValues[index][tableID][valType][i].x = (float)i;
                        masterVolumeValues[index][tableID][valType][i].y = (float)val;
                    }
                }
            }
        }
    }

    void MasterVolumeDSDTableToImVec2() {
        for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX; tableID++) {
            for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
                auto val = masterVolumeDSD.v[tableID][i];
                masterVolumeDSDValues[tableID][i] = ImVec2((float)i, (float)val);
            }
        }
    }

    void ToneControlToImVec2() {
        for (int tableID = 0; tableID < TONE_CONTROL_TABLE_MAX; tableID++) {
            for (int i = 0; i < CODEC_RAM_SIZE; i++) {
                auto val = toneControl.v[tableID][i];
                toneControlValues[tableID][i] = ImVec2((float)i, (float)val);
            }
        }
    }

    void MasterVolumeImVec2ToTable() {
        for (int index = 0; index < 2; index++) {
            for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX; tableID++) {
                for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
                    for (int valType = 1; valType < MASTER_VOLUME_VALUE_MAX; valType++) {
                        auto val = masterVolumeValues[index][tableID][valType][i].y;
                        masterVolume.SetValue(index, tableID, i, (MASTER_VOLUME_VALUE)valType, uint(val));
                    }
                }
            }
        }
    }

    void MasterVolumeDSDImVec2ToTable() {
        for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX; tableID++) {
            for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
                for (int valType = 1; valType < MASTER_VOLUME_VALUE_MAX; valType++) {
                    auto val = masterVolumeDSDValues[tableID][i];
                    masterVolumeDSD.v[tableID][i] = (int)val.y;
                }
            }
        }
    }

    void ToneControlImVec2ToTable() {
        for (int tableID = 0; tableID < TONE_CONTROL_TABLE_MAX; tableID++) {
            for (int i = 0; i < CODEC_RAM_SIZE; i++) {
                auto val = toneControlValues[tableID][i];
                toneControl.v[tableID][i] = (int)val.y;
            }
        }
    }

    static void ToggleDrawSettings(void *skin, void *) {
        auto s = (Skin *)skin;
        assert(s);
        if (s->displaySettings == 0) {
            s->displaySettings = 1;
            s->RefreshWinampSkinList();
            s->RefreshCassetteTapeReelLists();
        } else {
            s->displaySettings = 0;
        }
    }

    static void ToggleDrawEQTab(void *skin, void *) {
        auto s = (Skin *)skin;
        assert(s);
        if (s->displaySettings == 0) {
            s->displaySettings = 1;
            s->displayTab = SettingsTab::TabEQ;
        } else {
            s->displaySettings = 0;
        }
    }

    static void RandomizeTape(void *skin, void *) {
        auto s = (Skin *)skin;
        assert(s);
        if (s->cassette.config->randomize) {
            s->cassette.SelectTape();
        }
    }

    void Header() {
        float offset = 15.0f;
        ImGui::Indent(offset);

        if (ImGui::Button("  W1  ")) {
            loadStatusStr = "";
            displayTab = SettingsTab::TabWalkmanOne;
        }

        ImGui::SameLine();
        if (ImGui::Button("VolT")) {
            loadStatusStr = "";
            displayTab = SettingsTab::TabSoundSettings;
            RefreshMasterVolumeTableFiles();
            RefreshMasterVolumeTableDSDFiles();
            RefreshToneControlFiles();
            PreprocessTableFilenames();
        }

        ImGui::SameLine();
        if (ImGui::Button("  EQ  ")) {
            loadStatusStr = "";
            displayTab = SettingsTab::TabEQ;
            connector->soundSettings.Update();
            connector->soundSettingsFw.Update();
            eqSongExists = SoundSettings::Exists(connector->status.Filename);
            eqSongDirExists = SoundSettings::ExistsDir(connector->status.Filename);
            prevSong = connector->status.Filename;
            eqStatus = "Refreshed";
        }

        ImGui::SameLine();
        if (ImGui::Button("EQ/Song")) {
            loadStatusStr = "";
            displayTab = SettingsTab::TabEQOld;

            connector->soundSettings.Update();
            connector->soundSettingsFw.Update();
            eqSongExists = SoundSettings::Exists(connector->status.Filename);
            eqSongDirExists = SoundSettings::ExistsDir(connector->status.Filename);
            prevSong = connector->status.Filename;
            eqStatus = "Refreshed";
        }

        ImGui::SameLine();
        if (ImGui::Button("DAC")) {
            loadStatusStr = "";
            displayTab = SettingsTab::TabDAC;
        }

        ImGui::SameLine();
        if (ImGui::Button(" FM ")) {
            loadStatusStr = "";
            displayTab = SettingsTab::TabFM;
            fmFreq = connector->soundSettings.s->fmStatus.freq;
            sprintf(fmFreqFormat, "%.1fMHz", float(connector->soundSettings.s->fmStatus.freq) / 1000);
        }

        auto miscSize = ImGui::CalcTextSize("Misc").x + ImGui::GetStyle().FramePadding.x * 2.f;
        auto skinSize = ImGui::CalcTextSize("Skin").x + ImGui::GetStyle().FramePadding.x * 2.f;
        auto closeSize = ImGui::CalcTextSize("Close").x + ImGui::GetStyle().FramePadding.x * 2.f;

        // clock
        char buffer[9];
        time_t rawtime;
        time(&rawtime);
        const auto timeinfo = localtime(&rawtime);
        strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);

        auto skinButtonPos = 800.0f - closeSize - miscSize - skinSize - offset * 3;

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((skinButtonPos - ImGui::GetCursorPosX()) / 2) - ImGui::CalcTextSize(buffer).x / 2);
        ImGui::Text("%s", buffer);

        ImGui::SameLine(skinButtonPos);
        if (ImGui::Button("Skin")) {
            loadStatusStr = "";
            displayTab = SettingsTab::SkinOpts;
            switch (activeSettingsTab) {
            case WINAMP:
                RefreshWinampSkinList();
                break;
            case CASSETTE:
                RefreshCassetteTapeReelLists();
                break;
            default:
            case DIGITAL_CLOCK:
                break;
            }
        }

        ImGui::SameLine(800.0f - closeSize - miscSize - offset * 2);
        if (ImGui::Button("Misc")) {
            loadStatusStr = "";
            displayTab = SettingsTab::Misc;
        }

        ImGui::SameLine(800.0f - closeSize - offset);
        if (ImGui::Button("Close")) {
            loadStatusStr = "";
            ToggleDrawSettings(this, nullptr);
        }
    }

    void Misc() {
        ImGui::NewLine();
        if (ImGui::BeginTable("##misctable", 4, ImGuiTableFlags_None)) {
            // #ifndef DESKTOP
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("Swap prev/next buttons", &config->misc.swapTrackButtons)) {
                config->Save();
            }
            ImGui::TableNextColumn();
            ImGui::Text("     "); // padding

            ImGui::TableNextColumn();
            if (ImGui::Button("Export Walkman bookmarks")) {
                ExportBookmarks();
                bookmarkExportStatus = "Exported";
            }

            ImGui::TableNextColumn();
            ImGui::Text(bookmarkExportStatus.c_str());
            ImGui::SameLine(20);
            if (ImGui::InvisibleButton("##bookmarkStatus", ImVec2(246, 30))) {
                bookmarkExportStatus = "";
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("Huge cover art", &config->features.bigCover)) {
                config->Save();
                connector->FeatureBigCover(config->features.bigCover);
            }

            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            if (ImGui::Button(logCleanupButtonLabel.c_str())) {
                RemoveLogs();
                logCleanupStatus = "Removed!";
                GetLogsDirSize();
            }

            ImGui::TableNextColumn();
            ImGui::Text(logCleanupStatus.c_str());
            ImGui::SameLine(20);
            if (ImGui::InvisibleButton("##logCleanupStatus", ImVec2(246, 30))) {
                logCleanupStatus = "";
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (isWalkmanOne) {
                ImGui::BeginDisabled();
                ImGui::Checkbox("Show time", &alwaysFalse);
                ImGui::EndDisabled();
            } else {
                if (ImGui::Checkbox("Show time", &config->features.showTime)) {
                    config->Save();
                    connector->FeatureShowTime(config->features.showTime);
                }
            }

            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            if (ImGui::Button("DB char count")) {
                std::vector<uint32_t> chars;
                getCharRange(&chars);
                charactersInDB = std::to_string(chars.size());
            }

            ImGui::TableNextColumn();
            ImGui::Text(charactersInDB.c_str());
            ImGui::SameLine(20);
            if (ImGui::InvisibleButton("##charactersInDBCount", ImVec2(246, 30))) {
                charactersInDB = "";
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("Limit max volume", &config->features.limitVolume)) {
                config->Save();
                connector->FeatureSetMaxVolume(config->features.limitVolume);
            }

            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            ImGui::Text("Window position");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("Enable EQ per song", &config->features.eqPerSong)) {
                config->Save();
            }

            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            if (ImGui::BeginCombo("##windowPos", WindowOffsetToString.at(config->windowOffset).c_str(), ImGuiComboFlags_HeightRegular)) {
                for (const auto &entry : WindowOffsetToString) {
                    if (ImGui::Selectable(entry.second.c_str(), false)) {
                        config->windowOffset = entry.first;
                        DLOG("selected offset %s\n", WindowOffsetToString.at(config->windowOffset).c_str());
                        config->Save();
                        CalcWindowPos();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("Disable touchscreen", &config->features.touchscreenStaysOFF)) {
                config->Save();
            }
            ImGui::TableNextColumn();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("");

            ImGui::TableNextColumn();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("Debug", &config->debug)) {
                winamp.debug = config->debug;
                cassette.debug = config->debug;
                config->Save();
            }
            ImGui::TableNextColumn();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("Limit fps", &config->limitFPS)) {
                config->Save();
            }
            ImGui::TableNextColumn();

            ImGui::EndTable();
        }

        auto website = ImGui::CalcTextSize("Website / Donate");
        auto verSize = ImGui::CalcTextSize(SOFTWARE_VERSION);
        auto licenseSize = ImGui::CalcTextSize("License");
        auto license3Size = ImGui::CalcTextSize("License 3rdparty");
        auto offset = 15.0f;

        // #ifndef DESKTOP
        if (config->debug) {

            ImGui::SetCursorPosY(480 - verSize.y - license3Size.y * 2 - ImGui::GetStyle().FramePadding.y * 3 - offset);
            if (ImGui::Button("Start ADB daemon (next boot)")) {
                startADB();
            }

            ImGui::SetCursorPosY(480 - verSize.y - license3Size.y - ImGui::GetStyle().FramePadding.y * 2);
            if (ImGui::Button("Create log file")) {
                createDump();
            }
        }
        // #endif
        ImGui::SetCursorPosY(480 - verSize.y - ImGui::GetStyle().FramePadding.y);
        printFPS();

        ImGui::SetCursorPosY(480 - verSize.y - licenseSize.y * 3 - ImGui::GetStyle().FramePadding.y * 2 - offset * 2 - 30);
        ImGui::SetCursorPosX(800 - website.x - ImGui::GetStyle().FramePadding.x - offset - 9);
        ImGui::PushStyleColor(ImGuiCol_Button, GOLD_DONATE);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1.0f));
        if (ImGui::Button("Website / Donate", ImVec2(200, 60))) {
            displayTab = SettingsTab::TabWebsite;
            loadStatusStr = "";
        }
        ImGui::PopStyleColor(2);

        ImGui::SetCursorPosY(480 - verSize.y - licenseSize.y * 2 - ImGui::GetStyle().FramePadding.y * 2 - offset);
        ImGui::SetCursorPosX(800 - licenseSize.x - ImGui::GetStyle().FramePadding.x - offset);
        if (ImGui::Button("License")) {
            displayTab = SettingsTab::TabLicense;
            loadStatusStr = "";
        }

        ImGui::SetCursorPosY(480 - verSize.y - license3Size.y - ImGui::GetStyle().FramePadding.y * 2);
        ImGui::SetCursorPosX(800 - license3Size.x - ImGui::GetStyle().FramePadding.x - offset);
        if (ImGui::Button("License 3rdparty")) {
            displayTab = SettingsTab::TabLicense3rd;
            loadStatusStr = "";
        }

        ImGui::SetCursorPosX(800 - verSize.x - ImGui::GetStyle().FramePadding.x);
        ImGui::SetCursorPosY(480 - verSize.y - ImGui::GetStyle().FramePadding.y);
        ImGui::Text("%s", SOFTWARE_VERSION);
    }

    void Website() const {
        ImGui::NewLine();
        float columnWidth = 380.0f;
        if (ImGui::BeginTable("##website", 2, ImGuiTableFlags_None)) {
            ImGui::TableSetupColumn("Github", ImGuiTableColumnFlags_WidthFixed, 380);
            ImGui::TableSetupColumn("Donate", ImGuiTableColumnFlags_WidthFixed, 380);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + columnWidth / 2 - ImGui::CalcTextSize("Github").x / 2);
            ImGui::Text("Github");
            ImGui::TableNextColumn();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + columnWidth / 2 - ImGui::CalcTextSize("Donate").x / 2);
            ImGui::TextColored(GOLD_DONATE, "Donate");

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + columnWidth / 2 - qrSide / 2);
            ImGui::Image((void *)(intptr_t)qrTexture, ImVec2(qrSide, qrSide));

            ImGui::TableNextColumn();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + columnWidth / 2 - qrSide / 2);
            ImGui::Image((void *)(intptr_t)qrDonateTexture, ImVec2(qrSide, qrSide));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + columnWidth / 2 - ImGui::CalcTextSize("github.com/unknown321/wampy").x / 2);
            ImGui::Text("github.com/unknown321/wampy");

            ImGui::TableNextColumn();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + columnWidth / 2 - ImGui::CalcTextSize("boosty.to/unknown321/donate").x / 2);
            ImGui::TextColored(GOLD_DONATE, "boosty.to/unknown321/donate");

            ImGui::EndTable();
        }
    }

    void WalkmanOne() {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 15.0f));
        ImGui::SeparatorText("Walkman One settings");
        ImGui::PopStyleVar();

        if (!walkmanOneOptions.configFound) {
            ImGui::Text("config file not found\n");
            return;
        }

        static ImGuiTableFlags flags =
            ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchSame;

        ImVec2 outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 3);

        if (ImGui::BeginTable("walkmanOneTable", 2, flags, outer_size)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Interface color");

            ImGui::TableNextColumn();
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
            ImGui::PushItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo(
                    "##walkmanOneColor", W1::colorByValueWalkmanOne.at(walkmanOneOptions.color).c_str(), ImGuiComboFlags_HeightRegular
                )) {
                for (const auto &entry : W1::colorByNameWalkmanOne) {
                    if (ImGui::Selectable(entry.first.c_str(), false)) {
                        walkmanOneOptions.color = entry.second;
                        DLOG("selected color %s\n", W1::colorByValueWalkmanOne.at(walkmanOneOptions.color).c_str());
                        walkmanOneOptions.Save();
                        needRestartWalkmanOne = true;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopStyleVar(2);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Sound signature");

            ImGui::TableNextColumn();
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
            if (ImGui::BeginCombo(
                    "##walkmanOneSignature",
                    W1::signatureByValueWalkmanOne.at(walkmanOneOptions.signature).c_str(),
                    ImGuiComboFlags_HeightRegular
                )) {
                for (const auto &entry : W1::signatureByNameWalkmanOne) {
                    if (ImGui::Selectable(entry.first.c_str(), false)) {
                        walkmanOneOptions.signature = entry.second;
                        DLOG("selected signature %s\n", W1::signatureByValueWalkmanOne.at(walkmanOneOptions.signature).c_str());
                        walkmanOneOptions.Save();
                        needRestartWalkmanOne = true;
                        walkmanOneOptions.tuningChanged = true;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopStyleVar(2);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Region");

            ImGui::TableNextColumn();
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
            if (ImGui::BeginCombo("##walkmanOneRegion", walkmanOneOptions.region, ImGuiComboFlags_HeightRegular)) {
                for (const auto &entry : W1::regionWalkmanOne) {
                    if (ImGui::Selectable(entry.c_str(), false)) {
                        strncpy(walkmanOneOptions.region, entry.c_str(), sizeof walkmanOneOptions.region);
                        DLOG("selected region %s\n", entry.c_str());
                        walkmanOneOptions.Save();
                        needRestartWalkmanOne = true;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopStyleVar(2);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Remote option with any region");
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("##remote", &walkmanOneOptions.remote)) {
                DLOG("Set remote option to %d\n", walkmanOneOptions.remote);
                walkmanOneOptions.Save();
                needRestartWalkmanOne = true;
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Plus mode v2");
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("##plusv2", &walkmanOneOptions.plusModeVersionBOOL)) {
                DLOG("Set plus mode v2 to %d\n", walkmanOneOptions.plusModeVersionBOOL);
                walkmanOneOptions.Save();
                needRestartWalkmanOne = true;
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Plus mode by default");
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("##plusdefault", &walkmanOneOptions.plusModeByDefault)) {
                DLOG("Set plus mode default to %d\n", walkmanOneOptions.plusModeByDefault);
                walkmanOneOptions.Save();
                needRestartWalkmanOne = true;
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Lower gain mode");
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("##gainmode", &walkmanOneOptions.gainMode)) {
                DLOG("Set gain mode to %d\n", walkmanOneOptions.gainMode);
                walkmanOneOptions.Save();
                needRestartWalkmanOne = true;
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Different DAC init mode");
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("##dacinitmode", &walkmanOneOptions.dacInitializationMode)) {
                DLOG("Set gain mode to %d\n", walkmanOneOptions.dacInitializationMode);
                walkmanOneOptions.Save();
                needRestartWalkmanOne = true;
            }

            ImGui::EndTable();
        }

        ImGui::NewLine();
        if (needRestartWalkmanOne) {
#ifndef DESKTOP
            if (walkmanOneOptions.tuningChanged) {
                if (ImGui::Button("Apply tuning", ImVec2(200, 60))) {
                    walkmanOneOptions.ApplyTuning();
                    walkmanOneOptions.tuningChanged = false;
                }
            } else {
                ImGui::Text("Reboot the device to apply changes");
            }
#endif
        }
    }

    void WalkmanOneTab() {
        if (isWalkmanOne) {
            WalkmanOne();
        } else {
            Wee1();
        }
    }

    void Wee1() {
        ImGui::NewLine();
        ImGui::Text("Interface color");

        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        if (ImGui::BeginCombo("##w1color", W1::colorByValue.at(w1Options.deviceColor).c_str(), ImGuiComboFlags_HeightRegular)) {
            for (const auto &entry : W1::colorByName) {
                if (ImGui::Selectable(entry.first.c_str(), false)) {
                    w1Options.deviceColor = entry.second;
                    DLOG("selected %s\n", W1::colorByValue.at(w1Options.deviceColor).c_str());
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleVar(2);
        ImGui::SameLine();
        if (ImGui::Button("Change color")) {
            W1::SetColor(w1Options.deviceColor);
            needRestartWalkmanOne = true;
        }

        if (needRestartWalkmanOne) {
            ImGui::NewLine();
            ImGui::Text("Reboot the device to apply changes");
            //            if (ImGui::Button("Reboot device", ImVec2(200, 60))) {
            //                DLOG("rebooting\n");
            // #ifndef DESKTOP
            //                system("sync");
            //                system("reboot");
            // #endif
            //            }
        }
    }

    void ReadLicense() {
        std::ifstream f;
        f.open(licensePath);
        std::string contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        license = contents;
        f.close();

        f.open(license3rdPath);
        std::string contents2((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        license3rd = contents2;
        f.close();
    }

    void License() const {
        ImGui::SetCursorPosY(60);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        ImGui::BeginChild("licenseText", ImVec2(740, 400));
        ImGui::PushTextWrapPos(740 + 18 - 40 - 6);
        ImGui::Text("%s", license.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
    }

    void License3rd() const {
        ImGui::SetCursorPosY(60);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        ImGui::BeginChild("license3Text", ImVec2(740, 400));
        ImGui::PushTextWrapPos(740 + 18 - 40 - 6);
        ImGui::Text("%s", license3rd.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
    }

    void DrawSkin() {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 15.0f));
        ImGui::SeparatorText("Skin:");
        ImGui::PopStyleVar();

        ImGui::RadioButton("Winamp", &activeSettingsTab, WINAMP);
        ImGui::SameLine();
        if (ImGui::RadioButton("Cassette", &activeSettingsTab, CASSETTE)) {
            loadStatusStr = "";
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("DigiClock", &activeSettingsTab, DIGITAL_CLOCK)) {
            loadStatusStr = "";
        }

        if (activeSkinVariant != activeSettingsTab) {
            ImGui::SameLine();
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 10);
            if (ImGui::Button("Set as active", ImVec2(186, 50))) {
                switch (activeSettingsTab) {
                case WINAMP:
                    config->activeSkin = WINAMP;
                    break;
                case CASSETTE:
                    config->activeSkin = CASSETTE;
                    break;
                case DIGITAL_CLOCK:
                    config->activeSkin = DIGITAL_CLOCK;
                    break;
                default:
                    break;
                }

                DLOG("changed active skin type to %d\n", config->activeSkin);

                config->Save();

                needLoad = true;
            }
        }

        ImGui::NewLine();

        if (activeSettingsTab == WINAMP) {
            Winamp();
        } else if (activeSettingsTab == CASSETTE) {
            Cassette();
        } else if (activeSettingsTab == DIGITAL_CLOCK) {
            DigitalClock();
        }
    }

    void Winamp() {

        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        if (ImGui::BeginCombo("##", skinListWinamp.at(selectedSkinIdx).name.c_str(), ImGuiComboFlags_HeightRegular)) {
            for (int n = 0; n < skinListWinamp.size(); n++) {
                const bool is_selected = (selectedSkinIdx == n);
                if (ImGui::Selectable(skinListWinamp.at(n).name.c_str(), is_selected)) {
                    selectedSkinIdx = n;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleVar(2);

        ImGui::SameLine();
        if (activeSkinVariant == WINAMP) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 11);
            if (ImGui::Button("Load skin", ImVec2(186, 50))) {
                loadStatusStr = "Loading " + skinListWinamp.at(selectedSkinIdx).name;
                winamp.changeSkin(skinListWinamp.at(selectedSkinIdx).fullPath);
                needLoad = true;
            }
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 8);
        ImGui::Text("%s", loadStatusStr.c_str());
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

        if (ImGui::Checkbox("Use bitmap font", &config->winamp.useBitmapFont)) {
            config->Save();
            if (activeSkinVariant == WINAMP) {
                winamp.Format(true);
            }
        }

        if (ImGui::Checkbox("Use bitmap font in playlist", &config->winamp.useBitmapFontInPlaylist)) {
            config->Save();
            if (activeSkinVariant == WINAMP) {
                winamp.Format(true);
            }
        }

        if (ImGui::Checkbox("Prefer time remaining", &config->winamp.preferTimeRemaining)) {
            config->Save();
        }

        if (ImGui::Checkbox("Show clutterbar", &config->winamp.showClutterbar)) {
            config->Save();
        }

        if (ImGui::Checkbox("Skin transparency", &config->winamp.skinTransparency)) {
            config->Save();
        }
    }

    void Cassette() {
        if (ImGui::Checkbox("Randomize?", &config->cassette.randomize)) {
            config->Save();
        }

        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                                       ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchProp;

        ImVec2 outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 7);

        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        if (ImGui::BeginTable("configTable", 3, flags, outer_size)) {
            ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
            ImGui::TableSetupColumn("Codec", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Tape", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Reel", ImGuiTableColumnFlags_None);
            ImGui::TableHeadersRow();

            for (auto &tt : config->cassette.data) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", tt.second.name.c_str());

                ImGui::TableNextColumn();
                if (ImGui::BeginCombo(("##" + tt.second.name + "tape").c_str(), tt.second.tape.c_str(), ImGuiComboFlags_HeightSmall)) {
                    for (auto &n : tapeListCassette) {
                        if (!n.valid) {
                            continue;
                        }

                        if (ImGui::Selectable(n.name.c_str(), false)) {
                            tt.second.tape = n.name;
                            config->cassette.data.at(tt.first).tape = n.name;
                            config->Save();

                            if (activeSkinVariant == CASSETTE) {
                                cassette.UnloadUnused();
                                cassette.LoadTape(n.name);
                                cassette.SelectTape();
                            }
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::TableNextColumn();
                if (ImGui::BeginCombo(("##" + tt.second.name + "reel").c_str(), tt.second.reel.c_str(), ImGuiComboFlags_HeightSmall)) {
                    for (auto &n : reelListCassette) {
                        if (!n.valid) {
                            continue;
                        }

                        if (ImGui::Selectable(n.name.c_str(), false)) {
                            tt.second.reel = n.name;
                            config->cassette.data.at(tt.first).reel = n.name;
                            config->Save();

                            if (activeSkinVariant == CASSETTE) {
                                cassette.UnloadUnused();
                                cassette.LoadReel(n.name);
                                cassette.SelectTape();
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleVar(2);

        if (ImGui::Button("Reset", ImVec2(186, 60))) {
            DLOG("resetting cassette config\n");
            cassette.config->Default();
            config->Save();
            if (activeSkinVariant == CASSETTE) {
                cassette.LoadImages();
                cassette.SelectTape();
                cassette.UnloadUnused();
            }
        }
    }

    void DigitalClock() {
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        if (ImGui::BeginCombo(
                "##digiClockColor",
                DigitalClock::DigitalClock::GetColorPreview(config->digitalClock.color).c_str(),
                ImGuiComboFlags_HeightRegular
            )) {
            for (const auto &entry : DigitalClock::colorsDigitalClock) {
                if (ImGui::Selectable(entry.first.c_str(), false)) {
                    DLOG("selected color %s\n", entry.second.c_str());
                    digitalClock.SetColor(entry.second);
                    if (activeSkinVariant == DIGITAL_CLOCK) {
                        needLoad = true;
                    } else {
                        config->digitalClock.color = entry.second;
                        config->Save();
                    }
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleVar(2);
    }

    // some winamp skins are ugly with unreadable text
    // use default imgui style
    void DrawSettings() {
        if (!*render) {
            return;
        }

        FontRegular->FontSize = SETTINGS_FONT_SIZE;

        ImGui::PushFont(FontRegular);

        // no header for curve editor only
        if (displayTab == SettingsTab::TabCurveEditor) {
            CurveEditor();

            ImGui::PopFont();
            return;
        }

        Header();
        switch (displayTab) {
        case SettingsTab::SkinOpts:
            DrawSkin();
            break;
        case SettingsTab::Misc:
            Misc();
            break;
        case SettingsTab::TabLicense:
            License();
            break;
        case SettingsTab::TabLicense3rd:
            License3rd();
            break;
        case SettingsTab::TabWebsite:
            Website();
            break;
        case SettingsTab::TabWalkmanOne:
            WalkmanOneTab();
            break;
        case SettingsTab::TabSoundSettings:
            SoundSettingsTab();
            break;
        case SettingsTab::TabEQ:
            TabEQ();
            break;
        case SettingsTab::TabEQOld:
            TabEQOld();
            break;
        case SettingsTab::TabDAC:
            TabDac();
            break;
        case SettingsTab::TabFM:
            TabFM();
            break;
        default:
            break;
        }

        ImGui::PopFont();
    }

    void PreprocessTableFilenames() {
        std::vector<std::vector<directoryEntry> *> tables = {&masterVolumeFiles, &masterVolumeDSDFiles, &toneControlFiles};
        for (auto &table : tables) {
            for (int i = 0; i < table->size(); i++) {
                auto entry = table->at(i);
                if (entry.fullPath.rfind(soundSettingsPathSystemWampy, 0) == 0) {
                    table->at(i).name = systemMark + entry.name;
                    continue;
                }
                if (entry.fullPath.rfind(soundSettingsPathSystemHagoromo, 0) == 0) {
                    table->at(i).name = systemMarkHagoromo + entry.name;
                    continue;
                }
            }
        }
    }

    static void CopyTableEntry(TableLike *table, std::vector<directoryEntry> *fileList, int *selectedIndex, const std::string &outDir) {
        mkpath(outDir.c_str(), 0755);

        auto d = directoryEntry{};
        d.name = basename(fileList->at(*selectedIndex).fullPath.c_str());
        d.fullPath = outDir + d.name;
        d.valid = true;

        DLOG("copying to %s\n", d.fullPath.c_str());

        table->ToFile(d.fullPath);

        bool exists{};
        for (int i = 0; i < fileList->size(); i++) {
            if (fileList->at(i).fullPath == d.fullPath) {
                exists = true;
                *selectedIndex = i;
                break;
            }
        }

        if (!exists) {
            fileList->push_back(d);
            *selectedIndex = (int)fileList->size() - 1;
        }
    }

    void TabMasterVolume() {
        if (ImGui::BeginTabItem(Dac::TableTypeToString.at(TABLE_ID_MASTER_VOLUME).c_str())) {
            curveEditorTarget = (float *)masterVolumeValues[int(soundEffectOn)][MasterVolumeTableType][MasterVolumeValueType];
            curveYLimit = (1 << 8) - 1; // 255
            curveElementCount = MASTER_VOLUME_MAX + 1;

            if (ImGui::BeginTable("##mvtable", 3, ImGuiTableFlags_None)) {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, 384);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("File:");

                ImGui::TableNextColumn();
                ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
                ImGui::PushItemWidth(-FLT_MIN);
                if (ImGui::BeginCombo(
                        "##masterVolumeFileCombo",
                        masterVolumeFiles.at(masterVolumeFileSelected).name.c_str(),
                        ImGuiComboFlags_HeightRegular
                    )) {
                    for (int idx = 0; idx < masterVolumeFiles.size(); idx++) {
                        auto entry = masterVolumeFiles.at(idx);
                        if (ImGui::Selectable(entry.name.c_str(), false)) {
                            masterVolumeFileSelected = idx;
                            DLOG("selected file %s\n", entry.name.c_str());
                            masterVolume.Reset();
                            memset(&masterVolumeValues, 0, sizeof(masterVolumeValues));
                            if (masterVolume.FromFile(entry.fullPath) != 0) {
                                DLOG("failed to load master volume table file %s\n", entry.fullPath.c_str());
                                statusStringMasterVolume = "Failed";
                            } else {
                                MasterVolumeTableToImVec2();
                                statusStringMasterVolume = "Loaded";
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopItemWidth();
                ImGui::PopStyleVar(2);

                ImGui::TableNextColumn();

                if (!statusStringMasterVolume.empty()) {
                    ImGui::Text(statusStringMasterVolume.c_str());
                    ImGui::SameLine(20);
                    if (ImGui::InvisibleButton("##statusmastervolume", ImVec2(246, 30))) {
                        statusStringMasterVolume = "";
                    }
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Table type:");
                ImGui::TableNextColumn();
                ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
                const char *preview;
                preview = Dac::MasterVolumeTableTypeToString.at(MasterVolumeTableType).c_str();
                ImGui::PushItemWidth(-FLT_MIN);
                if (ImGui::BeginCombo("##masterVolumeTypeCombo", preview, ImGuiComboFlags_HeightRegular)) {
                    for (const auto &entry : Dac::MasterVolumeTableTypeToString) {
                        if (ImGui::Selectable(entry.second.c_str(), false)) {
                            MasterVolumeTableType = entry.first;
                            DLOG("selected type %s\n", entry.second.c_str());
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopItemWidth();
                ImGui::PopStyleVar(2);
                ImGui::TableNextColumn();

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Value type:");
                ImGui::TableNextColumn();

                ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
                ImGui::PushItemWidth(-FLT_MIN);
                if (ImGui::BeginCombo(
                        "##masterVolumeValueTypeCombo",
                        Dac::MasterVolumeValueTypeToString.at(MasterVolumeValueType).c_str(),
                        ImGuiComboFlags_HeightRegular
                    )) {
                    for (const auto &entry : Dac::MasterVolumeValueTypeToString) {
                        if (ImGui::Selectable(entry.second.c_str(), false)) {
                            MasterVolumeValueType = entry.first;
                            DLOG("selected value type %s (%d)\n", entry.second.c_str(), entry.first);
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopItemWidth();
                ImGui::PopStyleVar(2);

                ImGui::TableNextColumn();
                ImGui::Checkbox("Sound effect", &soundEffectOn);
            }
            ImGui::EndTable();

            if (ImGui::BeginTable("##mvtcontent", 2, ImGuiTableFlags_None)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (masterVolumeValues[0][0][0][0].x == -FLT_MIN) {
                    ImGui::NewLine();
                    if (ImGui::Button("Load", ImVec2(512, 150))) {
                        if (masterVolume.FromFile(masterVolumeFiles.at(masterVolumeFileSelected).fullPath) != 0) {
                            DLOG(
                                "failed to load master volume table file %s\n",
                                masterVolumeFiles.at(masterVolumeFileSelected).fullPath.c_str()
                            );
                            statusStringMasterVolume = "Failed";
                        } else {
                            MasterVolumeTableToImVec2();
                            statusStringMasterVolume = "Loaded";
                        }
                    }
                } else {
                    CurveEditorSmall(&connector->status.VolumeRaw, ImVec2(512, 270));

                    ImGui::TableNextColumn();

                    // not efficient at all
                    if (masterVolumeFiles.at(masterVolumeFileSelected).name.rfind(systemMark, 0) == 0) {
                        if (ImGui::Button("Copy and edit", ImVec2(246, 60))) {
                            auto outDir = soundSettingsPathUser + "master_volume/";
                            CopyTableEntry(&masterVolume, &masterVolumeFiles, &masterVolumeFileSelected, outDir);
                            displayTab = TabCurveEditor;
                        }
                    } else {
                        if (ImGui::Button("Edit", ImVec2(246, 60))) {
                            displayTab = TabCurveEditor;
                        }
                    }

                    if (masterVolumeFiles.at(masterVolumeFileSelected).name.rfind(systemMark, 0) == 0) {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::Button("Save", ImVec2(246, 60))) {
                        auto out = masterVolumeFiles.at(masterVolumeFileSelected).fullPath;
                        DLOG("Saving to %s\n", out.c_str());
                        MasterVolumeImVec2ToTable();
                        if (masterVolume.ToFile(out) == 0) {
                            statusStringMasterVolume = "Saved";
                        } else {
                            statusStringMasterVolume = "Failed";
                        }
                    }

                    if (masterVolumeFiles.at(masterVolumeFileSelected).name.rfind(systemMark, 0) == 0) {
                        ImGui::EndDisabled();
                    }

                    if (ImGui::Button("Apply", ImVec2(246, 60))) {
                        DLOG("Applying\n");
                        MasterVolumeImVec2ToTable();
                        if (masterVolume.Apply(Dac::volumeTableOutPath) == 0) {
                            statusStringMasterVolume = "Applied";
                        } else {
                            statusStringMasterVolume = "Failed";
                        }
                    }

                    if (ImGui::Button("Copy val", ImVec2(121, 60))) {
                        memcpy(
                            masterVolumeValueBuffer,
                            masterVolumeValues[(int)soundEffectOn][MasterVolumeTableType][MasterVolumeValueType],
                            sizeof(masterVolumeValueBuffer)
                        );
                        statusStringMasterVolume = "Copied";
                    }
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));

                    ImGui::SameLine();
                    if (ImGui::Button("Paste val", ImVec2(121, 60))) {
                        memcpy(
                            masterVolumeValues[(int)soundEffectOn][MasterVolumeTableType][MasterVolumeValueType],
                            masterVolumeValueBuffer,
                            sizeof(masterVolumeValueBuffer)
                        );
                        statusStringMasterVolume = "Pasted";
                    }
                    ImGui::PopStyleVar();
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }
    }

    void TabMasterDSDVolume() {
        if (ImGui::BeginTabItem(Dac::TableTypeToString.at(TABLE_ID_MASTER_VOLUME_DSD).c_str())) {
            curveEditorTarget = (float *)masterVolumeDSDValues[MasterVolumeDSDTableType];
            curveYLimit = (1 << 15) - 1; // 32767
            curveElementCount = MASTER_VOLUME_MAX + 1;

            if (ImGui::BeginTable("##mvDSDtable", 3, ImGuiTableFlags_None)) {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, 384);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("File:");

                ImGui::TableNextColumn();
                ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
                ImGui::PushItemWidth(-FLT_MIN);
                if (ImGui::BeginCombo(
                        "##masterVolumeDSDFileCombo",
                        masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).name.c_str(),
                        ImGuiComboFlags_HeightRegular
                    )) {
                    for (int idx = 0; idx < masterVolumeDSDFiles.size(); idx++) {
                        auto entry = masterVolumeDSDFiles.at(idx);
                        if (ImGui::Selectable(entry.name.c_str(), false)) {
                            DLOG("selected dsd file %s\n", entry.fullPath.c_str());
                            masterVolumeDSDFileSelected = idx;
                            masterVolumeDSD.Reset();
                            memset(&masterVolumeDSDValues, 0, sizeof(masterVolumeDSDValues));
                            if (masterVolumeDSD.FromFile(entry.fullPath) != 0) {
                                DLOG("failed to load master volume dsd table file %s\n", entry.fullPath.c_str());
                                statusStringMasterVolumeDSD = "Failed";
                            } else {
                                MasterVolumeDSDTableToImVec2();
                                statusStringMasterVolumeDSD = "Loaded";
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopItemWidth();
                ImGui::PopStyleVar(2);

                ImGui::TableNextColumn();
                if (!statusStringMasterVolumeDSD.empty()) {
                    ImGui::Text(statusStringMasterVolumeDSD.c_str());
                    ImGui::SameLine(20);
                    if (ImGui::InvisibleButton("##statusmastervolumeDSD", ImVec2(246, 30))) {
                        statusStringMasterVolumeDSD = "";
                    }
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Table type:");

                ImGui::TableNextColumn();
                ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
                ImGui::PushItemWidth(-FLT_MIN);
                const char *preview;
                preview = Dac::MasterVolumeTableTypeToString.at(MasterVolumeDSDTableType).c_str();
                if (ImGui::BeginCombo("##masterVolumeDSDTypeCombo", preview, ImGuiComboFlags_HeightRegular)) {
                    for (const auto &entry : Dac::MasterVolumeTableTypeToString) {
                        if (ImGui::Selectable(entry.second.c_str(), false)) {
                            MasterVolumeDSDTableType = entry.first;
                            DLOG("selected dsd type %s\n", entry.second.c_str());
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopItemWidth();
                ImGui::PopStyleVar(2);
                ImGui::TableNextColumn();

                ImGui::EndTable();
            }

            if (ImGui::BeginTable("##mvtDSDcontent", 2, ImGuiTableFlags_None)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (masterVolumeDSDValues[0][0].x == -FLT_MIN) {
                    ImGui::NewLine();
                    if (ImGui::Button("Load", ImVec2(512, 150))) {
                        if (masterVolumeDSD.FromFile(masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).fullPath) != 0) {
                            DLOG(
                                "failed to load master volume DSD table file %s\n",
                                masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).fullPath.c_str()
                            );
                            statusStringMasterVolumeDSD = "Failed";
                        } else {
                            MasterVolumeDSDTableToImVec2();
                            statusStringMasterVolumeDSD = "Loaded";
                        }
                    }
                } else {
                    CurveEditorSmall(&connector->status.VolumeRaw, ImVec2(512, 305));

                    ImGui::TableNextColumn();

                    // not efficient at all
                    if (masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).name.rfind(systemMark, 0) == 0) {
                        if (ImGui::Button("Copy and edit", ImVec2(246, 60))) {
                            auto outDir = soundSettingsPathUser + "master_volume_dsd/";
                            CopyTableEntry(&masterVolumeDSD, &masterVolumeDSDFiles, &masterVolumeDSDFileSelected, outDir);
                            displayTab = TabCurveEditor;
                        }
                    } else {
                        if (ImGui::Button("Edit", ImVec2(246, 60))) {
                            displayTab = TabCurveEditor;
                        }
                    }

                    if (masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).name.rfind(systemMark, 0) == 0) {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::Button("Save", ImVec2(246, 60))) {
                        auto out = masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).fullPath;
                        DLOG("Saving to %s\n", out.c_str());
                        MasterVolumeDSDImVec2ToTable();
                        if (masterVolumeDSD.ToFile(out) == 0) {
                            statusStringMasterVolumeDSD = "Saved";
                        } else {
                            statusStringMasterVolumeDSD = "Failed";
                        }
                    }

                    if (masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).name.rfind(systemMark, 0) == 0) {
                        ImGui::EndDisabled();
                    }

                    if (ImGui::Button("Apply", ImVec2(246, 60))) {
                        DLOG("Applying to %s\n", Dac::volumeTableDSDOutPath.c_str());
                        MasterVolumeDSDImVec2ToTable();
                        if (masterVolumeDSD.Apply(Dac::volumeTableDSDOutPath) == 0) {
                            statusStringMasterVolumeDSD = "Applied";
                        } else {
                            statusStringMasterVolumeDSD = "Failed";
                        }
                    }

                    if (ImGui::Button("Copy val", ImVec2(121, 60))) {
                        memcpy(
                            masterVolumeDSDValueBuffer, masterVolumeDSDValues[MasterVolumeDSDTableType], sizeof(masterVolumeDSDValueBuffer)
                        );
                        statusStringMasterVolumeDSD = "Copied";
                    }
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));

                    ImGui::SameLine();
                    if (ImGui::Button("Paste val", ImVec2(121, 60))) {
                        memcpy(
                            masterVolumeDSDValues[MasterVolumeDSDTableType], masterVolumeDSDValueBuffer, sizeof(masterVolumeDSDValueBuffer)
                        );
                        statusStringMasterVolumeDSD = "Pasted";
                    }
                    ImGui::PopStyleVar();
                }

                ImGui::EndTable();
            }

            ImGui::EndTabItem();
        }
    }

    void TabToneControl() {
        if (ImGui::BeginTabItem(Dac::TableTypeToString.at(TABLE_ID_TONE_CONTROL).c_str())) {
            curveEditorTarget = (float *)toneControlValues[toneControlTableType];
            curveYLimit = (1 << 8) - 1; // 255
            curveElementCount = CODEC_RAM_SIZE;

            if (ImGui::BeginTable("##mvtable", 3, ImGuiTableFlags_None)) {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, 384);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("File:");

                ImGui::TableNextColumn();
                ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
                ImGui::PushItemWidth(-FLT_MIN);
                if (ImGui::BeginCombo(
                        "##toneControlFileCombo", toneControlFiles.at(toneControlFileSelected).name.c_str(), ImGuiComboFlags_HeightRegular
                    )) {
                    for (int idx = 0; idx < toneControlFiles.size(); idx++) {
                        auto entry = toneControlFiles.at(idx);
                        if (ImGui::Selectable(entry.name.c_str(), false)) {
                            DLOG("selected tone control file %s\n", entry.fullPath.c_str());
                            toneControlFileSelected = idx;
                            toneControl.Reset();
                            memset(&toneControlValues, 0, sizeof(toneControlValues));
                            if (toneControl.FromFile(entry.fullPath) != 0) {
                                DLOG("failed to load tone control table file %s\n", entry.fullPath.c_str());
                                statusStringToneControl = "Failed";
                            } else {
                                ToneControlToImVec2();
                                statusStringToneControl = "Loaded";
                            }
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::PopItemWidth();
                ImGui::PopStyleVar(2);

                ImGui::TableNextColumn();
                if (!statusStringToneControl.empty()) {
                    ImGui::Text(statusStringToneControl.c_str());
                    ImGui::SameLine(20);
                    if (ImGui::InvisibleButton("##statusToneControl", ImVec2(246, 30))) {
                        statusStringToneControl = "";
                    }
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Table type:");

                ImGui::TableNextColumn();
                ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
                const char *preview;
                preview = Dac::ToneControlTableTypeToString.at(toneControlTableType).c_str();
                ImGui::PushItemWidth(-FLT_MIN);
                if (ImGui::BeginCombo("##toneControlTypeCombo", preview, ImGuiComboFlags_HeightRegular)) {
                    for (const auto &entry : Dac::ToneControlTableTypeToString) {
                        if (ImGui::Selectable(entry.second.c_str(), false)) {
                            toneControlTableType = entry.first;
                            DLOG("selected tone control type %s\n", entry.second.c_str());
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopItemWidth();
                ImGui::PopStyleVar(2);

                ImGui::EndTable();
            }

            if (ImGui::BeginTable("##ttcontent", 2, ImGuiTableFlags_None)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (toneControlValues[0][0].x == -FLT_MIN) {
                    ImGui::NewLine();
                    if (ImGui::Button("Load", ImVec2(512, 150))) {
                        if (toneControl.FromFile(toneControlFiles.at(toneControlFileSelected).fullPath) != 0) {
                            DLOG(
                                "failed to load tone control table file %s\n", toneControlFiles.at(toneControlFileSelected).fullPath.c_str()
                            );
                            statusStringToneControl = "Failed";
                        } else {
                            ToneControlToImVec2();
                            statusStringToneControl = "Loaded";
                        }
                    }
                } else {
                    CurveEditorSmall(nullptr, ImVec2(512, 305));

                    ImGui::TableNextColumn();

                    // not efficient at all
                    if (toneControlFiles.at(toneControlFileSelected).name.rfind(systemMark, 0) == 0) {
                        if (ImGui::Button("Copy and edit", ImVec2(246, 60))) {
                            auto outDir = soundSettingsPathUser + "tone_control/";
                            CopyTableEntry(&toneControl, &toneControlFiles, &toneControlFileSelected, outDir);
                            displayTab = TabCurveEditor;
                        }
                    } else {
                        if (ImGui::Button("Edit", ImVec2(246, 60))) {
                            displayTab = TabCurveEditor;
                        }
                    }

                    if (toneControlFiles.at(toneControlFileSelected).name.rfind(systemMark, 0) == 0) {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::Button("Save", ImVec2(246, 60))) {
                        auto out = toneControlFiles.at(toneControlFileSelected).fullPath;
                        DLOG("Saving to %s\n", out.c_str());
                        ToneControlImVec2ToTable();
                        if (toneControl.ToFile(out) == 0) {
                            statusStringToneControl = "Saved";
                        } else {
                            statusStringToneControl = "Failed";
                        }
                    }

                    if (toneControlFiles.at(toneControlFileSelected).name.rfind(systemMark, 0) == 0) {
                        ImGui::EndDisabled();
                    }

                    if (ImGui::Button("Apply", ImVec2(246, 60))) {
                        DLOG("Applying to %s\n", Dac::toneControlOutPath.c_str());
                        ToneControlImVec2ToTable();
                        if (toneControl.Apply(Dac::toneControlOutPath) == 0) {
                            statusStringToneControl = "Applied";
                        } else {
                            statusStringToneControl = "Failed";
                        }
                    }

                    if (ImGui::Button("Copy val", ImVec2(121, 60))) {
                        memcpy(toneControlValueBuffer, toneControlValues[toneControlTableType], sizeof(toneControlValueBuffer));
                        statusStringToneControl = "Copied";
                    }
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));

                    ImGui::SameLine();
                    if (ImGui::Button("Paste val", ImVec2(121, 60))) {
                        memcpy(toneControlValues[toneControlTableType], toneControlValueBuffer, sizeof(toneControlValueBuffer));
                        statusStringToneControl = "Pasted";
                    }
                    ImGui::PopStyleVar();
                }

                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }
    }

    void TabSoundStatus() {
        if (ImGui::BeginTabItem("Status")) {
            ImGui::NewLine();
            if (ImGui::BeginTable("##soundstatustable", 2, ImGuiTableFlags_None)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ImGui::Button("Refresh", ImVec2(240, 60))) {
                    statusMasterVTFile = Dac::getStatus(&masterVolumeFiles, Dac::volumeTableOutPath);
                    statusMasterVTDSDFile = Dac::getStatus(&masterVolumeDSDFiles, Dac::volumeTableDSDOutPath);
                    statusToneControlFile = Dac::getStatus(&toneControlFiles, Dac::toneControlOutPath);
                    deviceProduct = GetProduct();
                    deviceModelID = GetModelID();
                    deviceRegionID = GetRegionID();
                    deviceRegionStr = GetRegionIDStr();
                    deviceAudioInUse = AudioDeviceInUse();
                    minCpuFreq = ReadFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
                    rstrip(&minCpuFreq, '\n');
                    curCpuFreq = ReadFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
                    rstrip(&curCpuFreq, '\n');
                    freqStr = minCpuFreq + " (" + curCpuFreq + ") Hz";
                    deviceAudioSampleRate = CardSampleRate();

                    refreshStatus = "Refreshed";
                }

                ImGui::TableNextColumn();

                ImGui::Text(refreshStatus.c_str());
                ImGui::SameLine(20);
                if (ImGui::InvisibleButton("##refreshStatus", ImVec2(246, 30))) {
                    refreshStatus = "";
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Model ID");
                ImGui::TableNextColumn();
                ImGui::Text(deviceModelID.c_str());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Region ID");
                ImGui::TableNextColumn();
                if (!deviceRegionID.empty() && !deviceRegionStr.empty()) {
                    ImGui::Text("%s (%s)", deviceRegionID.c_str(), deviceRegionStr.c_str());
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Device product");
                ImGui::TableNextColumn();
                ImGui::Text(deviceProduct.c_str());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Master volume table");
                ImGui::TableNextColumn();
                ImGui::Text(statusMasterVTFile.c_str());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Master volume table (DSD)");
                ImGui::TableNextColumn();
                ImGui::Text(statusMasterVTDSDFile.c_str());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Tone control");
                ImGui::TableNextColumn();
                ImGui::Text(statusToneControlFile.c_str());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Audio card");
                ImGui::TableNextColumn();
                ImGui::Text(deviceAudioInUse.first.c_str());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Audio device");
                ImGui::TableNextColumn();
                ImGui::Text(deviceAudioInUse.second.c_str());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Sample rate");
                ImGui::TableNextColumn();
                ImGui::Text(deviceAudioSampleRate.c_str());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("CPU frequency (min/cur)");
                ImGui::TableNextColumn();
                ImGui::Text(freqStr.c_str(), ImVec2(200, 100));

                ImGui::EndTable();
            }

            ImGui::EndTabItem();
        }
    }

    void SoundSettingsTab() {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("SoundSettingsTabBar", tab_bar_flags)) {
            TabMasterVolume();
            TabMasterDSDVolume();
            TabToneControl();
            TabSoundStatus();
            ImGui::EndTabBar();
        }
    }

    void CurveEditorSmall(const int *volume, ImVec2 size) {
        int newCount;
        int selected;
        int hovered;

        if (size.x == 0 || size.y == 0) {
            size = ImVec2(512, 270);
        }

        if (ImGui::CurveEditor(
                "##curvaSmall",
                curveEditorTarget,
                curveElementCount,
                curveElementCount,
                size,
                (int)ImGui::CurveEditorFlags::NO_TANGENTS | (int)ImGui::CurveEditorFlags::SHOW_GRID | (int)ImGui::CurveEditorFlags::RESET,
                &newCount,
                &selected,
                &hovered,
                &zoomZero,
                0,
                false,
                curveYLimit,
                volume
            )) {
        }
    }

    void CurveEditor() {
        int newCount;
        int selected;
        int hovered;

        if (ImGui::Button("Zoom in")) {
            zoomDirection = 1;
        }

        ImGui::SameLine();
        if (ImGui::Button("Zoom out")) {
            zoomDirection = -1;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset zoom")) {
            zoomDirection = 2;
        }

        float offset = 15.0f;
        auto closeSize = ImGui::CalcTextSize("Back").x + ImGui::GetStyle().FramePadding.x * 2.f;
        auto resetSize = ImGui::CalcTextSize("Reset").x + ImGui::GetStyle().FramePadding.x * 2.f;
        ImGui::SameLine(800.0f - closeSize - offset);
        if (ImGui::Button("Back")) {
            displayTab = TabSoundSettings;
        }

        ImGui::SameLine(800.0f - resetSize - closeSize - offset * 2);
        if (ImGui::Button("Reset")) {
            MasterVolumeTableToImVec2();
        }

        if (ImGui::CurveEditor(
                "##curva",
                curveEditorTarget,
                curveElementCount,
                curveElementCount,
                ImVec2(780, 430),
                (int)ImGui::CurveEditorFlags::NO_TANGENTS | (int)ImGui::CurveEditorFlags::SHOW_GRID,
                &newCount,
                &selected,
                &hovered,
                &zoomDirection,
                10,
                true,
                curveYLimit
            )) {
        }

        zoomDirection = 0;
    }

    void Draw() {
        if (displaySettings == 0) {
            switch (activeSkinVariant) {
            case WINAMP:
                FontRegular->FontSize = Winamp::fontSizeTTF;
                winamp.Draw();
                break;
            case CASSETTE:
                FontRegular->FontSize = Cassette::fontSizeTTF;
                cassette.Draw();
                break;
            case DIGITAL_CLOCK:
                digitalClock.Draw();
                break;
            default:
                FontRegular->FontSize = Winamp::fontSizeTTF;
                winamp.Draw();
                break;
            }
        } else {
            DrawSettings();
        }

        if (config->debug) {
            DisplayKeys();
        }
    }

    void KeyHandler() {
        if (*hold_toggled) {
            *hold_toggled = false;

            HgrmToggleAction action = Show;
            if (*hold_value == 1) {
                action = Hide;
            }

            DLOG("hold toggled, value %d, render %d, action %d\n", *hold_value, *render, action);
            connector->ToggleHgrm(action, render);

            if (action == Hide) {
                RefreshWinampSkinList();
                RefreshCassetteTapeReelLists();
                RefreshMasterVolumeTableFiles();
                RefreshMasterVolumeTableDSDFiles();
                RefreshToneControlFiles();
                PreprocessTableFilenames();
            }

            connector->soundSettings.Update();
            connector->soundSettingsFw.Update();
            eqStatus = "Refreshed";

            ImGui::GetIO().AddMousePosEvent(0.0f, 0.0f);
        }

        if (*power_pressed) {
            *power_pressed = false;

            if (*render) {
                *render = false;
            }
        }

        if (ImGui::IsKeyReleased(ImGuiKey_Prev)) {
            if (config->misc.swapTrackButtons) {
                connector->NextTrack();
            } else {
                connector->PrevTrack();
            }
            return;
        }

        if (ImGui::IsKeyReleased(ImGuiKey_Next)) {
            if (config->misc.swapTrackButtons) {
                connector->PrevTrack();
            } else {
                connector->NextTrack();
            }
            return;
        }

        if (ImGui::IsKeyReleased(ImGuiKey_Play)) {
            connector->Play();
            return;
        }

        if (ImGui::IsKeyReleased(ImGuiKey_VolumeDown)) {
            connector->SetVolume(-1, true);
            return;
        }

        if (ImGui::IsKeyReleased(ImGuiKey_VolumeUp)) {
            connector->SetVolume(1, true);
            //            connector->TestCommand();
            return;
        }
    }

    void GetLogsDirSize() {
        std::vector<directoryEntry> files{};
        listdir("/contents/wampy/log/", &files, "*");

        int res = 0;
        struct stat sb {};
        for (const auto &e : files) {
            if (stat(e.fullPath.c_str(), &sb) != 0) {
                DLOG("cannot stat %s\n", e.fullPath.c_str());
                continue;
            }

            res += (int)sb.st_size;
        }

        res = res / 1024 / 1024;

        logCleanupButtonLabel = "Remove Wampy logs (" + std::to_string(res) + " MB)";
    }

    void SetActiveEqFilter(const std::string &v) {
        DLOG("setting active filter tab: %s\n", v.c_str());
        if (v == "donate") {
            eActiveFilterTab = ActiveFilterTab_Donate;
            activeFilter = v;
            return;
        }

        if (v == "misc") {
            eActiveFilterTab = ActiveFilterTab_Misc;
            activeFilter = v;
            return;
        }

        if (connector->soundSettingsFw.filters.find(v) == connector->soundSettingsFw.filters.end()) {
            activeFilter = connector->soundSettingsFw.filterInvalid;
            eActiveFilterTab = ActiveFilterTab_Invalid;
            return;
        }

        activeFilter = v;

        if (filterToTab.find(activeFilter) == filterToTab.end()) {
            eActiveFilterTab = ActiveFilterTab_Invalid;
            activeFilter = connector->soundSettingsFw.filterInvalid;
            return;
        }

        eActiveFilterTab = filterToTab[activeFilter];
    }

    void TabEQ_Donate() const {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 425 / 2 - qrSide / 2 - 28);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 580 / 2 - qrSide / 2);

        ImGui::Image((void *)(intptr_t)qrDonateTexture, ImVec2(qrSide, qrSide));

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 580 / 2 - ImGui::CalcTextSize("boosty.to/unknown321/donate").x / 2);
        ImGui::TextColored(GOLD_DONATE, "boosty.to/unknown321/donate");
    }

    void TabEQ_Misc() {
        static ImGuiTableFlags flags = ImGuiTableFlags_NoBordersInBody;
        if (ImGui::BeginTable("##sswMisc", 2, flags)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 65.0);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 35.0);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Direct Source");

            ImGui::TableNextColumn();
            if (connector->soundSettingsFw.s->directSourceOn) {
                if (ImGui::Button("Disable", ImVec2(186, 60))) {
                    connector->soundSettingsFw.SetDirectSource(false);
                    nanosleep(&ssfwUpdateDelay, nullptr);
                    connector->soundSettingsFw.Update();
                }
            } else {
                if (ImGui::Button("Enable", ImVec2(186, 60))) {
                    connector->soundSettingsFw.SetDirectSource(true);
                    nanosleep(&ssfwUpdateDelay, nullptr);
                    connector->soundSettingsFw.Update();
                }
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("ClearAudio+");

            ImGui::TableNextColumn();
            if (isWalkmanOne) {
                ImGui::BeginDisabled();
            }
            if (connector->soundSettingsFw.s->clearAudioPlusOn) {
                ImGui::PushID(30 + 1);
                if (ImGui::Button("Disable", ImVec2(186, 60))) {
                    connector->soundSettingsFw.SetClearAudioPlus(false);
                    nanosleep(&ssfwUpdateDelay, nullptr);
                    connector->soundSettingsFw.Update();
                }
                ImGui::PopID();
            } else {
                ImGui::PushID(30 + 1);
                if (ImGui::Button("Enable", ImVec2(186, 60))) {
                    connector->soundSettingsFw.SetClearAudioPlus(true);
                    nanosleep(&ssfwUpdateDelay, nullptr);
                    connector->soundSettingsFw.Update();
                }
                ImGui::PopID();
            }

            if (isWalkmanOne) {
                ImGui::EndDisabled();
            }

            ImGui::EndTable();
        }
    }

    void TabEq_EnableDisableFilter() {
        if (connector->soundSettingsFw.filters.at(activeFilter)) {
            if (ImGui::Button("Disable", ImVec2(186, 60))) {
                connector->soundSettingsFw.SetFilter(activeFilter, false);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();
            }
        } else {
            if (ImGui::Button("Enable", ImVec2(186, 60))) {
                connector->soundSettingsFw.SetFilter(activeFilter, true);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();
            }
        }
    }

    void TabEQ_Vinylizer() {
        TabEq_EnableDisableFilter();

        ImGui::SeparatorText("Vinylizer type:");
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        ImGui::PushItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo(
                "##vinylizerValues", vinylTypeToString.at(connector->soundSettingsFw.vinylizerValue).c_str(), ImGuiComboFlags_HeightRegular
            )) {
            for (const auto &entry : vinylTypeToString) {
                if (entry.second == "?") {
                    continue;
                }
                if (ImGui::Selectable(entry.second.c_str(), false)) {
                    connector->soundSettingsFw.SetVinylizerType(entry.first);
                    nanosleep(&ssfwUpdateDelay, nullptr);
                    connector->soundSettingsFw.Update();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleVar(2);
    }

    void TabEQ_DCPhase() {
        TabEq_EnableDisableFilter();
        ImGui::SeparatorText("Type:");
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        ImGui::PushItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo(
                "##DcPhaseValues", dcFilterToString.at(connector->soundSettingsFw.dcValue).c_str(), ImGuiComboFlags_HeightRegular
            )) {
            for (const auto &entry : dcFilterToString) {
                if (entry.second == "?") {
                    continue;
                }
                if (ImGui::Selectable(entry.second.c_str(), false)) {
                    connector->soundSettingsFw.SetDcFilterType(entry.first);
                    nanosleep(&ssfwUpdateDelay, nullptr);
                    connector->soundSettingsFw.Update();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleVar(2);
    }

    void TabEQ_Vpt() {
        TabEq_EnableDisableFilter();
        ImGui::SeparatorText("VPT type:");
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        ImGui::PushItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo(
                "##vptValues", vptA50SmallToString.at(connector->soundSettingsFw.vptValue).c_str(), ImGuiComboFlags_HeightRegular
            )) {
            for (const auto &entry : vptA50SmallToString) {
                if (entry.second == "?") {
                    continue;
                }
                if (ImGui::Selectable(entry.second.c_str(), false)) {
                    connector->soundSettingsFw.SetVptMode(entry.first);
                    nanosleep(&ssfwUpdateDelay, nullptr);
                    connector->soundSettingsFw.Update();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleVar(2);
    }

    void TabEQ_DynamicNormalizer() {
        TabEq_EnableDisableFilter();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 580 / 2 - ImGui::CalcTextSize("No options").x / 2);
        ImGui::SetCursorPosY(
            ImGui::GetCursorPosY() + 425 / 2 - ImGui::CalcTextSize("No options").y / 2 - ImGui::GetTextLineHeightWithSpacing() - 60
        );
        ImGui::Text("No options");
    }

    void TabEQ_Eq6Band() {
        TabEq_EnableDisableFilter();

        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
        ImGui::SeparatorText("Preset:");
        ImGui::PushItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo(
                "##eq6preset", eq6PresetToString.at(connector->soundSettingsFw.eq6Value).c_str(), ImGuiComboFlags_HeightRegular
            )) {
            for (const auto &entry : eq6PresetToString) {
                if (ImGui::Selectable(entry.second.c_str(), false)) {
                    connector->soundSettingsFw.SetEq6Preset(entry.first);
                    nanosleep(&ssfwUpdateDelay, nullptr);
                    connector->soundSettingsFw.Update();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleVar(2);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(18, 14));
        static ImGuiTableFlags flags = ImGuiTableFlags_NoBordersInBody;
        if (ImGui::BeginTable("##ssweq6", 6, flags)) {
            ImGui::TableNextRow(0, ImGui::TableGetHeaderRowHeight() + 4);
            for (int i = 0; i < 6; i++) {
                ImGui::TableNextColumn();
                ImGui::Text("%d", connector->soundSettingsFw.eq6bands.at(i).second);
            }
            ImGui::TableNextRow();
            for (int i = 0; i < 6; i++) {
                ImGui::TableNextColumn();
                ImGui::PushID(20 + i);
                if (ImGui::VSliderInt("##int", ImVec2(40, 196), &connector->soundSettingsFw.eq6bands.at(i).second, -10, 10, "")) {
                    connector->soundSettingsFw.SetEq6Band(i, connector->soundSettingsFw.eq6bands.at(i).second);
                }
                ImGui::PopID();
            }
            ImGui::TableNextRow();
            for (int i = 0; i < 6; i++) {
                ImGui::TableNextColumn();
                ImGui::Text(connector->soundSettingsFw.eq6bands.at(i).first.c_str());
            }

            ImGui::EndTable();
        }

        ImGui::PopStyleVar(2);
    }

    void TabEQ_Eq10Band() {
        TabEq_EnableDisableFilter();

        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(18, 14));

        static ImGuiTableFlags flags = ImGuiTableFlags_NoBordersInBody;
        if (ImGui::BeginTable("##ssweq10", 10, flags)) {
            ImGui::TableNextRow(0, ImGui::TableGetHeaderRowHeight() + 4);
            for (int i = 0; i < 10; i++) {
                ImGui::TableNextColumn();
                ImGui::Text("%.1f", float(connector->soundSettingsFw.eq10bands.at(i).second) / 2);
            }
            ImGui::TableNextRow();
            for (int i = 0; i < 10; i++) {
                ImGui::TableNextColumn();
                ImGui::PushID(i);
                if (ImGui::VSliderInt("##int", ImVec2(40, 278), &connector->soundSettingsFw.eq10bands.at(i).second, -20, 20, "")) {
                    connector->soundSettingsFw.SetEq10Band(i, connector->soundSettingsFw.eq10bands.at(i).second);
                }
                ImGui::PopID();
            }
            ImGui::TableNextRow();
            for (int i = 0; i < 10; i++) {
                ImGui::TableNextColumn();
                ImGui::Text(connector->soundSettingsFw.eq10bands.at(i).first.c_str());
            }

            ImGui::EndTable();
        }

        ImGui::PopStyleVar(2);
    }

    void TabEQ_EqTone() {
        TabEq_EnableDisableFilter();

        ImGui::SeparatorText("Tone:");

        static ImGuiTableFlags flags = ImGuiTableFlags_NoBordersInBody;
        if (ImGui::BeginTable("##sswEqtone", 3, flags)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 20.0);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 70.0);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 10.0);
            for (int i = 0; i < 3; i++) {
                ImGui::TableNextRow(0, 40.0f);
                ImGui::TableNextColumn();
                ImGui::Text(connector->soundSettingsFw.eqtone.at(i).first.c_str());

                ImGui::TableNextColumn();
                ImGui::PushID(i);
                ImGui::PushItemWidth(-FLT_MIN);
                ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
                if (ImGui::SliderInt("##int", &connector->soundSettingsFw.eqtone.at(i).second, -20, 20, "")) {
                    connector->soundSettingsFw.SetEqTone(i, connector->soundSettingsFw.eqtone.at(i).second);
                }
                ImGui::PopStyleVar();
                ImGui::PopItemWidth();
                ImGui::PopID();
                ImGui::TableNextColumn();
                ImGui::Text("%.1f", float(connector->soundSettingsFw.eqtone.at(i).second) / 2);
            }

            ImGui::EndTable();
        }

        ImGui::SeparatorText("Tone frequency type:");
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

        if (ImGui::BeginTable("##sswEqtone2", 3, flags)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 20.0);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 70.0);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 10.0);

            for (int i = 0; i < 3; i++) {
                ImGui::TableNextRow(0, 40.0f);
                ImGui::TableNextColumn();
                ImGui::Text(connector->soundSettingsFw.eqtoneFreq.at(i).first.c_str());

                ImGui::TableNextColumn();
                ImGui::PushID(10 + i);
                ImGui::PushItemWidth(-FLT_MIN);
                ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
                if (ImGui::SliderInt("##int", &connector->soundSettingsFw.eqtoneFreq.at(i).second, 0, 5, "")) {
                    connector->soundSettingsFw.SetEqToneFreq(i, connector->soundSettingsFw.eqtoneFreq.at(i).second);
                }
                ImGui::PopStyleVar();
                ImGui::PopItemWidth();
                ImGui::PopID();
                ImGui::TableNextColumn();
                ImGui::Text("%d", connector->soundSettingsFw.eqtoneFreq.at(i).second);
            }

            ImGui::EndTable();
        }
    }

    void TabDac() {
        if (connector->status.State == PlayStateE::PLAYING) {
            ImGui::NewLine();
            ImGui::Text("Stop music first.");
            ImGui::NewLine();
            if (ImGui::Button("Stop music", ImVec2(186, 60))) {
                connector->Pause();
            }
            return;
        }

        ImGui::Text(llusbdacStatus.c_str());
        auto label = llusbdacLoaded ? "Disable" : "Enable";
        if (ImGui::Button(label, ImVec2(756, 350))) {
            if (llusbdacLoaded) {
                llusbdacLoaded = !DisableLLUSBDAC();
                llusbdacStatus = !llusbdacLoaded ? "Unloaded" : "Failure";
            } else {
                llusbdacLoaded = EnableLLUSBDAC();
                llusbdacStatus = llusbdacLoaded ? "Loaded" : "Failure";
            }
        }
    }

    void TabFM() {
        ImGui::NewLine();
        if (!radioAvailable) {
            ImGui::Text("FM radio is unavailable on this device");
            return;
        }

        if (connector->soundSettings.s->fmStatus.state == 2) {
            if (ImGui::Button("Disable", ImVec2(186, 60))) {
                RadioOff();
                connector->soundSettings.SetFM(0);
            }
        } else {
            if (ImGui::Button("Enable", ImVec2(186, 60))) {
                if (connector->status.State == PlayStateE::PLAYING) {
                    connector->Pause();
                }
                RadioOn();
                connector->soundSettings.SetFM(1);
            }
        }

        if (connector->soundSettings.s->fmStatus.state != 2) {
            return;
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(windowSize.x - ImGui::CalcTextSize("Stereo").x - 50);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
        if (ImGui::Checkbox("Stereo", &connector->soundSettings.s->fmStatus.stereo)) {
            connector->soundSettings.SetFMStereo(connector->soundSettings.s->fmStatus.stereo);
        }

        ImGui::PushItemWidth(windowSize.x - 15 * 2 - ImGui::GetStyle().FramePadding.x * 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(40.0f, 40.0f));
        if (ImGui::SliderInt("##fmfreq", &fmFreq, 76000, 108000, fmFreqFormat, ImGuiSliderFlags_NoInput)) {
            if (fmFreq % 100 > 0) {
                fmFreq = fmFreq - fmFreq % 100;
            }
            connector->soundSettings.SetFMFreq(fmFreq);
            sprintf(fmFreqFormat, "%.1fMHz", float(fmFreq) / 1000);
        }
        ImGui::PopStyleVar(2);
        ImGui::PopItemWidth();

        if (ImGui::Button("<<", ImVec2(80, 80))) {
            fmFreq -= 500;
            if (fmFreq < FM_FREQ_MIN) {
                fmFreq = FM_FREQ_MIN;
            }
            connector->soundSettings.SetFMFreq(fmFreq);
            sprintf(fmFreqFormat, "%.1fMHz", float(fmFreq) / 1000);
        }

        ImGui::SameLine();
        if (ImGui::Button("<", ImVec2(80, 80))) {
            fmFreq -= 100;
            if (fmFreq < FM_FREQ_MIN) {
                fmFreq = FM_FREQ_MIN;
            }
            connector->soundSettings.SetFMFreq(fmFreq);
            sprintf(fmFreqFormat, "%.1fMHz", float(fmFreq) / 1000);
        }

        ImGui::SameLine();
        ImGui::SetCursorPosX((windowSize.x / 2) - 100 - 1);
        if (ImGui::Button("Save", ImVec2(100, 80))) {
            if (std::find(config->fmPresets.begin(), config->fmPresets.end(), fmFreq) == config->fmPresets.end()) {
                config->fmPresets.emplace_back(fmFreq);
                std::sort(config->fmPresets.begin(), config->fmPresets.end());
                config->Save();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Remove", ImVec2(100, 80))) {
            auto pos = std::find(config->fmPresets.begin(), config->fmPresets.end(), fmFreq);
            if (pos != config->fmPresets.end()) {
                config->fmPresets.erase(pos);
                std::sort(config->fmPresets.begin(), config->fmPresets.end());
                config->Save();
            }
        }

        ImGui::SameLine();
        ImGui::SetCursorPosX(windowSize.x - 15 * 2 - 80 * 2 + ImGui::GetStyle().ItemSpacing.x - 1);
        if (ImGui::Button(">", ImVec2(80, 80))) {
            fmFreq += 100;
            if (fmFreq > FM_FREQ_MAX) {
                fmFreq = FM_FREQ_MAX;
            }
            connector->soundSettings.SetFMFreq(fmFreq);
            sprintf(fmFreqFormat, "%.1fMHz", float(fmFreq) / 1000);
        }

        ImGui::SameLine();
        if (ImGui::Button(">>", ImVec2(80, 80))) {
            fmFreq += 500;
            if (fmFreq > FM_FREQ_MAX) {
                fmFreq = FM_FREQ_MAX;
            }
            connector->soundSettings.SetFMFreq(fmFreq);
            sprintf(fmFreqFormat, "%.1fMHz", float(fmFreq) / 1000);
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        ImGui::BeginChild(
            "##fmpresets", ImVec2(windowSize.x - 15 * 2 - ImGui::GetStyle().FramePadding.x * 2.0f, 145), ImGuiChildFlags_None, window_flags
        );
        ImVec2 button_sz(88, 60);
        float window_visible_x2 = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
        for (int i = 0; i < config->fmPresets.size(); i++) {
            float last_button_x2 = ImGui::GetItemRectMax().x;
            float next_button_x2 =
                last_button_x2 + ImGui::GetStyle().ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
            if (i < config->fmPresets.size() && next_button_x2 < window_visible_x2)
                ImGui::SameLine();
            char label[20];
            sprintf(label, "%.1f", (float)(config->fmPresets.at(i)) / 1000);
            if (ImGui::Button(label, button_sz)) {
                fmFreq = config->fmPresets.at(i);
                connector->soundSettings.SetFMFreq(config->fmPresets.at(i));
                sprintf(fmFreqFormat, "%.1fMHz", float(fmFreq) / 1000);
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
    }

    void TabEQ() {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

        if (prevSong != connector->status.Filename) {
            connector->soundSettings.Update();
            connector->soundSettingsFw.Update();
            eqSongExists = SoundSettings::Exists(connector->status.Filename);
            eqSongDirExists = SoundSettings::ExistsDir(connector->status.Filename);
            prevSong = connector->status.Filename;
            eqStatus = "Refreshed";
        }

        {
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
            ImGui::BeginChild("ChildL", ImVec2(580, 425), ImGuiChildFlags_Border, window_flags);

            switch (eActiveFilterTab) {

            case ActiveFilterTab_Invalid:
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 580 / 2 - ImGui::CalcTextSize("Select filter").x / 2);
                ImGui::SetCursorPosY(
                    ImGui::GetCursorPosY() + 425 / 2 - ImGui::CalcTextSize("Select filter").y / 2 - ImGui::GetTextLineHeightWithSpacing()
                );
                ImGui::Text("Select filter");
                break;
            case ActiveFilterTab_DynamicNormalizer:
                TabEQ_DynamicNormalizer();
                break;
            case ActiveFilterTab_Eq6Band:
                TabEQ_Eq6Band();
                break;
            case ActiveFilterTab_Vpt:
                TabEQ_Vpt();
                break;
            case ActiveFilterTab_DCPhaseLinearizer:
                TabEQ_DCPhase();
                break;
            case ActiveFilterTab_Vinylizer:
                TabEQ_Vinylizer();
                break;
            case ActiveFilterTab_Eq10Band:
                TabEQ_Eq10Band();
                break;
            case ActiveFilterTab_EqTone:
                TabEQ_EqTone();
                break;
            case ActiveFilterTab_Donate:
                TabEQ_Donate();
                break;
            case ActiveFilterTab_Misc:
                TabEQ_Misc();
                break;
            }

            ImGui::EndChild();
        }

        ImGui::SameLine();

        {
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
            ImGui::BeginChild("ChildR", ImVec2(180, 425), ImGuiChildFlags_Border, window_flags);

            static ImGuiTableFlags flags = ImGuiTableFlags_NoBordersInBody;
            if (ImGui::BeginTable("##sswFilters", 2, flags)) {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 10.0f);
                ImGui::TableSetupColumn("Filter", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 120.0f);
                ImGui::TableHeadersRow();
                for (auto v : connector->soundSettingsFw.s->FilterStatus) {
                    if (v.name[0] == '\0') {
                        continue;
                    }
                    if (strcmp(v.name, "i2f") == 0) {
                        continue;
                    }
                    if (strcmp(v.name, "f2i") == 0) {
                        continue;
                    }
                    if (strcmp(v.name, "heq") == 0) {
                        continue;
                    }
                    if (strcmp(v.name, "alc") == 0) {
                        continue;
                    }
                    if (strcmp(v.name, "attn") == 0) {
                        continue;
                    }
                    if (strcmp(v.name, "clearphase") == 0) {
                        continue;
                    }
                    if (strlen(v.name) > 4) {
                        if (strncmp(v.name, "dsee", 4) == 0) {
                            continue;
                        }
                    }
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (v.is_proc) {
                        ImGui::Text("+");
                    }
                    ImGui::TableNextColumn();

                    if (strcmp(v.name, activeFilter.c_str()) == 0) {
                        ImGui::PushStyleColor(ImGuiCol_Button, PLEASANT_GREEN);
                        if (ImGui::Button(v.name, ImVec2(130, 38))) {
                            SetActiveEqFilter(v.name);
                        }
                        ImGui::PopStyleColor();
                    } else {
                        if (ImGui::Button(v.name, ImVec2(130, 38))) {
                            SetActiveEqFilter(v.name);
                        }
                    }
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TableNextColumn();
                if (strcmp("misc", activeFilter.c_str()) == 0) {
                    ImGui::PushStyleColor(ImGuiCol_Button, PLEASANT_GREEN);
                    if (ImGui::Button("Misc", ImVec2(130, 38))) {
                        SetActiveEqFilter("misc");
                    }
                    ImGui::PopStyleColor();
                } else {
                    if (ImGui::Button("Misc", ImVec2(130, 38))) {
                        SetActiveEqFilter("misc");
                    }
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TableNextColumn();
                ImGui::PushStyleColor(ImGuiCol_Button, GOLD_DONATE);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1.0f));
                if (ImGui::Button("Donate", ImVec2(130, 38))) {
                    SetActiveEqFilter("donate");
                }
                ImGui::PopStyleColor(2);

                ImGui::EndTable();
            }

            ImGui::EndChild();
        }
    }

    void TabEQOld() {
        ImGui::NewLine();

        if (!config->features.eqPerSong) {
            ImGui::Text("Feature disabled. Toggle it on 'Misc' tab or use button below.");
            ImGui::NewLine();
            if (ImGui::Button("Enable", ImVec2(186, 60))) {
                config->features.eqPerSong = true;
                config->Save();
            }
            return;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        ImGui::BeginChild("##textwrap", ImVec2(800 - 20 - 10, ImGui::GetTextLineHeight() * 2 + 10), ImGuiWindowFlags_NoResize);

        ImGui::PushTextWrapPos(800 - 20 - 40 - 20);
        ImGui::Text("File: %s\n", connector->status.Filename.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndChild();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);

        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                                       ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchProp;

        ImVec2 outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 7);

        if (ImGui::BeginTable("##eqtable", 2, flags, outer_size)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("ClearAudio+");
            ImGui::TableNextColumn();
            ImGui::Text(
                "%s (%s)",
                connector->soundSettings.s->status.clearAudioOn == 1 ? "On" : "Off",
                connector->soundSettings.s->status.clearAudioAvailable == 1 ? "available" : "unavailable"
            );

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Direct Source");
            ImGui::TableNextColumn();
            ImGui::Text(
                "%s (%s)",
                connector->soundSettings.s->status.directSourceOn == 1 ? "On" : "Off",
                connector->soundSettings.s->status.directSourceAvailable == 1 ? "available" : "unavailable"
            );

            if ((connector->soundSettings.s->status.clearAudioOn == 1 && connector->soundSettings.s->status.clearAudioAvailable) ||
                (connector->soundSettings.s->status.directSourceOn == 1 && connector->soundSettings.s->status.directSourceAvailable)) {
                ImGui::BeginDisabled(true);
            }
            //
            //            ImGui::TableNextRow();
            //            ImGui::TableNextColumn();
            //            ImGui::Text("EQ6");
            //            ImGui::TableNextColumn();
            //            ImGui::Text("%s", connector->soundSettings.s->eq6On == 1 ? "On" : "Off");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("EQ6 Preset");
            ImGui::TableNextColumn();
            ImGui::Text("%s", eq6PresetToString.at(connector->soundSettings.s->status.eq6Preset).c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("EQ6 Bands");
            ImGui::TableNextColumn();

            ImGui::Text(
                "%d, %d, %d, %d, %d, %d",
                connector->soundSettings.s->status.eq6Bands[0],
                connector->soundSettings.s->status.eq6Bands[1],
                connector->soundSettings.s->status.eq6Bands[2],
                connector->soundSettings.s->status.eq6Bands[3],
                connector->soundSettings.s->status.eq6Bands[4],
                connector->soundSettings.s->status.eq6Bands[5]
            );
            //
            //            ImGui::TableNextRow();
            //            ImGui::TableNextColumn();
            //            ImGui::Text("EQ10");
            //            ImGui::TableNextColumn();
            //            ImGui::Text("%s", connector->soundSettings.s->eq10On == 1 ? "On" : "Off");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("EQ10 Preset");
            ImGui::TableNextColumn();
            ImGui::Text("%d", connector->soundSettings.s->status.eq10Preset);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("EQ10 Bands");
            ImGui::TableNextColumn();
            ImGui::Text(
                "%.1f, %.1f, %.1f, %.1f, %.1f,\n%.1f, %.1f, %.1f, %.1f, %.1f",
                float(connector->soundSettings.s->status.eq10Bands[0]) / 2,
                float(connector->soundSettings.s->status.eq10Bands[1]) / 2,
                float(connector->soundSettings.s->status.eq10Bands[2]) / 2,
                float(connector->soundSettings.s->status.eq10Bands[3]) / 2,
                float(connector->soundSettings.s->status.eq10Bands[4]) / 2,
                float(connector->soundSettings.s->status.eq10Bands[5]) / 2,
                float(connector->soundSettings.s->status.eq10Bands[6]) / 2,
                float(connector->soundSettings.s->status.eq10Bands[7]) / 2,
                float(connector->soundSettings.s->status.eq10Bands[8]) / 2,
                float(connector->soundSettings.s->status.eq10Bands[9]) / 2
            );

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("DSEE HX");
            ImGui::TableNextColumn();
            ImGui::Text("%s", connector->soundSettings.s->status.dseeHXOn == 1 ? "On" : "Off");

            //            ImGui::TableNextRow();
            //            ImGui::TableNextColumn();
            //            ImGui::Text("DSEE");
            //            ImGui::TableNextColumn();
            //            ImGui::Text("%s", connector->soundSettings.s->dseeOn == 1 ? "On" : "Off");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("DSEE custom");
            ImGui::TableNextColumn();
            ImGui::Text("%s", connector->soundSettings.s->status.dseeCustOn == 1 ? "On" : "Off");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("DSEE custom mode");
            ImGui::TableNextColumn();
            ImGui::Text("%s", dseeModeToString.at(connector->soundSettings.s->status.dseeCustMode).c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("DC Phase");
            ImGui::TableNextColumn();
            ImGui::Text("%s", connector->soundSettings.s->status.dcLinearOn == 1 ? "On" : "Off");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("DC Phase Filter");
            ImGui::TableNextColumn();
            ImGui::Text("%s", dcFilterToString.at(connector->soundSettings.s->status.dcLinearFilter).c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("VPT");
            ImGui::TableNextColumn();
            ImGui::Text("%s", connector->soundSettings.s->status.vptOn == 1 ? "On" : "Off");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("VPT Mode");
            ImGui::TableNextColumn();
            ImGui::Text("%s", vptA50SmallToString.at(connector->soundSettings.s->status.vptMode).c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Tone Control or 10 band EQ?");
            ImGui::TableNextColumn();
            ImGui::Text("%s", connector->soundSettings.s->status.eqUse == 2 ? "10 band EQ" : "Tone Control");
            //
            //            ImGui::TableNextRow();
            //            ImGui::TableNextColumn();
            //            ImGui::Text("Tone Control");
            //            ImGui::TableNextColumn();
            //            ImGui::Text("%s", connector->soundSettings.s->toneControlOn == 1 ? "On" : "Off");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Tone Control Low");
            ImGui::TableNextColumn();
            ImGui::Text("%d", connector->soundSettings.s->status.toneControlLow / 2);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Tone Control Mid");
            ImGui::TableNextColumn();
            ImGui::Text("%d", connector->soundSettings.s->status.toneControlMid / 2);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Tone Control High");
            ImGui::TableNextColumn();
            ImGui::Text("%d", connector->soundSettings.s->status.toneControlHigh / 2);
            //
            //            ImGui::TableNextRow();
            //            ImGui::TableNextColumn();
            //            ImGui::Text("Tone Control LowFreq");
            //            ImGui::TableNextColumn();
            //            ImGui::Text("%d", connector->soundSettings.s->toneControlLowFreq);
            //
            //            ImGui::TableNextRow();
            //            ImGui::TableNextColumn();
            //            ImGui::Text("Tone Control MidFreq");
            //            ImGui::TableNextColumn();
            //            ImGui::Text("%d", connector->soundSettings.s->toneControlMidFreq);
            //
            //            ImGui::TableNextRow();
            //            ImGui::TableNextColumn();
            //            ImGui::Text("Tone Control HighFreq");
            //            ImGui::TableNextColumn();
            //            ImGui::Text("%d", connector->soundSettings.s->toneControlHighFreq);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Vinyl Processor");
            ImGui::TableNextColumn();
            ImGui::Text("%s", connector->soundSettings.s->status.vinylOn == 1 ? "On" : "Off");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Vinyl Type");
            ImGui::TableNextColumn();
            ImGui::Text("%s", vinylTypeToString.at(connector->soundSettings.s->status.vinylType).c_str());

            //            ImGui::TableNextRow();
            //            ImGui::TableNextColumn();
            //            ImGui::Text("Master Volume");
            //            ImGui::TableNextColumn();
            //            ImGui::Text("%d", connector->soundSettings.s->masterVolume);

            //            ImGui::TableNextRow();
            //            ImGui::TableNextColumn();
            //            ImGui::Text("Clear Phase");
            //            ImGui::TableNextColumn();
            //            ImGui::Text("%s", connector->soundSettings.s->clearPhaseOn == 1 ? "On" : "Off");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Dynamic Normalizer");
            ImGui::TableNextColumn();
            ImGui::Text("%s", connector->soundSettings.s->status.DNOn == 1 ? "On" : "Off");

            if ((connector->soundSettings.s->status.clearAudioOn == 1 && connector->soundSettings.s->status.clearAudioAvailable) ||
                (connector->soundSettings.s->status.directSourceOn == 1 && connector->soundSettings.s->status.directSourceAvailable)) {
                ImGui::EndDisabled();
            }

            ImGui::EndTable();
        }
        ImGui::PopStyleVar(2);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

        if (prevSong == connector->status.Filename) {
            ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_RED);
            if (eqSongExists) {
                if (ImGui::Button("Remove", ImVec2(186, 60))) {
                    if (SoundSettings::Remove(connector->status.Filename) == 0) {
                        eqStatus = "Removed";
                    } else {
                        eqStatus = "Remove failed";
                    }

                    eqSongExists = SoundSettings::Exists(connector->status.Filename);
                }
            } else {
                ImGui::BeginDisabled();
                ImGui::Button("Remove", ImVec2(186, 60));
                ImGui::EndDisabled();
            }

            ImGui::PopStyleColor();

            ImGui::SameLine();
        } else {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 186 + ImGui::GetStyle().ItemSpacing.x);
        }

        if (prevSong != connector->status.Filename) {
            connector->soundSettings.Update();
            connector->soundSettingsFw.Update();
            eqSongExists = SoundSettings::Exists(connector->status.Filename);
            eqSongDirExists = SoundSettings::ExistsDir(connector->status.Filename);
            prevSong = connector->status.Filename;
            eqStatus = "Refreshed";
        }

        ImGui::SameLine();
        if (ImGui::Button("Set as default", ImVec2(187, 60))) {
            if (connector->soundSettings.Save("default") == 0) {
                eqStatus = "Default EQ set";
            } else {
                eqStatus = "Set EQ as default failed";
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Save", ImVec2(186, 60))) {
            if (connector->soundSettings.Save(connector->status.Filename) == 0) {
                eqStatus = "Saved";
            } else {
                eqStatus = "Save failed";
            }
            eqSongExists = SoundSettings::Exists(connector->status.Filename);
        }

        ImGui::SameLine();
        if (ImGui::Button("Save dir", ImVec2(186, 60))) {
            if (connector->soundSettings.SaveDir(connector->status.Filename) == 0) {
                eqStatus = "Saved dir";
            } else {
                eqStatus = "Save dir failed";
            }
            eqSongDirExists = SoundSettings::ExistsDir(connector->status.Filename);
        }

        ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_RED);
        if (eqSongDirExists) {
            if (ImGui::Button("Remove dir", ImVec2(186, 60))) {
                if (SoundSettings::RemoveDir(connector->status.Filename) == 0) {
                    eqStatus = "Dir removed";
                } else {
                    eqStatus = "Dir remove failed";
                }
                eqSongDirExists = SoundSettings::ExistsDir(connector->status.Filename);
            }
        } else {
            ImGui::BeginDisabled();
            ImGui::Button("Remove dir", ImVec2(186, 60));
            ImGui::EndDisabled();
        }

        ImGui::PopStyleColor();

        ImGui::SameLine();
        auto curpos = ImGui::GetCursorPos();
        ImGui::SetCursorPosY(curpos.y + 8);
        ImGui::Text(eqStatus.c_str());
        ImGui::SetCursorPosY(curpos.y - 8);

        ImGui::SameLine();
        ImGui::SetCursorPos(curpos);
        if (ImGui::InvisibleButton("##removeEqStatus", ImVec2(186 * 3 + ImGui::GetStyle().ItemSpacing.x + 3, 60))) {
            eqStatus = "";
        }
    }

    void CalcWindowPos() const {
        assert(windowOffset);
        switch (config->windowOffset) {
        case EWindowOffset_UNKNOWN:
            break;
        case EWindowOffset_LEFT:
            windowOffset->x = 0;
            windowOffset->y = 0;
            break;
        case EWindowOffset_CENTER:
            windowOffset->x = (screenMode.x - windowSize.x) / 2;
            windowOffset->y = 0;
            break;
        case EWindowOffset_RIGHT:
            windowOffset->x = screenMode.x - windowSize.x;
            windowOffset->y = 0;
            break;
        }
    }
};

#endif // WAMPY_SKIN_H
