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
        eqStatus = gettext("Refreshed");
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
        eqStatus = gettext("Refreshed");
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

    auto miscSize = ImGui::CalcTextSize(gettext("Misc")).x + ImGui::GetStyle().FramePadding.x * 2.f;
    auto skinSize = ImGui::CalcTextSize(gettext("Skin")).x + ImGui::GetStyle().FramePadding.x * 2.f;
    auto closeSize = ImGui::CalcTextSize(gettext("Close")).x + ImGui::GetStyle().FramePadding.x * 2.f;

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
    if (ImGui::Button(gettext("Skin"))) {
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
    if (ImGui::Button(gettext("Misc"))) {
        loadStatusStr = "";
        displayTab = SettingsTab::Misc;
    }

    ImGui::SameLine(800.0f - closeSize - indent);
    if (ImGui::Button(gettext("Close"))) {
        loadStatusStr = "";
        ToggleDrawSettings(this, nullptr);
    }
}

void Skin::WalkmanOneTab() {
    if (isWalkmanOne) {
        WalkmanOne();
    } else {
        Wee1();
    }
}

void Skin::DrawSkin() {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 15.0f));
    ImGui::SeparatorText(gettext("Skin:"));
    ImGui::PopStyleVar();

    ImGui::RadioButton("Winamp", &activeSettingsTab, WINAMP);
    ImGui::SameLine();
    if (ImGui::RadioButton(gettext("Cassette"), &activeSettingsTab, CASSETTE)) {
        loadStatusStr = "";
    }
    ImGui::SameLine();
    if (ImGui::RadioButton(gettext("DigiClock"), &activeSettingsTab, DIGITAL_CLOCK)) {
        loadStatusStr = "";
    }

    if (connector->storagePresent) {
        if (activeSkinVariant != activeSettingsTab) {
            ImGui::SameLine();
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 10);
            if (ImGui::Button(gettext("Set as active"), ImVec2(186, 50))) {
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
    case SettingsTab::TabDebug:
        TabDebug();
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

    logCleanupButtonLabel = gettext("Remove Wampy logs (") + std::to_string(res) + " MB)";
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