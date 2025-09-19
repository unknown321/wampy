#include "skin.h"
#include "implot_widgets.h"

void Skin::WithWinampSkinDir(const std::string &d) { winampSkinDirectories.push_back(d); }

void Skin::RefreshWinampSkinList() {
    DLOG("\n");
    assert(config);
    if (activeSkinVariant == WINAMP && activeSettingsTab == SkinOpts) {
        DLOG("not refreshing skin list on active winamp tab\n");
        return;
    }

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

void Skin::WithCassetteTapeDir(const std::string &d) { cassetteTapeDirectories.push_back(d); }

void Skin::WithCassetteReelDir(const std::string &d) { cassetteReelDirectories.push_back(d); }

void Skin::RefreshCassetteTapeReelLists() {
    DLOG("enter\n");
    assert(config);
    if (activeSkinVariant == CASSETTE && activeSettingsTab == SkinOpts) {
        DLOG("not refreshing skin list on active winamp tab\n");
        return;
    }

    std::vector<directoryEntry> fl{};
    reelListCassette.clear();
    for (const auto &d : cassetteReelDirectories) {
        listdirs(d.c_str(), &reelListCassette);
        for (auto it = reelListCassette.begin(); it != reelListCassette.end();) {
            listdir(it->fullPath.c_str(), &fl, ".jpg");
            listdir(it->fullPath.c_str(), &fl, ".pkm");
            if (fl.empty()) {
                DLOG("ignoring empty reel dir %s\n", it->fullPath.c_str());
                reelListCassette.erase(it);
            } else {
                ++it;
            }
            fl.clear();
        }
    }

    tapeListCassette.clear();
    std::vector<int> invalid;
    for (const auto &d : cassetteTapeDirectories) {
        listdirs(d.c_str(), &tapeListCassette);
        for (auto it = tapeListCassette.begin(); it != tapeListCassette.end();) {
            listdir(it->fullPath.c_str(), &fl, ".jpg");
            listdir(it->fullPath.c_str(), &fl, ".pkm");
            if (fl.empty()) {
                DLOG("ignoring empty tape dir %s\n", it->fullPath.c_str());
                tapeListCassette.erase(it);
            } else {
                ++it;
            }
            fl.clear();
        }
    }

    DLOG("exit\n");
}

void Skin::WithMasterVolumeTableDirs(const std::string &d) { masterVolumeTableDirectories.push_back(d); };

void Skin::RefreshMasterVolumeTableFiles() {
    DLOG("\n");
    masterVolumeFiles.clear();
    for (const auto &d : masterVolumeTableDirectories) {
        listdir(d.c_str(), &masterVolumeFiles, ".tbl");
    }

    if (masterVolumeFiles.empty()) {
        listdir("/system/usr/share/audio_dac/", &masterVolumeFiles, ".tbl");
    }
}

void Skin::WithMasterVolumeTableDSDDirs(const std::string &d) { masterVolumeTableDSDDirectories.push_back(d); };

void Skin::RefreshMasterVolumeTableDSDFiles() {
    DLOG("\n");
    masterVolumeDSDFiles.clear();
    for (const auto &d : masterVolumeTableDSDDirectories) {
        listdir(d.c_str(), &masterVolumeDSDFiles, ".tbl");
    }

    if (masterVolumeDSDFiles.empty()) {
        listdir("/system/usr/share/audio_dac/", &masterVolumeDSDFiles, ".tbl");
    }
}

void Skin::WithToneControlTableDirs(const std::string &d) { toneControlTableDirectories.push_back(d); };

void Skin::RefreshToneControlFiles() {
    DLOG("\n");
    toneControlFiles.clear();
    for (const auto &d : toneControlTableDirectories) {
        listdir(d.c_str(), &toneControlFiles, ".tbl");
    }

    if (toneControlFiles.empty()) {
        listdir("/system/usr/share/audio_dac/", &toneControlFiles, ".tbl");
    }
}

void Skin::ReloadFont() {
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

void Skin::ReadQR() {
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

void Skin::Load() {
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

void Skin::LoadUpdatedSkin() {
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

void Skin::MasterVolumeTableToImVec2() {
    for (int index = 0; index < 2; index++) {
        for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX + 1; tableID++) {
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

void Skin::MasterVolumeDSDTableToImVec2() {
    for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX + 1; tableID++) {
        for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
            auto val = masterVolumeDSD.v[tableID][i];
            masterVolumeDSDValues[tableID][i] = ImVec2((float)i, (float)val);
        }
    }
}

void Skin::ToneControlToImVec2() {
    for (int tableID = 0; tableID < TONE_CONTROL_TABLE_MAX + 1; tableID++) {
        for (int i = 0; i < CODEC_RAM_SIZE; i++) {
            auto val = toneControl.v[tableID][i];
            toneControlValues[tableID][i] = ImVec2((float)i, (float)val);
        }
    }
}

void Skin::MasterVolumeImVec2ToTable() {
    for (int index = 0; index < 2; index++) {
        for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX + 1; tableID++) {
            for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
                for (int valType = 1; valType < MASTER_VOLUME_VALUE_MAX; valType++) {
                    auto val = masterVolumeValues[index][tableID][valType][i].y;
                    masterVolume.SetValue(index, tableID, i, (MASTER_VOLUME_VALUE)valType, uint(val));
                }
            }
        }
    }
}

void Skin::MasterVolumeDSDImVec2ToTable() {
    for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX + 1; tableID++) {
        for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
            for (int valType = 1; valType < MASTER_VOLUME_VALUE_MAX + 1; valType++) {
                auto val = masterVolumeDSDValues[tableID][i];
                masterVolumeDSD.v[tableID][i] = (int)val.y;
            }
        }
    }
}

void Skin::ToneControlImVec2ToTable() {
    for (int tableID = 0; tableID < TONE_CONTROL_TABLE_MAX + 1; tableID++) {
        for (int i = 0; i < CODEC_RAM_SIZE; i++) {
            auto val = toneControlValues[tableID][i];
            toneControl.v[tableID][i] = (int)val.y;
        }
    }
}

void Skin::ToggleAudioAnalyzerOn() const {
    DLOG("enter\n");
    if (activeSkinVariant != WINAMP) {
        return;
    }

    connector->soundSettings.SetAnalyzer(1);
    if (config->winamp.visualizerWinampBands) {
        connector->soundSettings.SetAnalyzerBandsWinamp();
    } else {
        connector->soundSettings.SetAnalyzerBandsOrig();
    }

    for (auto &v : connector->soundSettings.peaks) {
        v = 0;
    }

    DLOG("exit\n");
}

void Skin::ToggleAudioAnalyzerOff() const {
    DLOG("enter\n");

    if (activeSkinVariant != WINAMP) {
        return;
    }

    if (config->winamp.visualizerEnable) {
        connector->soundSettings.SetAnalyzer(0);
    }

    DLOG("exit\n");
}

void Skin::ToggleDrawSettings(void *skin, void *) {
    auto s = (Skin *)skin;
    assert(s);

    if (s->displaySettings == 0) {
        s->displaySettings = 1;
        s->ToggleAudioAnalyzerOff();
        s->RefreshWinampSkinList();
        s->RefreshCassetteTapeReelLists();
    } else {
        s->displaySettings = 0;
        s->ToggleAudioAnalyzerOn();
    }
}

void Skin::ToggleDrawEQTab(void *skin, void *) {
    auto s = (Skin *)skin;
    assert(s);
    if (s->displaySettings == 0) {
        s->ToggleAudioAnalyzerOff();
        s->displaySettings = 1;
        s->displayTab = SettingsTab::TabEQ;
    } else {
        s->ToggleAudioAnalyzerOn();
        s->displaySettings = 0;
    }
}

void Skin::RandomizeTape(void *skin, void *) {
    auto s = (Skin *)skin;
    assert(s);
    if (s->cassette.config->randomize) {
        s->cassette.SelectTape();
    }
}

void Skin::SaveConfig(void *skin) {
    auto s = (Skin *)skin;
    assert(s);
    s->config->Save();
}

void Skin::Header() {
    float offset = 15.0f;
    ImGui::Indent(offset);

    if (ImGui::Button("  W1  ")) {
        loadStatusStr = "";
        if (connector->storagePresent) {
            ParseSettings(&walkmanOneOptions);
        }
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
    if (config->showFmInSettings) {
        if (ImGui::Button(" FM ")) {
            loadStatusStr = "";
            displayTab = SettingsTab::TabFM;
            fmFreq = connector->soundSettings.s->fmStatus.freq;
            sprintf(fmFreqFormat, "%.1fMHz", float(connector->soundSettings.s->fmStatus.freq) / 1000);
        }
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

void Skin::Misc() {
    ImGui::NewLine();
    if (!connector->storagePresent) {
        ImGui::Text("Disable USB mass storage mode");
        return;
    }
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
        if (ImGui::Button("Set default volume tables")) {
            config->volumeTables.ToneControl = "";
            config->volumeTables.MasterVolumeTable = "";
            config->volumeTables.MasterVolumeTableDSD = "";
            config->Save();
            resetVolTablesStatus = "OK!";

            std::vector<std::vector<directoryEntry> *> tables = {&masterVolumeFiles, &masterVolumeDSDFiles, &toneControlFiles};
            for (auto &table : tables) {
                for (int i = 0; i < table->size(); i++) {
                    auto entry = table->at(i);
                    if (entry.name.rfind(defaultMark, 0) == 0) {
                        table->at(i).name.erase(0, defaultMark.length());
                    }
                }
            }
        }

        ImGui::TableNextColumn();
        ImGui::Text(resetVolTablesStatus.c_str());
        ImGui::SameLine(20);
        if (ImGui::InvisibleButton("##resetVolTablesStatus", ImVec2(246, 30))) {
            resetVolTablesStatus = "";
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
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Lock when screen is off", &config->disableKeysWhenPowerOff)) {
            config->Save();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Control filters", &config->controlFilters)) {
            for (auto f : connector->soundSettingsFw.s->FilterStatus) {
                SetConfigFilter(f.name, f.is_proc);
            }
            config->Save();
        }

        ImGui::TableNextColumn();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Debug", &config->debug)) {
            winamp.debug = config->debug;
            cassette.debug = config->debug;
            config->Save();
        }
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Show FM tab", &config->showFmInSettings)) {
            config->Save();
        }

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

void Skin::Website() const {
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

void Skin::WalkmanOne() {
    if (!connector->storagePresent) {
        ImGui::NewLine();
        ImGui::Text("Disable USB mass storage mode");
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 15.0f));
    ImGui::SeparatorText("Walkman One settings");
    ImGui::PopStyleVar();

    if (!walkmanOneOptions.configFound) {
        ImGui::Text("config file not found\n");
        return;
    }

    static ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchSame;

    ImVec2 outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 3);

    if (ImGui::BeginTable("walkmanOneTable", 2, flags, outer_size)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Interface color");

        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        ImGui::PushItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##walkmanOneColor", W1::colorByValueWalkmanOne.at(walkmanOneOptions.color).c_str(), ImGuiComboFlags_HeightRegular)) {
            for (const auto &entry : W1::colorByNameWalkmanOne) {
                if (ImGui::Selectable(entry.first.c_str(), false)) {
                    walkmanOneOptions.color = entry.second;
                    DLOG("selected color %s\n", W1::colorByValueWalkmanOne.at(walkmanOneOptions.color).c_str());
                    walkmanOneOptions.Save();
                    needRestart = true;
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
        if (ImGui::BeginCombo("##walkmanOneSignature", W1::signatureByValueWalkmanOne.at(walkmanOneOptions.signature).c_str(), ImGuiComboFlags_HeightRegular)) {
            for (const auto &entry : W1::signatureByNameWalkmanOne) {
                if (ImGui::Selectable(entry.first.c_str(), false)) {
                    walkmanOneOptions.signature = entry.second;
                    DLOG("selected signature %s\n", W1::signatureByValueWalkmanOne.at(walkmanOneOptions.signature).c_str());
                    walkmanOneOptions.Save();
                    needRestart = true;
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
                    needRestart = true;
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
            needRestart = true;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Plus mode v2");
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("##plusv2", &walkmanOneOptions.plusModeVersionBOOL)) {
            DLOG("Set plus mode v2 to %d\n", walkmanOneOptions.plusModeVersionBOOL);
            walkmanOneOptions.Save();
            needRestart = true;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Plus mode by default");
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("##plusdefault", &walkmanOneOptions.plusModeByDefault)) {
            DLOG("Set plus mode default to %d\n", walkmanOneOptions.plusModeByDefault);
            walkmanOneOptions.Save();
            needRestart = true;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Lower gain mode");
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("##gainmode", &walkmanOneOptions.gainMode)) {
            DLOG("Set gain mode to %d\n", walkmanOneOptions.gainMode);
            walkmanOneOptions.Save();
            needRestart = true;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Different DAC init mode");
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("##dacinitmode", &walkmanOneOptions.dacInitializationMode)) {
            DLOG("Set dac init mode to %d\n", walkmanOneOptions.dacInitializationMode);
            walkmanOneOptions.Save();
            needRestart = true;
        }

        ImGui::EndTable();
    }

    ImGui::NewLine();
    if (needRestart) {
#ifndef DESKTOP
        if (walkmanOneOptions.tuningChanged) {
            if (ImGui::Button("Apply tuning and reboot", ImVec2(300, 60))) {
                walkmanOneOptions.Reboot();
            }
        } else {
            ImGui::Text("Reboot the device to apply changes");
        }
#endif
    }
}

void Skin::WalkmanOneTab() {
    if (isWalkmanOne) {
        WalkmanOne();
    } else {
        Wee1();
    }
}

void Skin::Wee1() {
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
        needRestart = true;
    }

    if (needRestart) {
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

void Skin::ReadLicense() {
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

void Skin::License() const {
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

void Skin::License3rd() const {
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

void Skin::DrawSkin() {
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

    if (connector->storagePresent) {
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

void Skin::Winamp() {
    if (!connector->storagePresent) {
        ImGui::Text("Disable USB mass storage mode");
        return;
    }
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

    static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp;

    ImVec2 outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 7);

    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    if (ImGui::BeginTable("winampConfigTable", 2, flags, outer_size)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Use bitmap font", &config->winamp.useBitmapFont)) {
            config->Save();
            if (activeSkinVariant == WINAMP) {
                winamp.Format(true);
            }
        }
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Enable visualizer", &config->winamp.visualizerEnable)) {
            config->Save();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Use bitmap font in playlist", &config->winamp.useBitmapFontInPlaylist)) {
            config->Save();
            if (activeSkinVariant == WINAMP) {
                winamp.Format(true);
            }
        }
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Visualizer Winamp mode", &config->winamp.visualizerWinampBands)) {
            config->Save();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Prefer time remaining", &config->winamp.preferTimeRemaining)) {
            config->Save();
        }
        ImGui::TableNextColumn();
        ImGui::Text("Visualizer sensitivity");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Show clutterbar", &config->winamp.showClutterbar)) {
            config->Save();
        }

        ImGui::TableNextColumn();
        if (ImGui::SliderFloat("##sensslider", &config->winamp.visualizerSensitivity, WINAMP_VISUALIZER_SENS_MIN, WINAMP_VISUALIZER_SENS_MAX, "")) {
            config->winamp.visualizerSensitivity *= 100;
            config->winamp.visualizerSensitivity = round(config->winamp.visualizerSensitivity) / 100;
            config->Save();
        }
        ImGui::SameLine();
        ImGui::Text("%.02f", config->winamp.visualizerSensitivity);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Skin transparency", &config->winamp.skinTransparency)) {
            config->Save();
        }

        ImGui::TableNextColumn();

        ImGui::EndTable();
    }

    ImGui::PopStyleVar(2);
}

void Skin::Cassette() {
    if (!connector->storagePresent) {
        ImGui::Text("Disable USB mass storage mode");
        return;
    }

    if (ImGui::Checkbox("Randomize?", &config->cassette.randomize)) {
        config->Save();
    }

    static ImGuiTableFlags flags =
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchProp;

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
            if (tt.second.name == Cassette::hiddenEntry) {
                continue;
            }
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
                        DLOG("invalid %s\n", n.name.c_str());
                        continue;
                    }

                    if (ImGui::Selectable(n.name.c_str(), false)) {
                        tt.second.reel = n.name;
                        config->cassette.data.at(tt.first).reel = n.name;
                        config->Save();

                        if (activeSkinVariant == CASSETTE) {
                            cassette.UnloadUnused();
                            if (cassette.LoadReelAtlas(n.name) != Tape::ERR_OK) {
                                cassette.LoadReel(n.name);
                            }
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

void Skin::DigitalClock() {
    if (!connector->storagePresent) {
        ImGui::Text("Disable USB mass storage mode");
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    if (ImGui::BeginCombo("##digiClockColor", DigitalClock::DigitalClock::GetColorPreview(config->digitalClock.color).c_str(), ImGuiComboFlags_HeightRegular)) {
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
void Skin::DrawSettings() {
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

void Skin::PreprocessTableFilenames() {
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

    for (int i = 0; i < masterVolumeFiles.size(); i++) {
        if (masterVolumeFiles.at(i).fullPath == config->volumeTables.MasterVolumeTable) {
            masterVolumeFiles.at(i).name = defaultMark + masterVolumeFiles.at(i).name;
            break;
        }
    }

    for (int i = 0; i < masterVolumeDSDFiles.size(); i++) {
        if (masterVolumeDSDFiles.at(i).fullPath == config->volumeTables.MasterVolumeTableDSD) {
            masterVolumeDSDFiles.at(i).name = defaultMark + masterVolumeDSDFiles.at(i).name;
            break;
        }
    }

    for (int i = 0; i < toneControlFiles.size(); i++) {
        if (toneControlFiles.at(i).fullPath == config->volumeTables.ToneControl) {
            toneControlFiles.at(i).name = defaultMark + toneControlFiles.at(i).name;
            break;
        }
    }
}

void Skin::CopyTableEntry(TableLike *table, std::vector<directoryEntry> *fileList, int *selectedIndex, const std::string &outDir) {
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

void Skin::TabMasterVolume() {
    if (ImGui::BeginTabItem(Dac::TableTypeToString.at(TABLE_ID_MASTER_VOLUME).c_str())) {
        curveEditorTarget = (float *)masterVolumeValues[int(soundEffectOn)][MasterVolumeTableType][MasterVolumeValueType];
        curveYLimit = (1 << 8) - 1; // 255
        curveElementCount = MASTER_VOLUME_MAX + 1;

        if (ImGui::BeginTable("##mvtable", 3, ImGuiTableFlags_None)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, 388);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("File:");

            ImGui::TableNextColumn();
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
            ImGui::PushItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##masterVolumeFileCombo", masterVolumeFiles.at(masterVolumeFileSelected).name.c_str(), ImGuiComboFlags_HeightRegular)) {
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
                    "##masterVolumeValueTypeCombo", Dac::MasterVolumeValueTypeToString.at(MasterVolumeValueType).c_str(), ImGuiComboFlags_HeightRegular)) {
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
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, 512);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (masterVolumeValues[0][0][0][0].x == -FLT_MIN) {
                ImGui::NewLine();
                if (ImGui::Button("Load", ImVec2(512, 150))) {
                    if (masterVolume.FromFile(masterVolumeFiles.at(masterVolumeFileSelected).fullPath) != 0) {
                        DLOG("failed to load master volume table file %s\n", masterVolumeFiles.at(masterVolumeFileSelected).fullPath.c_str());
                        statusStringMasterVolume = "Failed";
                    } else {
                        MasterVolumeTableToImVec2();
                        statusStringMasterVolume = "Loaded";
                    }
                }
            } else {
                auto flags = ImPlotFlags_None;
                if (ImPlot::BeginPlot("##lines", ImVec2(512, 270), ImPlotFlags_NoMouseText | ImPlotFlags_NoInputs)) {
                    ImPlot::SetupAxesLimits(
                        curveElementCount * -0.05, curveElementCount + curveElementCount * 0.05, curveYLimit * -0.05, curveYLimit + curveYLimit * 0.1);
                    ImPlot::SetNextLineStyle(GOLD_DONATE, 2.0f);
                    ImPlot::PlotInfLines("##volline", &connector->status.VolumeRaw, 1);
                    ImPlot::SetNextLineStyle(IMPLOT_AUTO_COL, 2.0f);
                    ImPlot::PlotLineG("##curvaSmall", ImPlot::ImVec2Getter, curveEditorTarget, curveElementCount);
                    ImPlot::EndPlot();
                }

                ImGui::TableNextColumn();

                // not efficient at all
                if (masterVolumeFiles.at(masterVolumeFileSelected).name.rfind(systemMark, 0) == 0) {
                    if (ImGui::Button("Copy and edit", ImVec2(246, 60))) {
                        auto outDir = soundSettingsPathUser + "master_volume/";
                        CopyTableEntry(&masterVolume, &masterVolumeFiles, &masterVolumeFileSelected, outDir);

                        for (int i = 0; i < curveElementCount; i++) {
                            valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
                            valuesy[i] = double(((ImVec2 *)(curveEditorTarget) + i)->y);
                        }

                        zoomLimitXL = (double)curveElementCount * -0.05;
                        zoomLimitXR = (double)curveElementCount * 1.05;
                        zoomLimitYT = (double)curveYLimit * 1.05;
                        zoomLimitYB = (double)curveYLimit * -0.05;

                        displayTab = TabCurveEditor;
                    }
                } else {
                    if (ImGui::Button("Edit", ImVec2(246, 60))) {

                        for (int i = 0; i < curveElementCount; i++) {
                            valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
                            valuesy[i] = double(((ImVec2 *)(curveEditorTarget) + i)->y);
                        }

                        zoomLimitXL = (double)curveElementCount * -0.05;
                        zoomLimitXR = (double)curveElementCount * 1.05;
                        zoomLimitYT = (double)curveYLimit * 1.05;
                        zoomLimitYB = (double)curveYLimit * -0.05;

                        displayTab = TabCurveEditor;
                    }
                }

                if (masterVolumeFiles.at(masterVolumeFileSelected).name.rfind(systemMark, 0) == 0) {
                    ImGui::BeginDisabled();
                }

                if (ImGui::Button("Save", ImVec2(121, 60))) {
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

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
                ImGui::SameLine();

                if (ImGui::Button("Apply", ImVec2(121, 60))) {
                    DLOG("Applying\n");
                    MasterVolumeImVec2ToTable();
                    if (masterVolume.Apply(Dac::volumeTableOutPath) == 0) {
                        statusStringMasterVolume = "Applied";
                    } else {
                        statusStringMasterVolume = "Failed";
                    }
                }

                if (ImGui::Button("Copy val", ImVec2(121, 60))) {
                    memcpy(masterVolumeValueBuffer,
                        masterVolumeValues[(int)soundEffectOn][MasterVolumeTableType][MasterVolumeValueType],
                        sizeof(masterVolumeValueBuffer));
                    statusStringMasterVolume = "Copied";
                }

                ImGui::SameLine();
                if (ImGui::Button("Paste val", ImVec2(121, 60))) {
                    memcpy(masterVolumeValues[(int)soundEffectOn][MasterVolumeTableType][MasterVolumeValueType],
                        masterVolumeValueBuffer,
                        sizeof(masterVolumeValueBuffer));
                    statusStringMasterVolume = "Pasted";
                }

                if (ImGui::Button("Set as default", ImVec2(246, 60))) {
                    DLOG("Setting master volume table %s as default\n", masterVolumeFiles.at(masterVolumeFileSelected).fullPath.c_str());
                    config->volumeTables.MasterVolumeTable = masterVolumeFiles.at(masterVolumeFileSelected).fullPath;
                    config->Save();

                    for (int i = 0; i < masterVolumeFiles.size(); i++) {
                        if (masterVolumeFiles.at(i).name.rfind(defaultMark, 0) == 0) {
                            masterVolumeFiles.at(i).name.erase(0, defaultMark.length());
                        }
                        if (i == masterVolumeFileSelected) {
                            masterVolumeFiles.at(i).name = defaultMark + masterVolumeFiles.at(i).name;
                        }
                    }

                    statusStringMasterVolume = "Set as default";
                }

                ImGui::PopStyleVar();
            }
            ImGui::EndTable();
        }
        ImGui::EndTabItem();
    }
}

void Skin::TabMasterDSDVolume() {
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
                    "##masterVolumeDSDFileCombo", masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).name.c_str(), ImGuiComboFlags_HeightRegular)) {
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
                        DLOG("failed to load master volume DSD table file %s\n", masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).fullPath.c_str());
                        statusStringMasterVolumeDSD = "Failed";
                    } else {
                        MasterVolumeDSDTableToImVec2();
                        statusStringMasterVolumeDSD = "Loaded";
                    }
                }
            } else {
                if (ImPlot::BeginPlot("##lines", ImVec2(512, 305), ImPlotFlags_NoMouseText | ImPlotFlags_NoInputs)) {
                    ImPlot::SetupAxesLimits(
                        curveElementCount * -0.05, curveElementCount + curveElementCount * 0.05, curveYLimit * -0.05, curveYLimit + curveYLimit * 0.1);
                    ImPlot::SetNextLineStyle(GOLD_DONATE, 2.0f);
                    ImPlot::PlotInfLines("##volline", &connector->status.VolumeRaw, 1);
                    ImPlot::SetNextLineStyle(IMPLOT_AUTO_COL, 2.0f);
                    ImPlot::PlotLineG("##curvaSmall", ImPlot::ImVec2Getter, curveEditorTarget, curveElementCount);
                    ImPlot::EndPlot();
                }

                ImGui::TableNextColumn();

                // not efficient at all
                if (masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).name.rfind(systemMark, 0) == 0) {
                    if (ImGui::Button("Copy and edit", ImVec2(246, 60))) {
                        auto outDir = soundSettingsPathUser + "master_volume_dsd/";
                        CopyTableEntry(&masterVolumeDSD, &masterVolumeDSDFiles, &masterVolumeDSDFileSelected, outDir);

                        for (int i = 0; i < curveElementCount; i++) {
                            valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
                            valuesy[i] = double(((ImVec2 *)(curveEditorTarget) + i)->y);
                        }

                        zoomLimitXL = (double)curveElementCount * -0.05;
                        zoomLimitXR = (double)curveElementCount * 1.05;
                        zoomLimitYT = (double)curveYLimit * 1.05;
                        zoomLimitYB = (double)curveYLimit * -0.05;

                        displayTab = TabCurveEditor;
                    }
                } else {
                    if (ImGui::Button("Edit", ImVec2(246, 60))) {

                        for (int i = 0; i < curveElementCount; i++) {
                            valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
                            valuesy[i] = double(((ImVec2 *)(curveEditorTarget) + i)->y);
                        }

                        zoomLimitXL = (double)curveElementCount * -0.05;
                        zoomLimitXR = (double)curveElementCount * 1.05;
                        zoomLimitYT = (double)curveYLimit * 1.05;
                        zoomLimitYB = (double)curveYLimit * -0.05;

                        displayTab = TabCurveEditor;
                    }
                }

                if (masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).name.rfind(systemMark, 0) == 0) {
                    ImGui::BeginDisabled();
                }

                if (ImGui::Button("Save", ImVec2(121, 60))) {
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
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
                ImGui::SameLine();

                if (ImGui::Button("Apply", ImVec2(121, 60))) {
                    DLOG("Applying to %s\n", Dac::volumeTableDSDOutPath.c_str());
                    MasterVolumeDSDImVec2ToTable();
                    if (masterVolumeDSD.Apply(Dac::volumeTableDSDOutPath) == 0) {
                        statusStringMasterVolumeDSD = "Applied";
                    } else {
                        statusStringMasterVolumeDSD = "Failed";
                    }
                }

                if (ImGui::Button("Copy val", ImVec2(121, 60))) {
                    memcpy(masterVolumeDSDValueBuffer, masterVolumeDSDValues[MasterVolumeDSDTableType], sizeof(masterVolumeDSDValueBuffer));
                    statusStringMasterVolumeDSD = "Copied";
                }

                ImGui::SameLine();
                if (ImGui::Button("Paste val", ImVec2(121, 60))) {
                    memcpy(masterVolumeDSDValues[MasterVolumeDSDTableType], masterVolumeDSDValueBuffer, sizeof(masterVolumeDSDValueBuffer));
                    statusStringMasterVolumeDSD = "Pasted";
                }

                if (ImGui::Button("Set as default", ImVec2(246, 60))) {
                    DLOG("Setting master volume DSD table %s as default\n", masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).fullPath.c_str());
                    config->volumeTables.MasterVolumeTableDSD = masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).fullPath;
                    config->Save();

                    for (int i = 0; i < masterVolumeDSDFiles.size(); i++) {
                        if (masterVolumeDSDFiles.at(i).name.rfind(defaultMark, 0) == 0) {
                            masterVolumeDSDFiles.at(i).name.erase(0, defaultMark.length());
                        }
                        if (i == masterVolumeDSDFileSelected) {
                            masterVolumeDSDFiles.at(i).name = defaultMark + masterVolumeDSDFiles.at(i).name;
                        }
                    }

                    statusStringMasterVolumeDSD = "Set as default";
                }

                ImGui::PopStyleVar();
            }

            ImGui::EndTable();
        }

        ImGui::EndTabItem();
    }
}

