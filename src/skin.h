#ifndef WAMPY_SKIN_H
#define WAMPY_SKIN_H

#include "Version.h"
#include "cassette/cassette.h"
#include "config.h"
#include "digital_clock/digital_clock.h"
#include "skinVariant.h"
#include "w1/w1.h"
#include "winamp/winamp.h"
#include <thread>

#define SETTINGS_FONT_SIZE 25

enum SettingsTab {
    SkinOpts = 0,
    Misc = 1,
    TabFonts = 2,
    TabLicense3rd = 3,
    TabLicense = 4,
    TabWebsite = 5,
    TabWalkmanOne = 6,
};

struct Skin {
    bool *render{};
    ImFont *FontRegular{};
    SkinList *skinList{};
    SkinList *reelList{};
    SkinList *tapeList{};
    W1::W1Options w1Options{};
    W1::WalkmanOneOptions walkmanOneOptions{};
    int selectedSkinIdx{};       // winamp skin idx
    std::string loadStatusStr{}; // skin loading status in settings
    Connector *connector{};
    bool loading{};
    bool needLoad{};
    bool *hold_toggled = nullptr;
    int *hold_value = nullptr;
    bool *power_pressed = nullptr;
    int displaySettings{};
    SettingsTab displayTab = SettingsTab::SkinOpts;

    ESkinVariant activeSkinVariant = EMPTY;
    int activeSettingsTab{};
    Winamp::Winamp winamp{};
    Cassette::Cassette cassette{};
    DigitalClock::DigitalClock digitalClock{};

    AppConfig::AppConfig *config{};
    bool onlyFont{};

    std::string needRestartFontsText = "Changes will be applied on device restart";
    bool needRestartFonts{};
    bool needRestartWalkmanOne{};

    std::string license{};
    std::string license3rd{};

#ifdef DESKTOP
    std::string licensePath = "../LICENSE";
    std::string license3rdPath = "../LICENSE_3rdparty";
    std::string qrPath = "../qr.bmp";
#else
    std::string licensePath = "/system/vendor/unknown321/usr/share/wampy/doc/LICENSE";
    std::string license3rdPath = "/system/vendor/unknown321/usr/share/wampy/doc/LICENSE_3rdparty";
    std::string qrPath = "/system/vendor/unknown321/usr/share/wampy/qr.bmp";
#endif
    GLuint qrTexture{};
    float qrSide{};
    bool isWalkmanOne{};

    void ReloadFont() {
        loading = true;
        std::string filepath{};
        switch (activeSkinVariant) {
        case CASSETTE:
            cassette.AddFonts(&FontRegular);
            break;
        case WINAMP:
            winamp.Unload();

            if (!SkinExists(config->winamp.filename, skinList, &filepath)) {
                DLOG("no skin %s found in skinlist\n", config->winamp.filename.c_str());
                exit(1);
            }

            break;
        default:
            break;
        }

        config->Save();

        onlyFont = false;
        loading = false;
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
    }

    void Load() {
        if (onlyFont && activeSkinVariant == CASSETTE) {
            DLOG("reloading font for cassette\n");
            ReloadFont();
            return;
        }

        if (ImGui::GetCurrentContext()->WithinFrameScope) {
            DLOG("cannot load in frame, perhaps this https://github.com/ocornut/imgui/pull/3761 ?\n");
            return;
        }

        loading = true;

        if (config->activeSkin == WINAMP) {
            DLOG("loading winamp\n");
            cassette.Unload();
            cassette.active = false;
            digitalClock.Unload();

            std::string filepath{};
            if (!SkinExists(config->winamp.filename, skinList, &filepath)) {
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

            cassette.reelList = reelList;
            cassette.tapeList = tapeList;
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
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.06f, 0.06f, 0.94f));
        }

        activeSkinVariant = config->activeSkin;
        activeSettingsTab = activeSkinVariant;
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
            loadStatusStr = "Loaded " + std::string(basename(winamp.GetCurrentSkin().c_str()));

