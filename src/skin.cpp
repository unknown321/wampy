#include "skin.h"
#include "implot_widgets.h"
#include "mkpath.h"

#include <libintl.h>

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

    fm_codec = config->fmRecording.Codec == "wav" ? RecordCodec::WAV : RecordCodec::MP3;
    fm_storage = config->fmRecording.Storage == "sdcard" ? RecordStorage::SD_CARD : RecordStorage::INTERNAL;

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
        loadStatusStr = gettext("✓ Loaded ") + std::string(basename(winamp.GetCurrentSkin().c_str()));

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
    ImGui::Indent(indent);

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

    auto skinButtonPos = 800.0f - closeSize - miscSize - skinSize - indent * 3;

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

    ImGui::SameLine(800.0f - closeSize - miscSize - indent * 2);
    if (ImGui::Button("Misc")) {
        loadStatusStr = "";
        displayTab = SettingsTab::Misc;
    }

    ImGui::SameLine(800.0f - closeSize - indent);
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
        ImGui::Text(gettext("Interface color"));

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