void Skin::TabToneControl() {
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
            if (ImGui::BeginCombo("##toneControlFileCombo", toneControlFiles.at(toneControlFileSelected).name.c_str(), ImGuiComboFlags_HeightRegular)) {
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
                        DLOG("failed to load tone control table file %s\n", toneControlFiles.at(toneControlFileSelected).fullPath.c_str());
                        statusStringToneControl = "Failed";
                    } else {
                        ToneControlToImVec2();
                        statusStringToneControl = "Loaded";
                    }
                }
            } else {
                if (ImPlot::BeginPlot("##lines", ImVec2(512, 305), ImPlotFlags_NoMouseText | ImPlotFlags_NoInputs)) {
                    ImPlot::SetupAxesLimits(
                        curveElementCount * -0.05, curveElementCount + curveElementCount * 0.05, curveYLimit * -0.05, curveYLimit + curveYLimit * 0.1);
                    ImPlot::SetNextLineStyle(GOLD_DONATE, 2.0f);
                    ImPlot::PlotLineG("##curvaSmall", ImPlot::ImVec2Getter, curveEditorTarget, curveElementCount);
                    ImPlot::EndPlot();
                }

                ImGui::TableNextColumn();

                // not efficient at all
                if (toneControlFiles.at(toneControlFileSelected).name.rfind(systemMark, 0) == 0) {
                    if (ImGui::Button("Copy and edit", ImVec2(246, 60))) {
                        auto outDir = soundSettingsPathUser + "tone_control/";
                        CopyTableEntry(&toneControl, &toneControlFiles, &toneControlFileSelected, outDir);

                        for (int i = 0; i < curveElementCount; i++) {
                            valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
                            valuesy[i] = double(((ImVec2 *)(curveEditorTarget) + i)->y);
                        }

                        zoomLimitXL = (double)curveElementCount * -0.05;
                        zoomLimitXR = (double)curveElementCount * 1.05;
                        zoomLimitYT = (double)curveYLimit * 1.05;
                        zoomLimitYB = (double)curveYLimit * -0.05;

                        displayTab = TabCurveEditor;
                    }
                } else {
                    if (ImGui::Button("Edit", ImVec2(246, 60))) {
                        for (int i = 0; i < curveElementCount; i++) {
                            valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
                            valuesy[i] = double(((ImVec2 *)(curveEditorTarget) + i)->y);
                        }

                        zoomLimitXL = (double)curveElementCount * -0.05;
                        zoomLimitXR = (double)curveElementCount * 1.05;
                        zoomLimitYT = (double)curveYLimit * 1.05;
                        zoomLimitYB = (double)curveYLimit * -0.05;

                        displayTab = TabCurveEditor;
                    }
                }

                if (toneControlFiles.at(toneControlFileSelected).name.rfind(systemMark, 0) == 0) {
                    ImGui::BeginDisabled();
                }

                if (ImGui::Button("Save", ImVec2(121, 60))) {
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

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
                ImGui::SameLine();

                if (ImGui::Button("Apply", ImVec2(121, 60))) {
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

                ImGui::SameLine();
                if (ImGui::Button("Paste val", ImVec2(121, 60))) {
                    memcpy(toneControlValues[toneControlTableType], toneControlValueBuffer, sizeof(toneControlValueBuffer));
                    statusStringToneControl = "Pasted";
                }

                if (ImGui::Button("Set as default", ImVec2(246, 60))) {
                    DLOG("Setting tone control table %s as default\n", toneControlFiles.at(toneControlFileSelected).fullPath.c_str());
                    config->volumeTables.ToneControl = toneControlFiles.at(toneControlFileSelected).fullPath;
                    config->Save();

                    for (int i = 0; i < toneControlFiles.size(); i++) {
                        if (toneControlFiles.at(i).name.rfind(defaultMark, 0) == 0) {
                            toneControlFiles.at(i).name.erase(0, defaultMark.length());
                        }
                        if (i == toneControlFileSelected) {
                            toneControlFiles.at(i).name = defaultMark + toneControlFiles.at(i).name;
                        }
                    }

                    statusStringToneControl = "Set as default";
                }

                ImGui::PopStyleVar();
            }

            ImGui::EndTable();
        }
        ImGui::EndTabItem();
    }
}