            for (const auto &v : *skinList) {
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

    static void ToggleDrawSettings(void *skin, void *) {
        auto s = (Skin *)skin;
        assert(s);
        if (s->displaySettings == 0) {
            s->displaySettings = 1;
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
        ImGui::Indent(15.0f);
        ImGui::Text("Settings");

        char buffer[9];
        time_t rawtime;
        time(&rawtime);
        const auto timeinfo = localtime(&rawtime);
        strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);

        ImGui::SameLine(380.0f);
        ImGui::Text("%s", buffer);

        float offset = 15.0f;

        ImGui::SameLine(ImGui::CalcTextSize("Settings").x + ImGui::GetStyle().FramePadding.x * 2.f + offset * 2);
        if (ImGui::Button("W1")) {
            loadStatusStr = "";
            displayTab = SettingsTab::TabWalkmanOne;
        }

        auto fontsSize = ImGui::CalcTextSize("Fonts").x + ImGui::GetStyle().FramePadding.x * 2.f;
        auto miscSize = ImGui::CalcTextSize("Misc").x + ImGui::GetStyle().FramePadding.x * 2.f;
        auto skinSize = ImGui::CalcTextSize("Skin").x + ImGui::GetStyle().FramePadding.x * 2.f;
        auto closeSize = ImGui::CalcTextSize("Close").x + ImGui::GetStyle().FramePadding.x * 2.f;
        ImGui::SameLine(800.0f - closeSize - miscSize - skinSize - fontsSize - offset * 4);
        if (ImGui::Button("Skin")) {
            loadStatusStr = "";
            displayTab = SettingsTab::SkinOpts;
        }

        ImGui::SameLine(800.0f - closeSize - miscSize - fontsSize - offset * 3);
        if (ImGui::Button("Fonts")) {
            loadStatusStr = "";
            displayTab = SettingsTab::TabFonts;
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

    void Fonts() {
        ImGui::NewLine();

        ImGui::Text("Additional languages to display text:");

        if (ImGui::Checkbox("Cyrillic", &config->fontRanges.Cyrillic)) {
            config->Save();
            needRestartFonts = true;
        }
        if (ImGui::Checkbox("Greek", &config->fontRanges.Greek)) {
            config->Save();
            needRestartFonts = true;
        }
        if (ImGui::Checkbox("Japanese", &config->fontRanges.Japanese)) {
            config->Save();
            needRestartFonts = true;
        }
        if (ImGui::Checkbox("Thai", &config->fontRanges.Thai)) {
            config->Save();
            needRestartFonts = true;
        }
        if (ImGui::Checkbox("Vietnamese", &config->fontRanges.Vietnamese)) {
            config->Save();
            needRestartFonts = true;
        }
        if (ImGui::Checkbox("Korean", &config->fontRanges.Korean)) {
            config->Save();
            needRestartFonts = true;
        }

#ifndef DESKTOP
        ImGui::BeginDisabled(true);
#endif
        if (ImGui::Checkbox("Chinese (desktop only)", &config->fontRanges.ChineseFull)) {
            config->Save();
            needRestartFonts = true;
        }
#ifndef DESKTOP
        ImGui::EndDisabled();
#endif

        ImGui::NewLine();
        if (needRestartFonts) {
            ImGui::Text("%s", needRestartFontsText.c_str());
        }
    }

    void Misc() {
#ifndef DESKTOP
        ImGui::NewLine();
        if (ImGui::Checkbox("Swap prev/next buttons", &config->misc.swapTrackButtons)) {
            config->Save();
        }

        if (ImGui::Checkbox("Huge cover art", &config->features.bigCover)) {
            config->Save();
            connector->FeatureBigCover(config->features.bigCover);
        }

        if (isWalkmanOne == false) {
            if (ImGui::Checkbox("Show time", &config->features.showTime)) {
                config->Save();
                connector->FeatureShowTime(config->features.showTime);
            }
        }

        if (ImGui::Checkbox("Limit max volume", &config->features.limitVolume)) {
            config->Save();
            connector->FeatureSetMaxVolume(config->features.limitVolume);
        }

        if (ImGui::Checkbox("Disable touchscreen", &config->features.touchscreenStaysOFF)) {
            config->Save();
        }
#endif

        ImGui::NewLine();
        if (ImGui::Checkbox("Debug", &config->debug)) {
            winamp.debug = config->debug;
            cassette.debug = config->debug;
            config->Save();
        }

        if (ImGui::Checkbox("Limit fps", &config->limitFPS)) {
            config->Save();
        }

        auto website = ImGui::CalcTextSize("Website");
        auto verSize = ImGui::CalcTextSize(SOFTWARE_VERSION);
        auto licenseSize = ImGui::CalcTextSize("License");
        auto license3Size = ImGui::CalcTextSize("License 3rdparty");
        auto offset = 15.0f;

#ifndef DESKTOP
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
#endif
        ImGui::SetCursorPosY(480 - verSize.y - ImGui::GetStyle().FramePadding.y);
        printFPS();

        ImGui::SetCursorPosY(480 - verSize.y - licenseSize.y * 3 - ImGui::GetStyle().FramePadding.y * 2 - offset * 2);
        ImGui::SetCursorPosX(800 - website.x - ImGui::GetStyle().FramePadding.x - offset);
        if (ImGui::Button("Website")) {
            displayTab = SettingsTab::TabWebsite;
            loadStatusStr = "";
        }

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
        ImGui::Text("https://github.com/unknown321/wampy");
        ImGui::NewLine();
        ImGui::Image((void *)(intptr_t)qrTexture, ImVec2(qrSide, qrSide));
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
            if (ImGui::Button("Reboot device")) {
                DLOG("rebooting\n");
#ifndef DESKTOP
                walkmanOneOptions.Reboot();
#endif
            }
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
            if (ImGui::Button("Reboot device")) {
                DLOG("rebooting\n");
#ifndef DESKTOP
                system("sync");
                system("reboot");
#endif
            }
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
        ImGui::NewLine();
        ImGui::SeparatorText("Skin:");

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
            if (ImGui::Button("Set as active")) {
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
        if (ImGui::BeginCombo("##", skinList->at(selectedSkinIdx).name.c_str(), ImGuiComboFlags_HeightRegular)) {
            for (int n = 0; n < skinList->size(); n++) {
                const bool is_selected = (selectedSkinIdx == n);
                if (ImGui::Selectable(skinList->at(n).name.c_str(), is_selected)) {
                    selectedSkinIdx = n;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleVar(2);

        ImGui::SameLine();
        if (activeSkinVariant == WINAMP) {
            if (ImGui::Button("Load skin")) {
                loadStatusStr = "Loading " + skinList->at(selectedSkinIdx).name;
                winamp.changeSkin(skinList->at(selectedSkinIdx).fullPath);
                needLoad = true;
            }
        }

        if (!loadStatusStr.empty()) {
            ImGui::Text("%s", loadStatusStr.c_str());
        }

        ImGui::NewLine();

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
                    for (auto &n : *tapeList) {
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
                    for (auto &n : *reelList) {
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

        if (ImGui::Button("Reset")) {
            DLOG("resetting cassette config\n");
            cassette.config->Default();
            config->Save();
            if (activeSkinVariant == CASSETTE) {
                cassette.SelectTape();
                cassette.LoadImages();
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
        Header();
        switch (displayTab) {
        case SettingsTab::SkinOpts:
            DrawSkin();
            break;
        case SettingsTab::Misc:
            Misc();
            break;
        case SettingsTab::TabFonts:
            Fonts();
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
        default:
            break;
        }

        ImGui::PopFont();
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

    void KeyHandler() const {

        if (*hold_toggled) {
            *hold_toggled = false;

            HgrmToggleAction action = Show;
            if (*hold_value == 1) {
                action = Hide;
            }

            DLOG("hold toggled, value %d, render %d, action %d\n", *hold_value, *render, action);
            connector->ToggleHgrm(action, render);
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
};

#endif // WAMPY_SKIN_H