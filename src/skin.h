#ifndef SKIN_H
#define SKIN_H

#include "cassette/cassette.h"
#include "config.h"
#include "skinVariant.h"
#include "winamp/winamp.h"
#include <thread>

enum SettingsTab {
    SkinOpts = 0,
    Misc = 1,
    TabFonts = 2,
};

struct Skin {
    bool *render{};
    ImFont *FontRegular{};
    SkinList *skinList{};
    SkinList *reelList{};
    SkinList *tapeList{};
    int selectedSkinIdx{};       // winamp skin idx
    std::string loadStatusStr{}; // skin loading status in settings
    Connector *connector{};
    bool loading{};
    bool *hold_toggled = nullptr;
    int *hold_value = nullptr;
    bool *power_pressed = nullptr;
    int displaySettings{};
    SettingsTab displayTab = SettingsTab::SkinOpts;

    std::string WinampSkinNewName{};
    std::string WinampSkinOldName{};
    std::string WinampCurrentSkinName{};

    ESkinVariant activeSkinVariant = EMPTY;
    int activeSettingsTab{};
    Winamp::Winamp winamp{};
    Cassette::Cassette cassette{};

    AppConfig::AppConfig *config{};
    //    int *hold_value{};
    //    bool loading{};
    int loadStatus{};
    bool onlyFont{};

    std::string needRestartFontsText = "Changes will be applied on device restart";
    bool needRestartFonts{};

    void ReloadFont() {
        loading = true;
        std::string filepath{};
        switch (activeSkinVariant) {
        case CASSETTE:
            cassette.AddFonts(FontRegular);
            break;
        case WINAMP:
            winamp.Unload();

            if (!SkinExists(config->winamp.filename, skinList, &filepath)) {
                DLOG("no skin %s found in skinlist\n", config->winamp.filename.c_str());
                exit(1);
            }

            winamp.Load(filepath, FontRegular);
            break;
        default:
            break;
        }

        config->Save();

        onlyFont = false;
        loading = false;
    }

    void Load() {
        if (onlyFont) {
            ReloadFont();
            return;
        }

        LoadNewWinampSkin();

        if (config->activeSkin == activeSkinVariant) {
            return;
        }

        if (ImGui::GetCurrentContext()->WithinFrameScope) {
            DLOG("cannot load in frame, perhaps this https://github.com/ocornut/imgui/pull/3761 ?\n");
            return;
        }

        loading = true;
        const char *defaultSkinPath;
#ifdef DESKTOP
        defaultSkinPath = "../skins/base-2.91.wsz";
#else
        defaultSkinPath = "/system/vendor/unknown321/usr/share/skins/winamp/base-2.91.wsz";
#endif

        if (config->activeSkin != CASSETTE) {
            cassette.Unload();
        }

        if (config->activeSkin != WINAMP) {
            winamp.Unload();
        }

        std::string filepath{};
        if (!SkinExists(config->winamp.filename, skinList, &filepath)) {
            DLOG("no skin %s found in skinlist\n", config->winamp.filename.c_str());
            filepath = defaultSkinPath;
        }

        switch (config->activeSkin) {
        case WINAMP:
            winamp.render = render;
            winamp.skin = (void *)this;
            winamp.WithConfig(&config->winamp);
            winamp.Load(filepath, FontRegular);
            break;
        case CASSETTE:
            cassette.reelList = reelList;
            cassette.tapeList = tapeList;
            cassette.render = render;
            cassette.skin = (void *)this;
            cassette.WithConfig(&config->cassette);
            cassette.Load("", FontRegular);
            break;
        default:
            //                winamp.Draw();
            break;
        }

        activeSkinVariant = config->activeSkin;
        activeSettingsTab = activeSkinVariant;
    }