void Skin::TabSoundStatus() {
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

void Skin::SoundSettingsTab() {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
    if (!connector->storagePresent) {
        ImGui::Text("Disable USB mass storage mode");
        return;
    }
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("SoundSettingsTabBar", tab_bar_flags)) {
        TabMasterVolume();
        TabMasterDSDVolume();
        TabToneControl();
        TabSoundStatus();
        ImGui::EndTabBar();
    }
}

void Skin::CurveEditor() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
    ImGui::BeginChild("##curvaBig",
        ImVec2(windowSize.x - 15 * 2 - (ImGui::GetStyle().FramePadding.x * 2.0f + 98 * 2 + ImGui::GetStyle().ItemSpacing.x * 2), 480),
        ImGuiChildFlags_None,
        window_flags);

    bool clicked = false;
    bool hovered = false;
    bool held[curveElementCount];

    for (int i = 0; i < curveElementCount; i++) {
        valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
    }

    auto plotWidth = windowSize.x - 15 * 2 - (ImGui::GetStyle().FramePadding.x * 2.0f + 98 * 2 + ImGui::GetStyle().ItemSpacing.x * 2);
    if (ImPlot::BeginPlot("##lines", ImVec2(plotWidth, 480), ImPlotFlags_NoMouseText | ImPlotFlags_NoBoxSelect)) {
        ImPlot::SetupAxesLimits(zoomLimitXL, zoomLimitXR, zoomLimitYB, zoomLimitYT, ImPlotCond_Always);

        float radius = 10;
        for (int i = 0; i < curveElementCount; i++) {
            ImPlot::DragPoint(i * 134, &valuesx[i], &valuesy[i], GOLD_DONATE, radius, ImPlotDragToolFlags_None, &clicked, &hovered, &held[i]);
            if (held[i]) {
                auto posx = double(((ImVec2 *)curveEditorTarget + i)->x);
                float offset = 70;
                if ((plotWidth - ImGui::GetMousePos().x) < 120) {
                    offset = -70;
                }
                ImPlot::Annotation(posx, valuesy[i], IMPLOT_AUTO_COL, ImVec2(offset, 10), false, true);
            }

            if (valuesy[i] > curveYLimit) {
                valuesy[i] = curveYLimit;
            }

            if (valuesy[i] < 0) {
                valuesy[i] = 0;
            }
        }

        ImPlot::SetNextLineStyle(IMPLOT_AUTO_COL, 2.0f);
        ImPlot::PlotLine("##curvaBig", valuesx, valuesy, curveElementCount);
        ImPlot::EndPlot();
    }

    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("##curvaBigButtons", ImVec2(windowSize.x - ImGui::GetCursorPosX(), 480), ImGuiChildFlags_None, window_flags);

    if (ImGui::Button("Zoom in", ImVec2(110, 60))) {
        zoomLimitXL = zoomLimitXL + curveElementCount * 0.06;
        zoomLimitXR = zoomLimitXR - curveElementCount * 0.06;
        zoomLimitYB = zoomLimitYB + curveYLimit * 0.06;
        zoomLimitYT = zoomLimitYT - curveYLimit * 0.06;
    }

    ImGui::SameLine();
    if (ImGui::Button("Zoom out", ImVec2(110, 60))) {
        zoomLimitXL = zoomLimitXL - curveElementCount * 0.07;
        zoomLimitXR = zoomLimitXR + curveElementCount * 0.07;
        zoomLimitYB = zoomLimitYB - curveYLimit * 0.07;
        zoomLimitYT = zoomLimitYT + curveYLimit * 0.07;
    }

    ImGui::NewLine();
    if (ImGui::Button("Left", ImVec2(110, 60))) {
        zoomLimitXL = zoomLimitXL - curveElementCount * 0.1;
        zoomLimitXR = zoomLimitXR - curveElementCount * 0.1;
    }

    ImGui::SameLine();
    if (ImGui::Button("Right", ImVec2(110, 60))) {
        zoomLimitXL = zoomLimitXL + curveElementCount * 0.09;
        zoomLimitXR = zoomLimitXR + curveElementCount * 0.09;
    }

    ImGui::NewLine();
    if (ImGui::Button("Up", ImVec2(110, 60))) {
        zoomLimitYB = zoomLimitYB + curveYLimit * 0.1;
        zoomLimitYT = zoomLimitYT + curveYLimit * 0.1;
    }

    ImGui::SameLine();
    if (ImGui::Button("Down", ImVec2(110, 60))) {
        zoomLimitYB = zoomLimitYB - curveYLimit * 0.09;
        zoomLimitYT = zoomLimitYT - curveYLimit * 0.09;
    }

    ImGui::NewLine();
    ImGui::Separator();
    ImGui::NewLine();
    ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_RED);
    if (ImGui::Button("Reset", ImVec2(230, 60))) {
        for (int i = 0; i < curveElementCount; i++) {
            valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
            valuesy[i] = double(((ImVec2 *)(curveEditorTarget) + i)->y);
        }
    }
    ImGui::PopStyleColor();
    ImGui::NewLine();
    if (ImGui::Button("Back", ImVec2(230, 60))) {
        for (int i = 0; i < curveElementCount; i++) {
            auto point = ((ImVec2 *)(curveEditorTarget) + i);
            point->y = static_cast<float>(valuesy[i]);
        }
        displayTab = TabSoundSettings;
    }

    ImGui::EndChild();
}

void Skin::Draw() {
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

void Skin::KeyHandler() {
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

        if (activeSkinVariant == WINAMP) {
            if (action == Hide) {
                if (config->winamp.visualizerEnable) {
                    ToggleAudioAnalyzerOn();
                } else {
                    ToggleAudioAnalyzerOff();
                }
            } else {
                if (config->winamp.visualizerEnable) {
                    ToggleAudioAnalyzerOff();
                } else {
                    ToggleAudioAnalyzerOn();
                }
            }
        }

        ImGui::GetIO().AddMousePosEvent(0.0f, 0.0f);
    }

    if (*power_pressed) {
        *power_pressed = false;

        if (*render) {
            *render = false;
        }
    }

    if ((config->disableKeysWhenPowerOff) && (!connector->power)) {
        return;
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

void Skin::GetLogsDirSize() {
    std::vector<directoryEntry> files{};
    listdir("/contents/wampy/log/", &files, "*");

    int res = 0;
    struct stat sb{};
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

void Skin::SetActiveEqFilter(const std::string &v) {
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

void Skin::TabEQ_Donate() const {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 425 / 2 - qrSide / 2 - 28);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 580 / 2 - qrSide / 2);

    ImGui::Image((void *)(intptr_t)qrDonateTexture, ImVec2(qrSide, qrSide));

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 580 / 2 - ImGui::CalcTextSize("boosty.to/unknown321/donate").x / 2);
    ImGui::TextColored(GOLD_DONATE, "boosty.to/unknown321/donate");
}

void Skin::TabEQ_Misc() {
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

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
        } else {
            if (ImGui::Button("Enable", ImVec2(186, 60))) {
                connector->soundSettingsFw.SetDirectSource(true);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("ClearAudio+");

        ImGui::TableNextColumn();
        if (isWalkmanOne || !connector->soundSettings.s->status.clearAudioAvailable) {
            ImGui::BeginDisabled();
        }
        if (connector->soundSettingsFw.s->clearAudioPlusOn) {
            ImGui::PushID(30 + 1);
            if (ImGui::Button("Disable", ImVec2(186, 60))) {
                connector->soundSettingsFw.SetClearAudioPlus(false);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
            ImGui::PopID();
        } else {
            ImGui::PushID(30 + 1);
            if (ImGui::Button("Enable", ImVec2(186, 60))) {
                connector->soundSettingsFw.SetClearAudioPlus(true);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
            ImGui::PopID();
        }

        if (isWalkmanOne || !connector->soundSettings.s->status.clearAudioAvailable) {
            ImGui::EndDisabled();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::Text(effectQueueStatus.c_str());

        ImGui::EndTable();
    }
}

void Skin::SetConfigFilter(std::string name, bool value) {
    switch (hash(name.c_str())) {
    case hash("vpt"):
        config->filters.vpt = value;
        break;
    case hash("vinylizer"):
        config->filters.vinylizer = value;
        break;
    case hash("eqtone"):
        config->filters.eqtone = value;
        break;
    case hash("eq6band"):
        config->filters.eq6Band = value;
        break;
    case hash("eq10band"):
        config->filters.eq10Band = value;
        break;
    case hash("dcphaselinear"):
        config->filters.dcphaselinear = value;
        break;
    case hash("dynamicnormalizer"):
        config->filters.dynamicnormalizer = value;
        break;
    default:
        break;
    }

    config->Save();
}

void Skin::TabEq_EnableDisableFilter() {
    if (connector->soundSettingsFw.filters.at(activeFilter)) {
        if (ImGui::Button("Disable", ImVec2(186, 60))) {
            connector->soundSettingsFw.SetFilter(activeFilter, false);
            SetConfigFilter(activeFilter, false);
            nanosleep(&ssfwUpdateDelay, nullptr);
            connector->soundSettingsFw.Update();
            if (connector->status.State != PLAYING) {
                effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
            } else {
                effectQueueStatus.clear();
            }
        }
    } else {
        if (ImGui::Button("Enable", ImVec2(186, 60))) {
            connector->soundSettingsFw.SetFilter(activeFilter, true);
            SetConfigFilter(activeFilter, true);
            nanosleep(&ssfwUpdateDelay, nullptr);
            connector->soundSettingsFw.Update();
            if (connector->status.State != PLAYING) {
                effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
            } else {
                effectQueueStatus.clear();
            }
        }
    }

    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 9);
    ImGui::Text(effectQueueStatus.c_str());
}

void Skin::TabEQ_Vinylizer() {
    TabEq_EnableDisableFilter();

    ImGui::SeparatorText("Vinylizer type:");
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    ImGui::PushItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##vinylizerValues", vinylTypeToString.at(connector->soundSettingsFw.vinylizerValue).c_str(), ImGuiComboFlags_HeightRegular)) {
        for (const auto &entry : vinylTypeToString) {
            if (entry.second == "?") {
                continue;
            }
            if (ImGui::Selectable(entry.second.c_str(), false)) {
                connector->soundSettingsFw.SetVinylizerType(entry.first);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();
                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleVar(2);
}

void Skin::TabEQ_DCPhase() {
    TabEq_EnableDisableFilter();
    ImGui::SeparatorText("Type:");
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    ImGui::PushItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##DcPhaseValues", dcFilterToString.at(connector->soundSettingsFw.dcValue).c_str(), ImGuiComboFlags_HeightRegular)) {
        for (const auto &entry : dcFilterToString) {
            if (entry.second == "?") {
                continue;
            }
            if (ImGui::Selectable(entry.second.c_str(), false)) {
                connector->soundSettingsFw.SetDcFilterType(entry.first);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleVar(2);
}

void Skin::TabEQ_Vpt() {
    TabEq_EnableDisableFilter();
    ImGui::SeparatorText("VPT type:");
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    ImGui::PushItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##vptValues", vptA50SmallToString.at(connector->soundSettingsFw.vptValue).c_str(), ImGuiComboFlags_HeightRegular)) {
        for (const auto &entry : vptA50SmallToString) {
            if (entry.second == "?") {
                continue;
            }
            if (ImGui::Selectable(entry.second.c_str(), false)) {
                connector->soundSettingsFw.SetVptMode(entry.first);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleVar(2);
}

void Skin::TabEQ_DynamicNormalizer() {
    TabEq_EnableDisableFilter();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 580 / 2 - ImGui::CalcTextSize("No options").x / 2);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 425 / 2 - ImGui::CalcTextSize("No options").y / 2 - ImGui::GetTextLineHeightWithSpacing() - 60);
    ImGui::Text("No options");
}

void Skin::TabEQ_Eq6Band() {
    TabEq_EnableDisableFilter();

    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
    ImGui::SeparatorText("Preset:");
    ImGui::PushItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##eq6preset", eq6PresetToString.at(connector->soundSettingsFw.eq6Value).c_str(), ImGuiComboFlags_HeightRegular)) {
        for (const auto &entry : eq6PresetToString) {
            if (ImGui::Selectable(entry.second.c_str(), false)) {
                connector->soundSettingsFw.SetEq6Preset(entry.first);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
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

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
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

void Skin::TabEQ_Eq10Band() {
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

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
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

void Skin::TabEQ_EqTone() {
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

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
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

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
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

void Skin::TabDac() {
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

void Skin::TabFM() {
    ImGui::NewLine();
    if (!radioAvailable) {
        ImGui::Text("FM radio is unavailable on this device");
        ImGui::NewLine();
        if (ImGui::Button("Hide this tab", ImVec2(200, 60))) {
            config->showFmInSettings = false;
            displayTab = SettingsTab::SkinOpts;
            config->Save();
        }
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
    ImGui::BeginChild("##fmpresets", ImVec2(windowSize.x - 15 * 2 - ImGui::GetStyle().FramePadding.x * 2.0f, 145), ImGuiChildFlags_None, window_flags);
    ImVec2 button_sz(88, 60);
    float window_visible_x2 = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
    for (int i = 0; i < config->fmPresets.size(); i++) {
        float last_button_x2 = ImGui::GetItemRectMax().x;
        float next_button_x2 = last_button_x2 + ImGui::GetStyle().ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
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

void Skin::TabEQ() {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

    if (prevSong != connector->status.Filename) {
        connector->soundSettings.Update();
        connector->soundSettingsFw.Update();
        eqSongExists = SoundSettings::Exists(connector->status.Filename);
        eqSongDirExists = SoundSettings::ExistsDir(connector->status.Filename);
        prevSong = connector->status.Filename;
        eqStatus = "Refreshed";
    }

    if (prevPlayState != connector->status.State) {
        prevPlayState = connector->status.State;
        if (connector->status.State == PLAYING) {
            effectQueueStatus.clear();
            connector->soundSettingsFw.Update();
        }
    }

    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        ImGui::BeginChild("ChildL", ImVec2(580, 425), ImGuiChildFlags_Border, window_flags);

        if (connector->soundSettingsFw.s->directSourceOn) {
            ImGui::Text("Direct Source is on, filters disabled");
            ImGui::NewLine();
            TabEQ_Misc();
        } else {
            switch (eActiveFilterTab) {

            case ActiveFilterTab_Invalid:
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 580 / 2 - ImGui::CalcTextSize("Select filter").x / 2);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 425 / 2 - ImGui::CalcTextSize("Select filter").y / 2 - ImGui::GetTextLineHeightWithSpacing());
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

void Skin::TabEQOld() {
    ImGui::NewLine();
    if (!connector->storagePresent) {
        ImGui::Text("Disable USB mass storage mode");
        return;
    }
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

    static ImGuiTableFlags flags =
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchProp;

    ImVec2 outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 7);

    if (ImGui::BeginTable("##eqtable", 2, flags, outer_size)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("ClearAudio+");
        ImGui::TableNextColumn();
        ImGui::Text("%s (%s)",
            connector->soundSettings.s->status.clearAudioOn == 1 ? "On" : "Off",
            connector->soundSettings.s->status.clearAudioAvailable == 1 ? "available" : "unavailable");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Direct Source");
        ImGui::TableNextColumn();
        ImGui::Text("%s (%s)",
            connector->soundSettings.s->status.directSourceOn == 1 ? "On" : "Off",
            connector->soundSettings.s->status.directSourceAvailable == 1 ? "available" : "unavailable");

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

        ImGui::Text("%d, %d, %d, %d, %d, %d",
            connector->soundSettings.s->status.eq6Bands[0],
            connector->soundSettings.s->status.eq6Bands[1],
            connector->soundSettings.s->status.eq6Bands[2],
            connector->soundSettings.s->status.eq6Bands[3],
            connector->soundSettings.s->status.eq6Bands[4],
            connector->soundSettings.s->status.eq6Bands[5]);
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
        ImGui::Text("%.1f, %.1f, %.1f, %.1f, %.1f,\n%.1f, %.1f, %.1f, %.1f, %.1f",
            float(connector->soundSettings.s->status.eq10Bands[0]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[1]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[2]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[3]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[4]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[5]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[6]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[7]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[8]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[9]) / 2);

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

void Skin::CalcWindowPos() const {
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