    static __attribute__((unused)) void DisplayKeys() {
        // We iterate both legacy native range and named ImGuiKey ranges. This is a little unusual/odd but this allows
        // displaying the data for old/new backends.
        // User code should never have to go through such hoops!
        // You can generally iterate between ImGuiKey_NamedKey_BEGIN and ImGuiKey_NamedKey_END.
#ifdef IMGUI_DISABLE_OBSOLETE_KEYIO
        struct funcs {
            static bool IsLegacyNativeDupe(ImGuiKey) { return false; }
        };
        ImGuiKey start_key = ImGuiKey_NamedKey_BEGIN;
#else
        struct funcs {
            static bool IsLegacyNativeDupe(ImGuiKey key) { return key >= 0 && key < 512 && ImGui::GetIO().KeyMap[key] != -1; }
        }; // Hide Native<>ImGuiKey duplicates when both exists in the array
        auto start_key = (ImGuiKey)0;
#endif
        ImGui::SetCursorPos({0, 400});
        ImGuiIO &io = ImGui::GetIO();
        ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
        ImGui::SameLine();
        ImGui::Text("Keys down:");
        for (ImGuiKey key = start_key; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) {
            if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key))
                continue;
            ImGui::SameLine();
            ImGui::Text((key < ImGuiKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key);
        }
        ImGui::Text(
            "Keys mods: %s%s%s%s",
            io.KeyCtrl ? "CTRL " : "",
            io.KeyShift ? "SHIFT " : "",
            io.KeyAlt ? "ALT " : "",
            io.KeySuper ? "SUPER " : ""
        );
        ImGui::Text("Chars queue:");
        for (int i = 0; i < io.InputQueueCharacters.Size; i++) {
            ImWchar c = io.InputQueueCharacters[i];
            ImGui::SameLine();
            ImGui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c);
        } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.
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

    int LoadNewWinampSkin() {
        if (WinampCurrentSkinName == WinampSkinNewName) {
            return 0;
        }

        WinampSkinOldName = WinampCurrentSkinName;
        WinampCurrentSkinName = WinampSkinNewName;
        loadStatus = winamp.Load(WinampCurrentSkinName, FontRegular);
        if (loadStatus != 0) {
            WinampCurrentSkinName = WinampSkinOldName;
            WinampSkinNewName = WinampSkinOldName;
            loadStatusStr = winamp.loadStatusStr;
            return loadStatus;
        }

        loadStatusStr = "Skin loaded";

        for (const auto &v : *skinList) {
            if (WinampCurrentSkinName == v.fullPath) {
                config->winamp.filename = v.name;
                config->Save();
                break;
            }
        }

        return 0;
    }

    void Header() {
        ImGui::Indent(15.0f);
        ImGui::Text("Settings");

        char buffer[9];
        time_t rawtime;
        time(&rawtime);
        const auto timeinfo = localtime(&rawtime);
        strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);

        //        ImGui::SetCursorPosY(480.0f - ImGui::GetTextLineHeight() - 10.0f);
        ImGui::SameLine(380.0f);
        ImGui::Text("%s", buffer);

        auto fontsSize = ImGui::CalcTextSize("Fonts").x + ImGui::GetStyle().FramePadding.x * 2.f;
        auto miscSize = ImGui::CalcTextSize("Misc").x + ImGui::GetStyle().FramePadding.x * 2.f;
        auto skinSize = ImGui::CalcTextSize("Skin").x + ImGui::GetStyle().FramePadding.x * 2.f;
        auto closeSize = ImGui::CalcTextSize("Close").x + ImGui::GetStyle().FramePadding.x * 2.f;
        float offset = 15.0f;
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
            displaySettings = 0;
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
#ifdef DESKTOP
        if (ImGui::Button("Apply")) {
            onlyFont = true;
        }
#else
        if (needRestartFonts) {
            ImGui::Text("%s", needRestartFontsText.c_str());
        }
#endif
    }

    void Misc() {
        printFPS();

        ImGui::NewLine();
        if (ImGui::Checkbox("Debug", &config->debug.enabled)) {
            winamp.debug = config->debug.enabled;
            cassette.debug = config->debug.enabled;
            config->Save();
        }

#ifdef DESKTOP
        return;
#endif

        ImGui::NewLine();
        if (ImGui::Checkbox("Swap prev/next buttons", &config->misc.swapTrackButtons)) {
            config->Save();
        }

        ImGui::NewLine();
        if (ImGui::Checkbox("Huge cover art", &config->features.bigCover)) {
            config->Save();
            connector->FeatureBigCover(config->features.bigCover);
        }

        ImGui::NewLine();
        if (ImGui::Checkbox("Show time", &config->features.showTime)) {
            config->Save();
            connector->FeatureShowTime(config->features.showTime);
        }
    }

    void DrawSkin() {
        ImGui::NewLine();
        ImGui::SeparatorText("Skin:");

        ImGui::RadioButton("Winamp", &activeSettingsTab, WINAMP);
        ImGui::SameLine();
        if (ImGui::RadioButton("Cassette", &activeSettingsTab, CASSETTE)) {
            loadStatusStr = "";
        }

        ImGui::NewLine();
        if (activeSettingsTab == WINAMP) {
            if (activeSkinVariant != WINAMP) {
                if (ImGui::Button("Set as active")) {
                    config->activeSkin = WINAMP;
                    config->Save();
                }

                ImGui::NewLine();
            }
            //            if (ImGui::Button("Unload")) {
            //                winamp.Unload();
            //            }

            if (ImGui::Checkbox("Use bitmap font", &config->winamp.useBitmapFont)) {
                config->Save();
            }

            if (ImGui::Checkbox("Use bitmap font in playlist", &config->winamp.useBitmapFontInPlaylist)) {
                config->Save();
            }

            ImGui::NewLine();

            if (ImGui::BeginCombo("##", skinList->at(selectedSkinIdx).name.c_str(), ImGuiComboFlags_HeightSmall)) {
#ifndef DESKTOP
                auto clip_min = ImVec2(0, 0);
                //                auto clip_max = ImVec2(ImGui::GetCursorPosX() + ImGui::GetTextLineHeightWithSpacing() * 6.3f,
                //                                       ImGui::GetCursorPosY() + ImGui::GetTextLineHeightWithSpacing() * 5.0f);
                auto clip_max = ImVec2(800, 800);
                ImGui::PushClipRect(clip_min, clip_max, false);
#endif

                for (int n = 0; n < skinList->size(); n++) {
                    const bool is_selected = (selectedSkinIdx == n);
                    if (ImGui::Selectable(skinList->at(n).name.c_str(), is_selected)) {
                        selectedSkinIdx = n;
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::NewLine();
            if (activeSkinVariant == WINAMP) {
                if (ImGui::Button("Load skin")) {
                    WinampSkinNewName = skinList->at(selectedSkinIdx).fullPath;
                    loadStatus = 0;
                    ImGui::NewLine();
                    ImGui::Text("loading %s", skinList->at(selectedSkinIdx).name.c_str());
                }

                ImGui::NewLine();
                ImGui::Text("%s", loadStatusStr.c_str());
            }
        } else if (activeSettingsTab == CASSETTE) {

            if (activeSkinVariant != CASSETTE) {
                if (ImGui::Button("Set as active")) {
                    config->activeSkin = CASSETTE;
                    config->Save();
                }

                ImGui::NewLine();
            }

            if (ImGui::Checkbox("Randomize?", &config->cassette.randomize)) {
                config->Save();
            }

            static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                                           ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchProp;

            ImVec2 outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 7);
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
                                    cassette.SelectTape(true);
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
                                    cassette.SelectTape(true);
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                ImGui::EndTable();
            }

            if (ImGui::Button("Reset")) {
                cassette.config->Default();
                config->Save();
                cassette.SelectTape(true);
                cassette.LoadImages();
                cassette.UnloadUnused();
            }
        }
    }

    // some winamp skins are ugly with unreadable text
    // use default imgui style
    void DrawSettings() {
        if (!*render) {
            return;
        }

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
        default:
            break;
        }

        ImGui::PopFont();
    }

    void Draw() {
        if (displaySettings == 0) {
            switch (activeSkinVariant) {
            case WINAMP:
                winamp.Draw();
                break;
            case CASSETTE:
                cassette.Draw();
                break;
            default:
                winamp.Draw();
                break;
            }
        } else {
            DrawSettings();
        }
        //        DisplayKeys();
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

            if (*render == true) {
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

#endif