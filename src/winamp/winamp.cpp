#include "winamp.h"
#include "../skin.h"
#include "../unzip/unzip.cpp"
#include "../wstring.h"
#include "imgui_impl_opengl3.h"
#include <thread>

namespace Winamp {
    std::string const ColorBlack = "#000000";
    const float fontSizeBitmap = 17.0f;
    const float MarqueeTitleWidthBitmap = 585.0f;
    const float MarqueeTitleWidthFont = 446.0f;
    const int MarqueeMaxLengthBitmap = 32;

    const int blinkInterval = 1200 * 1000; // microseconds, got it via screen recording, winamp 2.95
    const int marqueeInterval = 200 * 1000;

    const float PlaylistYRegular = 337.0;
    float PlaylistY = 337.0;
    const int PlaylistTitleHeight = 58;
    const int VolumeBarCount = 28;
    const int BalanceBarCount = 28;

    const int playlistSongWidth = 600.0f;
    const char *separator = "  ***  ";
    const char *remainingTimeSignMinus = "-";
    const char *remainingTimeSignPlus = "";

#ifdef DESKTOP
    //    const std::string FontPath = "../SSTJpPro-Regular.otf";
    const std::string FontPath = "../NotoSansKR-Regular.otf";
    const std::string FontPathCustom = "../font.ttf";
    const std::string FontPathCustom2 = "../font.otf";
    const char *defaultSkinPath = "../skins/base-2.91.wsz";
#else
    const std::string FontPath = "/system/vendor/sony/lib/fonts/NotoSansKR-Regular.otf";
    const std::string FontPathCustom = "/contents/wampy/fonts/font.ttf";
    const std::string FontPathCustom2 = "/contents/wampy/fonts/font.otf";
    const char *defaultSkinPath = "/system/vendor/unknown321/usr/share/skins/winamp/base-2.91.wsz";
#endif

    const std::list<const char *> filenames = {
        "main.bmp",
        "titlebar.bmp",
        "cbuttons.bmp",
        "shufrep.bmp",
        "posbar.bmp",
        "monoster.bmp",
        "playpaus.bmp",
        "numbers.bmp",
        "nums_ex.bmp",
        "text.bmp",
        "volume.bmp",
        "viscolor.txt",
        "region.txt",
        "balance.bmp",
        "pledit.bmp",
        "pledit.txt"};

    Winamp::Winamp() = default;

    Winamp::Winamp(Winamp const &other) : SkinVariant(other) {
        // copy constructor implementation
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
    Winamp &Winamp::Winamp::operator=(Winamp const &other) {
        // copy assignment operator
    }
#pragma GCC diagnostic pop

    void Winamp::Notify() {
        statusUpdatedM.lock();
        statusUpdated = true;
        statusUpdatedM.unlock();
    }

    void Winamp::processUpdate() {
        updateThreadRunning = true;

        for (;;) {
            std::this_thread::sleep_for(std::chrono::microseconds(50 * 1000));
            if (childThreadsStop) {
                break;
            }

            if (!*render) {
                continue;
            }

            if (loading) {
                continue;
            }

            if (!statusUpdated) {
                continue;
            }

            statusUpdatedM.lock();
            statusUpdated = false;
            statusUpdatedM.unlock();

            Format();
        }

        updateThreadRunning = false;
        DLOG("update thread stopped\n");
    }

    void Winamp::AddFonts(ImFont **fontRegular) {
        auto numFontData = textures["numbers.bmp"];
        if (textures["nums_ex.bmp"].len != 0) {
            numFontData = textures["nums_ex.bmp"];
            isEx = true;
        }

        std::string fp = FontPath;
        if (exists(FontPathCustom)) {
            fp = FontPathCustom;
        }
        if (exists(FontPathCustom2)) {
            fp = FontPathCustom2;
        }

        auto res = addFont(fp, numFontData, textures["text.bmp"]);
        FontBitmap = res.bitmap;
        FontNumbers = res.number;
        FontRegular = res.regular;
        *fontRegular = res.regular;

        ImGui_ImplOpenGL3_DestroyFontsTexture();
        ImGui_ImplOpenGL3_CreateFontsTexture();

        //        fontRegular->FontSize = 25; // this will scale down elements in settings
    }

    int Winamp::volumeTextureIsBalance() {
        if (textures["balance.bmp"].len != 0) {
            return 0;
        }

        if (textures["volume.bmp"].len > 0) {
            DLOG("balance.bmp is missing, using volume.bmp as replacement\n");

            textures["balance.bmp"].data = (char *)malloc(textures["volume.bmp"].len);
            memcpy((void *)textures["balance.bmp"].data, (void *)textures["volume.bmp"].data, textures["volume.bmp"].len);
            textures["balance.bmp"].len = textures["volume.bmp"].len;
        }

        DLOG("balance.bmp and volume.bmp are missing\n");

        return 0;
    }

    void Winamp::probeTrackTitleBackgroundColor() {
        FlatTexture t;
        t.FromPair(textures["text.bmp"]);
        colors.trackTitleBackground = t.GetColor(150, 5);
        t.Release();
    }

    int Winamp::Load(std::string filename, ImFont **fontRegular) {
        loading = true;
        isEx = false;
        loadStatusStr = "";

        if (filename.empty()) {
            DLOG("invalid skin filename, using default\n");
            filename = defaultSkinPath;
        }

        DLOG("Unzip %s\n", filename.c_str());

        for (auto &f : filenames) {
            textures[f];
        }

        if (unzip(filename, &textures, &loadStatusStr) < 0) {
            loading = false;
            DLOG("failed to unzip\n");
            return -1;
        }

        if (volumeTextureIsBalance() < 0) {
            loading = false;
            return -1;
        }

        readPlEdit();
        readRegionTxt();
        readVisColorTxt();

        probeTrackTitleBackgroundColor();

        DLOG("Load textures\n");
        initializeElements();

        DLOG("Adding fonts\n");
        AddFonts(fontRegular);

        DLOG("Fonts added\n");

        loading = false;
        freeUnzippedTextures();

        if (!timeRemainingSet) {
            timeRemaining = config->preferTimeRemaining;
            timeRemainingSet = true;
        }

        StartThreads();

        currentSkin = filename;

        return 0;
    }

    void Winamp::loadNewSkin(bool force) {
        if ((currentSkin == newSkin) && !force) {
            DLOG("not updating current skin %s\n", currentSkin.c_str());
            return;
        }

        std::string oldSkin = currentSkin;

        Unload();

        if (Load(newSkin, &FontRegular) < 0) {
            DLOG("failed to load skin %s, loading previous (%s)\n", newSkin.c_str(), oldSkin.c_str());
            Unload();
            newSkin = oldSkin;
            if (Load(newSkin, &FontRegular) < 0) {
                DLOG("failed to load old skin %s during fallback, loading default skin %s\n", oldSkin.c_str(), defaultSkinPath);
                newSkin = defaultSkinPath;
                if (Load(newSkin, &FontRegular) < 0) {
                    DLOG("failed to load default skin %s, exiting\n", defaultSkinPath);
                    exit(1);
                }
            }
        }

        currentSkin = newSkin;
    }

    ImFont *Winamp::GetFont() { return FontRegular; }

    std::string Winamp::GetCurrentSkin() { return currentSkin; };

    void Winamp::changeSkin(const std::string &s) { newSkin = s; }

    int Winamp::unzip(const std::string &filename, TextureMap *textures, std::string *status) {
        int err = unzipFiles(filename.c_str(), textures);
        if (err != UNZ_OK) {
            *status = "unzip error " + std::to_string(err);
            DLOG("%s\n", status->c_str());
            return -1;
        }

        std::vector<std::string> optional = {
            "nums_ex.bmp",
            "numbers.bmp",
            "balance.bmp",
            "region.txt",
            "monoster.bmp",
            "playpaus.bmp",
            "titlebar.bmp",
            "volume.bmp",
            "viscolor.txt",
            "numbers.bmp",
        };

        // are all files present?
        for (auto const &k : *textures) {
            if (std::find(optional.begin(), optional.end(), k.first) != optional.end()) {
                continue;
            }

            if (k.second.len == 0) {
                *status = k.first + " is empty or doesn't exist";
                DLOG("%s\n", status->c_str());
                return -1;
            }
        }

        return 0;
    }

    void Winamp::freeUnzippedTextures() {
        for (auto &c : textures) {
            if (c.second.data == nullptr) {
                continue;
            }

            free(c.second.data);
        }

        textures.clear();
    }

    void Winamp::readRegionTxt() {
        if (textures["region.txt"].len == 0) {
            DLOG("region.txt missing\n");
            return;
        }

        auto text = std::string(textures["region.txt"].data, textures["region.txt"].len);

        auto lines = split(text, "\n");
        if (lines.empty()) {
            lines = split(text, "\r\n");
        }

        if (lines.empty()) {
            DLOG("no lines in region.txt found\n");
            return;
        }

        bool normalFound{};
        int numPoints{};

        for (auto &line : lines) {
            //            DLOG("line: %s\n", line.c_str());
            if (line.rfind("[Normal]", 0) == 0) {
                normalFound = true;
                continue;
            }

            if (normalFound) {
                if (line.rfind("NumPoints", 0) == 0) {
                    auto parts = split(line, "=");
                    if (parts.size() < 2) {
                        DLOG("not enough values for numpoints: %s\n", line.c_str());
                        return;
                    }
                    numPoints = std::atoi(parts[1].c_str());
                    continue;
                }

                if (numPoints > 0) {
                    if (line.rfind("PointList", 0) == 0) {
                        auto parts = split(line, "=");
                        if (parts.size() < 2) {
                            DLOG("not enough values in pointlist: %s\n", line.c_str());
                            return;
                        }

                        std::string curchar;
                        for (const auto &c : parts[1]) {
                            if (c >= '0' && c <= '9') {
                                curchar += c;
                            } else {
                                if (curchar.empty()) {
                                    continue;
                                }

                                pointList.push_back(std::atoi(curchar.c_str()));
                                curchar.clear();
                            }
                        }

                        if (!curchar.empty()) {
                            pointList.push_back(std::atoi(curchar.c_str()));
                            curchar.clear();
                        }

                        break;
                    }
                }
            }
        }

        if (numPoints != 0) {
            DLOG("found %zu numbers out of expected %d\n", pointList.size(), numPoints * 2);
            if (pointList.size() % 2 != 0) {
                DLOG("uneven point count, ignoring\n");
                pointList.clear();
            }

            if (pointList.size() != (numPoints * 2)) {
                DLOG("NumPoints and points found do not match, ignoring\n");
                pointList.clear();
            }
        }
    }

    void Winamp::readVisColorTxt() {
        visColors.clear();

        if (textures["viscolor.txt"].len == 0) {
            DLOG("viscolor.txt missing\n");
            return;
        }

        auto text = std::string(textures["viscolor.txt"].data, textures["viscolor.txt"].len);

        auto lines = split(text, "\n");
        if (lines.empty()) {
            lines = split(text, "\r\n");
        }

        if (lines.empty()) {
            DLOG("no lines in viscolor.txt found\n");
            return;
        }

        for (auto &line : lines) {
            if (line.empty()) {
                continue;
            }

            if (line.at(0) == '/') {
                continue;
            }

            //            DLOG("line: %s\n", line.c_str());

            auto parts = split(line, ",");
            if (parts.size() < 3) {
                DLOG("not enough values in line: %s\n", line.c_str());
                continue;
            }

            std::string curchar;
            int i = 0;
            int values[3]{0};

            for (const auto &part : parts) {
                for (const auto &c : part) {
                    if (c >= '0' && c <= '9') {
                        curchar += c;
                    } else {
                        if (curchar.empty()) {
                            continue;
                        }

                        values[i] = std::atoi(curchar.c_str());
                        curchar.clear();
                    }
                }

                if (!curchar.empty()) {
                    values[i] = std::atoi(curchar.c_str());
                    curchar.clear();
                }

                i++;
                if (i == 3) {
                    //                    DLOG("%d %d %d\n", values[0], values[1], values[2]);
                    visColors.emplace_back((float)values[0], (float)values[1], (float)values[2], 255.0f);
                    auto c = visColors.at(visColors.size() - 1);
                    //                    DLOG("color %f %f %f %f\n", c.x, c.y, c.z, c.w);
                    break;
                }
            }
        }

        if (visColors.size() == 24) {
            colors.VisBarPeakColor = ImVec4ToColor(visColors.at(23));
        }
    }

    void Winamp::readPlEdit() {
        auto text = std::string(textures["pledit.txt"].data, textures["pledit.txt"].len);
        colors.PlaylistNormalBG = ColorBlack;
        colors.PlaylistSelectedBG = ColorBlack;
        colors.PlaylistCurrentText = ColorBlack;
        colors.PlaylistNormalText = ColorBlack;

        for (const auto &line : split(text, "\n")) {
            auto parts = split(line, "=");
            switch (hash(parts[0].c_str())) {
            case hash("NormalBG"): {
                colors.PlaylistNormalBG = parseColor(parts[1]);
                //                    colors.PlaylistNormalBGIV = colorToImVec4(colors.PlaylistNormalBG);
                break;
            }
            case hash("SelectedBD"): {
                colors.PlaylistSelectedBG = parseColor(parts[1]);
                //                    colors.PlaylistSelectedBGIV = colorToImVec4(colors.PlaylistSelectedBG);
                break;
            }
            case hash("Normal"): {
                colors.PlaylistNormalText = parseColor(parts[1]);
                colors.PlaylistNormalTextU32 = colorToImCol32(colors.PlaylistNormalText);
                break;
            }
            case hash("Current"): {
                colors.PlaylistCurrentText = parseColor(parts[1]);
                colors.PlaylistCurrentTextU32 = colorToImCol32(colors.PlaylistCurrentText);
                break;
            }
            }
        }
    }

    void Winamp::SeekPressed(void *winampSkin, void *i) {
        auto w = (Winamp *)winampSkin;
        w->MarqueeRunning = false;

        auto val = *(int *)i;

        int totalMin = w->connector->status.Duration / 60;
        int totalSec = w->connector->status.Duration % 60;
        int newMin = ((w->connector->status.Duration * val) / 100) / 60;
        int newSec = ((w->connector->status.Duration * val) / 100) % 60;

        snprintf(w->systemMessage, PLAYLIST_SONG_SIZE, "Seek To: %02d:%02d/%02d:%02d (%d%%)", newMin, newSec, totalMin, totalSec, val);
    }

    void Winamp::SeekReleased(void *winampSkin, void *i) {
        auto w = (Winamp *)winampSkin;

        auto val = *(int *)i;
        w->connector->SetPosition(val);
        memset(w->systemMessage, 0, 256);
        w->MarqueeRunning = true;
        w->connector->status.Elapsed = w->connector->status.Duration * val / 100;
#ifndef DESKTOP
        w->connector->updateElapsedCounter = 1;
#endif
    }

    void Winamp::VolumePressed(void *winampSkin, void *i) {
#ifdef DESKTOP
        Winamp::VolumePressedMPD(winampSkin, i);
#else
        Winamp::VolumePressedHagoromo(winampSkin, i);
#endif
    }

    void Winamp::VolumePressedMPD(void *winampSkin, void *i) {
        auto w = (Winamp *)winampSkin;
        w->MarqueeRunning = false;

        auto val = *(int *)i;

        w->connector->SetVolume(val, false);

        snprintf(w->systemMessage, 256, "Volume: %d%%", val);
    }

    void Winamp::VolumePressedHagoromo(void *winampSkin, void *i) {
        auto w = (Winamp *)winampSkin;
        w->MarqueeRunning = false;

        auto val = *(int *)i;
        w->connector->updateVolume = false;

        snprintf(w->systemMessage, 256, "Volume: %d%%", val);
    }

    void Winamp::VolumeReleased(void *winampSkin, void *i) {
#ifdef DESKTOP
        Winamp::VolumeReleasedMPD(winampSkin, i);
#else
        Winamp::VolumeReleasedHagoromo(winampSkin, i);
#endif
    }

    void Winamp::VolumeReleasedMPD(void *winampSkin, void *i) {
        auto w = (Winamp *)winampSkin;
        memset(w->systemMessage, 0, 256);
        w->MarqueeRunning = true;
    }

    void Winamp::VolumeReleasedHagoromo(void *winampSkin, void *i) {
        auto w = (Winamp *)winampSkin;
        w->MarqueeRunning = true;

        auto val = *(int *)i;
        w->connector->status.Volume = val;
        w->connector->SetVolume(val, false);
        w->connector->updateVolume = true;

        memset(w->systemMessage, 0, 256);
    }

    void Winamp::BalancePressed(void *winampSkin, void *i) {
        //        notImplemented(winampSkin, i);
        /*
                     if (i == 0) {
                        format = "Balance: Center";
                    } else if (i < 0) {
                        format = "Balance: %d%% left";
                    } else {
                        format = "Balance: %d%% right";
                    }
        snprintf(w->currentSongTitleMarquee, PLAYLIST_SONG_SIZE, format, i);
        */
    }

    void Winamp::BalanceReleased(void *winampSkin, void *i) {
        //        auto w = (Winamp *)winampSkin;
        //        w->MarqueeRunning = true;
    }

    void Winamp::Draw() {
        if (playlistFullscreen) {
            drawPlaylist();
            return;
        }

        Elements.Main.Draw();
        Elements.Title.Draw();
        if (config->showClutterbar) {
            Elements.ClutterBar.Draw();
        }

        switch (connector->status.State) {
        case PlayStateE::PLAYING:
            Elements.PlayIndicator.Draw();
            Elements.BufferingIndicator.Draw();
            break;
        case PlayStateE::PAUSED:
            if (stopped) {
                Elements.StopIndicator.Draw();
            } else {
                Elements.PauseIndicator.Draw();
            }
            break;
        case PlayStateE::STOPPED:
        default:
            Elements.StopIndicator.Draw();
            break;
        }

        if (stopped) {
            Elements.StereoOffIndicator.Draw();
            Elements.MonoOffIndicator.Draw();
        } else {
            if (connector->status.Channels == 1) {
                Elements.MonoOnIndicator.Draw();
                Elements.StereoOffIndicator.Draw();
            } else {
                Elements.MonoOffIndicator.Draw();
                Elements.StereoOnIndicator.Draw();
            }
        }

        Elements.ShuffleButton.Draw(connector->status.Shuffle);
        Elements.RepeatButton.Draw(connector->status.Repeat);
        Elements.EQButton.Draw(connector->soundSettingsFw.s->anyFilterEnabled);
        Elements.PlaylistButton.Draw();
        Elements.PrevButton.Draw();
        Elements.PlayButton.Draw();
        Elements.PauseButton.Draw();
        Elements.StopButton.Draw();
        Elements.NextButton.Draw();
        Elements.EjectButton.Draw();

        if (stopped || connector->stateString == "stop") {
            // do nothing
        } else {
            Elements.PositionSlider.Draw(&connector->status.PositionPercent);
        }

        Elements.VolumeSlider.Draw(&connector->status.Volume);
        Elements.BalanceSlider.Draw(&connector->status.Balance);

        drawPlaylist();

        if (config->useBitmapFont) {
            ImGui::PushFont(FontBitmap);
            ImGui::SetCursorPos(ImVec2(323, 78)); // to 771
        } else {
            ImGui::PushFont(FontRegular);
            ImGui::PushStyleColor(ImGuiCol_Text, colors.PlaylistNormalTextU32);
            ImGui::SetCursorPos(ImVec2(323, 74));
        }

        if (strcmp(m.text, "") == 0) {
            ImGui::Text("Wampy");
        } else {
            if (systemMessage[0] == '\0') {
                ImGui::Text(m.format, &m.text[m.start]);
            } else {
                ImGui::Text("%s", systemMessage);
            }
        }

        if (!config->useBitmapFont) {
            ImGui::PopStyleColor(1);
        }

        ImGui::PopFont();

        if (!stopped) {
            ImGui::PushFont(FontBitmap);
            ImGui::SetCursorPos(ImVec2(323, 125));
            ImGui::Text("%s", connector->status.BitrateString.c_str());

            ImGui::SetCursorPos(ImVec2(455, 125));
            ImGui::Text("%s", connector->status.SampleRateString.c_str());
            ImGui::PopFont();
        }

        if (stopped || connector->stateString == "stop") {
            // do nothing
        } else {
            blinkTrackTime();
        }

        ImGui::SetCursorPos(ImVec2(105, 76));
        if (ImGui::InvisibleButton("##tracktimeToggle", ImVec2(183, 37))) {
            timeRemaining = !timeRemaining;
            DLOG("track time toggled to %d\n", timeRemaining);
            formatDuration();
        }

        ImGui::SetCursorPos(ImVec2(64, 122));
        ImGui::BeginDisabled();
        if (ImGui::InvisibleButton("##visToggle", ImVec2(230, 56))) {
            config->visualizerEnable = !config->visualizerEnable;
            DLOG("visualization toggled to %d\n", config->visualizerEnable);
            Skin::Skin::SaveConfig(skin);
            if (config->visualizerEnable) {
                connector->soundSettings.SetAnalyzer(1);
                if (config->visualizerWinampBands) {
                    connector->soundSettings.SetAnalyzerBandsWinamp();
                } else {
                    connector->soundSettings.SetAnalyzerBandsOrig();
                }
                for (auto &v : peakValues) {
                    v.first = 0;
                    v.second = 0;
                }
                for (auto &v : connector->soundSettings.peaks) {
                    v = 0;
                }
                connector->soundSettings.StartShark();
            } else {
                connector->soundSettings.SetAnalyzer(0);
            }
        }
        ImGui::EndDisabled();

        if (config->visualizerEnable) {
            drawVis();
        }

        if (config->skinTransparency) {
            Elements.RegionMask.Draw();
        }
    }

    void Winamp::drawVis() {
        if (connector->status.State != STOPPED) {
            Elements.VisBackground.DrawAt({70, 122});
        } else {
            return;
        }

        if (connector->status.State == PLAYING) {
            if (connector->soundSettings.sharkCalls == SHARK_CALLS) {
                for (auto &v : peakValues) {
                    v.first = 100;
                    v.second = 3;
                }
            }
            barUpdateFC++;
            connector->soundSettings.RefreshAnalyzerPeaks(config->visualizerSensitivity);
        }

        int hp = 0;
        int fakeVal = 0;
        bool shark = connector->soundSettings.sharkCalls > 0;

        for (int i = 0; i < PEAKS_COUNT; i++) {
            if (withFakeBars) {
                switch (i) {
                case 0:
                    if (shark) {
                        drawVisBar(i, connector->soundSettings.peaks[hp]);
                        hp++;
                    } else {
                        drawVisBar(i, connector->soundSettings.peaks[hp] * 70 / 100);
                    }
                    break;
                case 18:
                    if (shark) {
                        drawVisBar(i, connector->soundSettings.peaks[hp]);
                        hp++;
                    } else {
                        drawVisBar(i, connector->soundSettings.peaks[hp - 1] * 70 / 100);
                    }
                    break;
                case 3:
                case 6:
                case 9:
                case 12:
                case 15:
                    if (shark) {
                        drawVisBar(i, connector->soundSettings.peaks[hp]);
                        hp++;
                    } else {
                        fakeVal = int(float(connector->soundSettings.peaks[hp - 1] + connector->soundSettings.peaks[hp]) / 2 * 0.78);
                        drawVisBar(i, fakeVal);
                    }
                    break;
                default:
                    drawVisBar(i, connector->soundSettings.peaks[hp]);
                    hp++;
                    break;
                }
            } else {
                drawVisBar(i, connector->soundSettings.peaks[hp]);
                hp++;
            }
        }
    }

    void Winamp::drawVisBar(int index, int value) {
        auto coeff = (float)(100 - value) / 100;
        auto uvx = 1.0f;
        float space = 2.75f;
        float decayRate = 0.957f;
        int decayDelay = 60;
        if (connector->soundSettings.sharkCalls > 0) {
            decayDelay = SHARK_DECAY_DELAY;
            decayRate = 0.90;
        }

        if (index > 16) {
            uvx = 0.9f;
        }

        if ((float)value >= peakValues[index].first) {
            //            DLOG("value up index%d: %d %d\n", index, value, peakValues[index].first);
            peakValues[index].first = (float)value;
            peakValues[index].second = decayDelay;
        } else {
            if (peakValues[index].second > 0) {
                if (connector->status.State == PLAYING) {
                    //                    DLOG("wait for index%d: %d\n", index, peakValues[index].second);
                    peakValues[index].second--;
                }
            }
        }

        if (peakValues[index].second < 1) {
            if (connector->status.State == PLAYING) {
                //                DLOG("decay index%d:\n", index);
                peakValues[index].first *= decayRate;
                if (peakValues[index].first < 2) {
                    peakValues[index].first = 0;
                }
                if (connector->soundSettings.sharkCalls > 0) {
                    // destroy shark trail faster
                    if (peakValues[index].first < 9) {
                        peakValues[index].first = 0;
                    }
                }
            }
        }

        if (value >= 1) {
            // bar
            ImGui::SetCursorPos(
                ImVec2(70 + Elements.VisBarPeak.upscaled.width * index + space * index, 128 + Elements.VisBar.upscaled.height * coeff)
            );

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
            auto tid = (ImTextureID)Elements.VisBar.textureID;
            switch (index) {
            case 0:
            case 18:
            case 3:
            case 6:
            case 9:
            case 12:
            case 15:
                tid = (ImTextureID)Elements.FakeBar.textureID;
                break;
            default:
                break;
            }
            ImGui::Image(
                tid,
                ImVec2(Elements.VisBar.upscaled.width * uvx, Elements.VisBar.upscaled.height * (1 - coeff)),
                ImVec2(0, coeff),
                ImVec2(1 * uvx, 1)
            );
        }

        auto coeffPeak = (float)(100 - peakValues[index].first) / 100;
        int offset = 50.0f * coeffPeak - Elements.VisBarPeak.upscaled.height;
        if (offset < 0) {
            offset = 2;
        }

        if (peakValues[index].second == SHARK_DECAY_DELAY && connector->soundSettings.sharkCalls > 0) {
            peakValues[index].first -= 5;
        }

        if (offset < 46 && offset >= 0) {
            // peak
            ImGui::SetCursorPos(ImVec2(70 + Elements.VisBarPeak.upscaled.width * index + space * index, 125 + offset));

            ImGui::Image(
                (ImTextureID)Elements.VisBarPeak.textureID,
                ImVec2(Elements.VisBarPeak.upscaled.width * uvx, Elements.VisBarPeak.upscaled.height),
                ImVec2(0, 0),
                ImVec2(1 * uvx, 1)
            );
        }
#pragma GCC diagnostic pop
    }

    void Winamp::drawPlaylist() const {
        if (loading) {
            return;
        }

        if (playlistFullscreen) {
            PlaylistY = 0;
        } else {
            PlaylistY = PlaylistYRegular;
        }

        Elements.PlaylistTitleBarLeftCorner.DrawAt(Elements.PlaylistTitleBarLeftCorner.position.x, PlaylistY);
        Elements.PlaylistTitleBarFiller.DrawAt(Elements.PlaylistTitleBarFiller.position.x, PlaylistY);
        Elements.PlaylistTitleBarTitle.DrawAt(Elements.PlaylistTitleBarTitle.position.x, PlaylistY);
        Elements.PlaylistTitleBarFiller.DrawAt((float)73 + 183 + 290 - 2, PlaylistY);
        Elements.PlaylistTitleBarRightCornerButton.DrawAt(Elements.PlaylistTitleBarRightCornerButton.position.x, PlaylistY);
        Elements.PlaylistLeftBorder.DrawAt(Elements.PlaylistLeftBorder.position.x, PlaylistY + PlaylistTitleHeight);
        Elements.PlaylistRightBorder.DrawAt(Elements.PlaylistRightBorder.position.x, PlaylistY + PlaylistTitleHeight);
        Elements.PlaylistScrollButton.DrawAt(Elements.PlaylistScrollButton.position.x, PlaylistY + PlaylistTitleHeight);
        Elements.PlaylistBG.DrawAt(Elements.PlaylistBG.position.x, PlaylistY + PlaylistTitleHeight);

        if (config->useBitmapFontInPlaylist) {
            ImGui::PushFont(FontBitmap);
        } else {
            ImGui::PushFont(FontRegular);
        }

        for (int i = 0; i < PLAYLIST_SIZE; i++) {
            if (!playlistFullscreen) {
                if (i > 2) {
                    break;
                }
            }

            auto s = playlist[i];

            if (i == 0) {
                ImGui::PushStyleColor(ImGuiCol_Text, colors.PlaylistCurrentTextU32);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, colors.PlaylistNormalTextU32);
            }

            auto plY = PlaylistY + 60 + ((float)i * 30);
            if (config->useBitmapFontInPlaylist) {
                plY = PlaylistY + 65 + ((float)i * 30);
            }

            ImGui::SetCursorPos(ImVec2(38, plY));
            ImGui::Text("%s", s.text);

            ImGui::SetCursorPos(ImVec2(800 - s.durationSize - 60, plY));
            ImGui::Text("%s", s.duration);
            ImGui::PopStyleColor();
        }

        ImGui::PopFont();
    }

    void Winamp::initializeElements() {
        Elements.Main.FromPair(textures["main.bmp"])
            ->WithCrop(Magick::RectangleInfo{275, 116, 0, 0})
            ->WithFilledRectangle({770, 96, 323, 78}, colors.trackTitleBackground)
            ->Load();
        Elements.RegionMask.FromPointList(pointList);

        if (textures["titlebar.bmp"].len > 0) {
            Elements.Title.FromPair(textures["titlebar.bmp"])->WithCrop(Magick::RectangleInfo{275, 14, 27, 0})->Load();
            Elements.ClutterBar.FromPair(textures["titlebar.bmp"])
                ->WithCrop(Magick::RectangleInfo{8, 43, 304, 0})
                ->WithPosition(ImVec2(29.0f, 64.0f))
                ->Load();
        }

        if (textures["monoster.bmp"].len > 0) {
            Elements.MonoOffIndicator.FromPair(textures["monoster.bmp"])
                ->WithCrop({0, 12, 29, 12})
                ->WithPosition(ImVec2(615.0f, 119.0f))
                ->WithScale({86, 35}, true)
                ->Load();
            Elements.MonoOnIndicator.FromPair(textures["monoster.bmp"])
                ->WithCrop({0, 12, 29, 0}) // width should be 29, but some skins have less width
                ->WithPosition(ImVec2(615.0f, 119.0f))
                ->WithScale({86, 35}, true)
                ->Load();
            Elements.StereoOnIndicator.FromPair(textures["monoster.bmp"])
                ->WithCrop({29, 12, 0, 0})
                ->WithPosition(ImVec2(695.0f, 119.0f))
                ->WithScale({85, 35}, false)
                ->Load();
            Elements.StereoOffIndicator.FromPair(textures["monoster.bmp"])
                ->WithCrop({29, 12, 0, 12})
                ->WithPosition(ImVec2(695.0f, 119.0f))
                ->WithScale({85, 35}, false)
                ->Load();
        }

        if (textures["playpaus.bmp"].len > 0) {
            Elements.StopIndicator.FromPair(textures["playpaus.bmp"])->WithCrop({9, 9, 18, 0})->WithPosition(ImVec2(79.0f, 81.0f))->Load();
            Elements.PlayIndicator.FromPair(textures["playpaus.bmp"])->WithCrop({9, 9, 0, 0})->WithPosition(ImVec2(79.0f, 81.0f))->Load();
            Elements.PauseIndicator.FromPair(textures["playpaus.bmp"])->WithCrop({9, 9, 9, 0})->WithPosition(ImVec2(79.0f, 81.0f))->Load();
            Elements.BufferingIndicator.FromPair(textures["playpaus.bmp"])
                ->WithCrop({3, 9, 36, 0})
                ->WithPosition(ImVec2(70.0f, 81.0f))
                ->WithScale({8, 24}, false)
                ->Load();
        }

        this->initializeButtons();
        this->initializePlaylist();
        this->initializeSliders();
        this->createVisBar();
        this->createFakeBar();
        this->createVisBarPeak();
        this->createVisBG();
    }

    void Winamp::initializePlaylist() {
        Elements.PlaylistTitleBarLeftCorner.FromPair(textures["pledit.bmp"])
            ->WithCrop({25, 20, 0, 0})
            ->WithScale({73, 58}, false)
            ->WithPosition(ImVec2(0.0f, PlaylistY))
            ->Load();
        Elements.PlaylistTitleBarFiller.FromPair(textures["pledit.bmp"])
            ->WithCrop({25, 20, 127, 0})
            ->WithScale({184, 58}, true)
            ->WithPosition(ImVec2(72.0f, PlaylistY))
            ->Load();
        Elements.PlaylistTitleBarTitle.FromPair(textures["pledit.bmp"])
            ->WithCrop({100, 20, 26, 0})
            ->WithScale({290, 58}, false)
            ->WithPosition(ImVec2(254.0f, PlaylistY))
            ->Load();
        Elements.PlaylistLeftBorder.FromPair(textures["pledit.bmp"])
            ->WithCrop({25, 29, 0, 42})
            ->WithScale({73, 480 - 58}, false)
            ->WithPosition(ImVec2(0.0f, PlaylistY + 58))
            ->Load();
        Elements.PlaylistRightBorder.FromPair(textures["pledit.bmp"])
            ->WithCrop({25, 29, 26, 42})
            ->WithScale({72, 480 - 58}, false)
            ->WithPosition(ImVec2(800.0f - 72.0f, PlaylistY + 58))
            ->Load();
        Elements.PlaylistScrollButton.FromPair(textures["pledit.bmp"])
            ->WithCrop({8, 18, 52, 53})
            ->WithPosition(ImVec2(800 - 44, PlaylistY + 58))
            ->Load();

        Elements.PlaylistBG.FromColor({800 - 35 - 58, 480, 0, 0}, Magick::Color(colors.PlaylistNormalBG));
        Elements.PlaylistBG.WithPosition(ImVec2(35.0f, PlaylistY + 58.0f));

        //=========
        ImGui::ButtonTextures bts;
        FlatTexture flatTexture;
        ImGui::ButtonTexture bt;
        bt.active = flatTexture.FromPair(textures["pledit.bmp"])->WithCrop({25, 20, 153, 0})->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();
        bt.pressed = bt.active;
        bts[0] = bt;
        bts[1] = bt;
        Elements.PlaylistTitleBarRightCornerButton.WithPosition(800 - bt.size.x, PlaylistY)
            ->WithTextures(bts)
            ->WithID("playlistTitle")
            ->WithCallback(Winamp::togglePlaylistFullscreen, this, nullptr);
    }

    void Winamp::initializeButtons() {
        FlatTexture flatTexture;
        ImGui::ButtonTexture bt;
        ImGui::ButtonTextures bts;
        bts.clear();

        bt.active = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({46, 15, 28, 0})->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();

        bt.pressed = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({46, 15, 28, 15})->Load();
        flatTexture.Reset();

        bts[0] = bt;

        bt.active = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({46, 15, 28, 30})->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();

        bt.pressed = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({46, 15, 28, 45})->Load();
        flatTexture.Reset();

        bts[1] = bt;

        Elements.ShuffleButton.WithPosition(477.0f, 259.0f)
            ->WithTextures(bts)
            ->WithID("shuffle")
            ->WithCallback(Connector::ToggleShuffle, connector, (void *)&connector->status.Shuffle);

        //=========
        bts.clear();
        flatTexture.Reset();
        bt.active = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({23, 12, 0, 61})->WithScale({67, 35}, false)->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();
        bt.pressed = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({23, 12, 46, 61})->WithScale({67, 35}, false)->Load();
        bts[0] = bt;

        flatTexture.Reset();
        bt.active = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({23, 12, 0, 73})->WithScale({67, 35}, false)->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();
        bt.pressed = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({23, 12, 46, 73})->WithScale({67, 35}, false)->Load();
        bts[1] = bt;
        Elements.EQButton.WithPosition(637.0f, 168.0f)
            ->WithTextures(bts)
            ->WithCallback(Skin::Skin::ToggleDrawEQTab, skin, nullptr)
            ->WithID("eq");

        //=========
        bts.clear();
        flatTexture.Reset();
        bt.active = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({23, 12, 23, 61})->WithScale({67, 35}, false)->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();
        bt.pressed = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({23, 12, 23 * 3, 61})->WithScale({67, 35}, false)->Load();
        bts[0] = bt;

        flatTexture.Reset();
        bt.active = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({23, 12, 23, 73})->WithScale({67, 35}, false)->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();
        bt.pressed = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({23, 12, 23 * 3, 73})->WithScale({67, 35}, false)->Load();
        bts[1] = bt;
        Elements.PlaylistButton.WithPosition(704.0f, 168.0f)
            ->WithTextures(bts)
            ->WithCallback(Winamp::togglePlaylistFullscreen, this, nullptr)
            ->WithID("playlist");

        //=========
        bts.clear();
        flatTexture.Reset();
        bt.active = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({28, 15, 0, 0})->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();
        bt.pressed = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({28, 15, 0, 15})->Load();
        bts[0] = bt;

        flatTexture.Reset();
        bt.active = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({28, 15, 0, 30})->WithScale({81 + 1, 43}, false)->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();
        bt.pressed = flatTexture.FromPair(textures["shufrep.bmp"])->WithCrop({28, 15, 0, 45})->WithScale({81 + 1, 43}, false)->Load();
        bts[1] = bt;
        bts[2] = bt; // hagoromo only
        Elements.RepeatButton.WithPosition(610.0f, 259.0f)
            ->WithTextures(bts)
            ->WithID("repeat")
            ->WithCallback(Connector::SetRepeat, connector, (void *)&connector->status.Repeat);

        //=========
        bts.clear();
        flatTexture.Reset();
        bt.active = flatTexture.FromPair(textures["cbuttons.bmp"])->WithCrop({23, 18, 0, 0})->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();
        bt.pressed = flatTexture.FromPair(textures["cbuttons.bmp"])->WithCrop({23, 18, 0, 18})->Load();
        bts[0] = bt;
        Elements.PrevButton.WithPosition(47.0f, 256.0f)->WithTextures(bts)->WithID("prev")->WithCallback(Winamp::Prev, this, nullptr);

        auto buttonWidth = 47 + (int)Elements.PrevButton.textures[0].size.x;

        //=========
        bts.clear();
        flatTexture.Reset();
        bt.active = flatTexture.FromPair(textures["cbuttons.bmp"])->WithCrop({23, 18, 23, 0})->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();
        bt.pressed = flatTexture.FromPair(textures["cbuttons.bmp"])->WithCrop({23, 18, 23, 18})->Load();
        bts[0] = bt;
        Elements.PlayButton.WithPosition((float)buttonWidth, 256.0f)
            ->WithTextures(bts)
            ->WithID("play")
            ->WithCallback(Winamp::Play, this, nullptr);

        buttonWidth += (int)Elements.PlayButton.textures[0].size.x;

        //=========
        bts.clear();
        flatTexture.Reset();
        bt.active = flatTexture.FromPair(textures["cbuttons.bmp"])->WithCrop({23, 18, 23 * 2, 0})->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();
        bt.pressed = flatTexture.FromPair(textures["cbuttons.bmp"])->WithCrop({23, 18, 23 * 2, 18})->Load();
        bts[0] = bt;
        Elements.PauseButton.WithPosition((float)(buttonWidth), 256.0f)
            ->WithTextures(bts)
            ->WithID("pause")
            ->WithCallback(Winamp::Pause, this, nullptr);
        buttonWidth += (int)Elements.PauseButton.textures[0].size.x;

        //=========
        bts.clear();
        flatTexture.Reset();
        bt.active = flatTexture.FromPair(textures["cbuttons.bmp"])->WithCrop({23, 18, 23 * 3, 0})->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();
        bt.pressed = flatTexture.FromPair(textures["cbuttons.bmp"])->WithCrop({23, 18, 23 * 3, 18})->Load();
        bts[0] = bt;
        Elements.StopButton.WithPosition((float)(buttonWidth), 256.0f)
            ->WithTextures(bts)
            ->WithID("stop")
            ->WithCallback(Winamp::Stop, this, nullptr);
        buttonWidth += (int)Elements.StopButton.textures[0].size.x;

        //=========
        // next button is smaller than others
        // scaling isn't discrete, grow button to regular size plus 1 pixel to fully cover buttons area
        bts.clear();
        flatTexture.Reset();
        bt.active =
            flatTexture.FromPair(textures["cbuttons.bmp"])
                ->WithCrop({22, 18, 23 * 4, 0})
                ->WithScale({(size_t)Elements.StopButton.textures[0].size.x + 1, (size_t)Elements.StopButton.textures[0].size.y}, false)
                ->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();
        bt.pressed =
            flatTexture.FromPair(textures["cbuttons.bmp"])
                ->WithCrop({22, 18, 23 * 4, 18})
                ->WithScale({(size_t)Elements.StopButton.textures[0].size.x + 1, (size_t)Elements.StopButton.textures[0].size.y}, false)
                ->Load();
        bts[0] = bt;
        Elements.NextButton.WithPosition((float)(buttonWidth), 256.0f)
            ->WithTextures(bts)
            ->WithID("next")
            ->WithCallback(Winamp::Next, this, nullptr);

        //=========
        bts.clear();
        flatTexture.Reset();
        bt.active = flatTexture.FromPair(textures["cbuttons.bmp"])->WithCrop({22, 16, 114, 0})->Load();
        bt.size = flatTexture.GetSize();
        flatTexture.Reset();
        bt.pressed = flatTexture.FromPair(textures["cbuttons.bmp"])->WithCrop({22, 16, 114, 16})->Load();
        bts[0] = bt;
        Elements.EjectButton.WithPosition(396.0f, 259.0f)
            ->WithTextures(bts)
            ->WithCallback(Skin::Skin::ToggleDrawSettings, skin, nullptr)
            ->WithID("eject");
    }

    void Winamp::initializeSliders() {
        ImGui::SliderBarTextures barTs;
        ImGui::ButtonTextures butTs; // ;^)
        FlatTexture ft;
        ImGui::SliderBarTexture barT;
        ImGui::ButtonTexture butT;

        barTs.clear();
        butTs.clear();

        barT.textureId = ft.FromPair(textures["posbar.bmp"])->WithCrop({248, 10, 0, 0})->Load();
        barT.size = ft.GetSize();
        barTs[0] = barT;

        ft.Reset();
        butT.active = ft.FromPair(textures["posbar.bmp"])->WithCrop({29, 10, 248, 0})->Load();
        butT.size = ft.GetSize();
        ft.Reset();

        butT.pressed = ft.FromPair(textures["posbar.bmp"])->WithCrop({29, 10, 248 + 29 + 1, 0})->Load();

        butTs[0] = butT;

        Elements.PositionSlider.WithID("position")
            ->WithLimits(0, 100)
            ->WithBarTextures(barTs)
            ->WithButtonTextures(butTs)
            ->WithPosition(47.0f, 209.0f)
            ->WithCallbackPressed(Winamp::SeekPressed, this)
            ->WithCallbackReleased(Winamp::SeekReleased, this);

        //=========
        butTs.clear();
        barTs.clear();
        ft.Reset();

        if (textures["volume.bmp"].len == 0) {
            butT.active = ft.FromColor({40, 31}, {255.0f, 0.0f, 0.0f, 0.0f});
        } else {
            butT.active = ft.FromPair(textures["volume.bmp"])->WithCrop({14, 11, 15, 422})->WithScale({40, 31}, false)->Load();
        }

        ft.Reset();
        if (textures["volume.bmp"].len == 0) {
            butT.pressed = ft.FromColor({40, 31}, {255.0f, 0.0f, 0.0f, 0.0f});
        } else {
            butT.pressed = ft.FromPair(textures["volume.bmp"])->WithCrop({14, 11, 0, 422})->WithScale({40, 31}, false)->Load();
        }
        butT.size = ft.GetSize();
        butTs[0] = butT;

        for (int i = 0; i < VolumeBarCount; i++) {
            ft.Reset();

            butT.pressed = ft.FromColor({40, 31}, {255.0f, 0.0f, 0.0f, 0.0f});
            if (textures["volume.bmp"].len == 0) {
                barT.textureId = ft.FromColor({198, 37}, {255.0f, 0.0f, 0.0f, 0.0f});
            } else {
                barT.textureId = ft.FromPair(textures["volume.bmp"])->WithCrop({68, 13, 0, i * 15})->WithScale({198, 37}, true)->Load();
            }
            barT.size = ft.GetSize();
            barTs[i] = barT;
        }

        Elements.VolumeSlider.WithID("volume")
            ->WithLimits(0, 100)
            ->WithBarTextures(barTs)
            ->WithButtonTextures(butTs)
            ->WithPosition(311.0f, 166.0f)
            ->WithCallbackPressed(Winamp::VolumePressed, this)
            ->WithCallbackReleased(Winamp::VolumeReleased, this);

        //=========
        barTs.clear();
        butTs.clear();

        ft.Reset();
        if (textures["balance.bmp"].len > 0) {
            butT.active = ft.FromPair(textures["balance.bmp"])->WithCrop({14, 11, 15, 422})->WithScale({40, 31}, false)->Load();
        } else {
            butT.active = ft.FromColor({40, 31}, {255.0f, 0.0f, 0.0f, 0.0f});
        }
        ft.Reset();
        if (textures["balance.bmp"].len > 0) {
            butT.pressed = ft.FromPair(textures["balance.bmp"])->WithCrop({14, 11, 0, 422})->WithScale({40, 31}, false)->Load();
        } else {
            butT.pressed = ft.FromColor({40, 31}, {255.0f, 0.0f, 0.0f, 0.0f});
        }
        butT.size = ft.GetSize();
        butTs[0] = butT;
        ft.Reset();

        for (int i = 0; i < BalanceBarCount; i++) {
            ft.Reset();
            if (textures["balance.bmp"].len > 0) {
                barT.textureId = ft.FromPair(textures["balance.bmp"])->WithCrop({38, 13, 9, i * 14 + i})->Load();
            } else {
                barT.textureId = ft.FromColor({38, 13}, {255.0f, 0.0f, 0.0f, 0.0f});
            }
            barT.size = ft.GetSize();
            barTs[i] = barT;
            if (i != 0) {
                barTs[-i] = barT;
            }
        }

        Elements.BalanceSlider.WithID("balance")
            ->WithLimits(-100, 100)
            ->WithBarTextures(barTs)
            ->WithButtonTextures(butTs)
            ->WithPosition(515.0f, 166.0f)
            ->WithCallbackReleased(Winamp::BalancePressed, this)
            ->WithCallbackReleased(Winamp::BalanceReleased, this);
    }

    void Winamp::createVisBar() {
        std::vector<std::string> barColors{};
        barColors.reserve(visColors.size());
        for (auto q : visColors) {
            barColors.emplace_back(ImVec4ToColor(q));
        }
        Elements.VisBar.BarFromColors({9, 44, 0, 0}, barColors);
        //        Elements.VisBar.FromColor({9, 44, 0, 0}, Magick::Color(colors.VisBarColor));
    }

    void Winamp::createFakeBar() {
        //        Elements.FakeBar.FromColor({9, 44, 0, 0}, Magick::Color("#ff0000"));
        Elements.FakeBar.textureID = Elements.VisBar.textureID;
    }

    void Winamp::createVisBarPeak() { Elements.VisBarPeak.FromColor({9, 3, 0, 0}, Magick::Color(colors.VisBarPeakColor)); }

    void Winamp::createVisBG() {
        Magick::Geometry size(224, 50, 0, 0);
        if (visColors.size() < 2) {
            Elements.VisBackground.FromColor(size, Magick::Color("#00000000"));
            return;
        }

        if (visColors.at(0) == visColors.at(1)) {
            Elements.VisBackground.FromColor(size, Magick::Color(ImVec4ToColor(visColors.at(0))));
            return;
        } else {
            // instead of drawing grid just make it transparent
            Elements.VisBackground.FromColor(size, Magick::Color("#00000000"));
        }
    }

    void Winamp::notImplemented(void *winampSkin, void *) {
        //        auto w = (Winamp *)winampSkin;
        //        w->MarqueeRunning = false;
    }

    void Winamp::drawTime() {
        auto s = playlist[0];
        if (connector->status.Duration < 0) {
            return;
        }

        if (stopped) {
            return;
        }

        ImGui::PushFont(FontNumbers);

        // minus sign
        if (timeRemaining) {
            if (isEx) {
                ImGui::SetCursorPos(ImVec2(111, 76));
            } else {
                ImGui::SetCursorPos(ImVec2(113, 93));
            }
            ImGui::Text("%s", remainingTimeSign);
        }

        ImGui::SetCursorPos(ImVec2(140, 76));
        ImGui::Text("%d", minute1);

        ImGui::SetCursorPos(ImVec2(175, 76));
        ImGui::Text("%d", minute2);

        ImGui::SetCursorPos(ImVec2(227, 76));
        ImGui::Text("%d", second1);

        ImGui::SetCursorPos(ImVec2(262, 76));
        ImGui::Text("%d", second2);
        ImGui::PopFont();
    }

    void Winamp::blinkTrackTime() {
        if (connector->status.State == PlayStateE::PAUSED) {
            if (!stopwatch.Running()) {
                stopwatch.Start();
                drawTime();
                return;
            }

            uint elapsed = stopwatch.elapsed_time<unsigned int, std::chrono::microseconds>();
            if (elapsed > blinkInterval && elapsed < blinkInterval * 2) {
                return;
            }

            if (stopwatch.elapsed_time<unsigned int, std::chrono::microseconds>() > blinkInterval * 2) {
                stopwatch.Stop();
                return;
            }
        }

        drawTime();
    }

    void Winamp::togglePlaylistFullscreen(void *arg, void *) {
        auto *w = (Winamp *)arg;
        if (w->playlistFullscreen) {
            w->playlistFullscreen = false;
        } else {
            w->playlistFullscreen = true;
        }
    }

    void Winamp::MarqueeLoop() {
        MarqueeRunning = true;
        marqueeThreadRunning = true;

        for (;;) {
            std::this_thread::sleep_for(std::chrono::microseconds(marqueeInterval));
            if (childThreadsStop) {
                break;
            }

            if (!titleIsMarquee) {
                continue;
            }

            if (!MarqueeRunning) {
                continue;
            }

            if (!*render) {
                continue;
            }

            if (loading) {
                continue;
            }

            if (!connector->stateString.empty()) {
                snprintf(systemMessage, 256, "%s", connector->stateString.c_str());
            }

            MarqueeCalculate();
        }

        marqueeThreadRunning = false;
        MarqueeRunning = false;
        DLOG("marquee thread stopped\n");
    }

    void Winamp::StartThreads() {
        childThreadsStop = false;

        auto exec = [this]() { MarqueeLoop(); };
        std::thread t(exec);
        t.detach();

        auto update = [this]() { processUpdate(); };
        std::thread v(update);
        v.detach();
    }

    void Winamp::Stop(void *winamp, void *) {
        auto w = (Winamp *)winamp;
        w->stopped = true;
        Connector::Stop(w->connector, nullptr);
    }

    void Winamp::Play(void *winamp, void *) {
        auto w = (Winamp *)winamp;
        w->stopped = false;
        DLOG("state %d\n", w->connector->status.State);
        if (w->connector->status.State == PlayStateE::PLAYING) {
            Connector::SetPosition(w->connector, 0);
            Connector::Play(w->connector, nullptr);
            return;
        }

        Connector::Play(w->connector, nullptr);
    }

    void Winamp::Pause(void *winamp, void *) {
        auto w = (Winamp *)winamp;
        if (w->stopped) {
            return;
        }

        Connector::Pause(w->connector, nullptr);
    }

    void Winamp::Prev(void *winamp, void *) {
        auto w = (Winamp *)winamp;
        if (w->stopped) {
            Connector::Prev(w->connector, nullptr);
            return;
        }

        w->stopped = false;

        if (w->connector->status.State == PlayStateE::PAUSED) {
            Connector::SetPosition(w->connector, 0);
            Connector::Prev(w->connector, nullptr);
            return;
        }

        if (w->connector->status.State == PlayStateE::PLAYING) {
            Connector::Pause(w->connector, nullptr);
            Connector::SetPosition(w->connector, 0);
            Connector::Prev(w->connector, nullptr);
            Connector::Play(w->connector, nullptr);
        }
    }

    void Winamp::Next(void *winamp, void *) {
        auto w = (Winamp *)winamp;
        if (w->stopped) {
            Connector::Next(w->connector, nullptr);
            return;
        }

        w->stopped = false;
        Connector::Next(w->connector, nullptr);
        Connector::Play(w->connector, nullptr);
    }

    void elements::Unload() {
        Main.Unload();
        Title.Unload();
        if (RegionMask.textureID > 0) {
            RegionMask.Unload();
            RegionMask.Reset();
        }
        VisBar.Unload();
        FakeBar.Unload();
        VisBarPeak.Unload();
        VisBackground.Unload();

        ClutterBar.Unload();
        MonoOffIndicator.Unload();
        MonoOnIndicator.Unload();
        StereoOnIndicator.Unload();
        StereoOffIndicator.Unload();

        StopIndicator.Unload();
        PlayIndicator.Unload();
        PauseIndicator.Unload();
        BufferingIndicator.Unload();

        ShuffleButton.Unload();
        EQButton.Unload();
        PlaylistButton.Unload();
        RepeatButton.Unload();
        PrevButton.Unload();
        PlayButton.Unload();
        PauseButton.Unload();
        StopButton.Unload();
        NextButton.Unload();
        EjectButton.Unload();

        PlaylistTitleBarLeftCorner.Unload();
        PlaylistTitleBarFiller.Unload();
        PlaylistTitleBarTitle.Unload();
        PlaylistLeftBorder.Unload();
        PlaylistRightBorder.Unload();
        PlaylistScrollButton.Unload();
        PlaylistBG.Unload();

        PlaylistTitleBarRightCornerButton.Unload();

        PositionSlider.Unload();
        VolumeSlider.Unload();
        BalanceSlider.Unload();
    }

    void Winamp::Unload() {
        Elements.Unload();
        childThreadsStop = true;
        pointList.clear();
        while (marqueeThreadRunning || updateThreadRunning) {
            // wait for threads to stop
        }

        memset(m.title, 0, PLAYLIST_SONG_SIZE);
        memset(m.text, 0, PLAYLIST_SONG_SIZE);
        memset(m.format, 0, PLAYLIST_SONG_SIZE);
        m.start = 0;

        memset(mStaging.title, 0, PLAYLIST_SONG_SIZE);
        memset(mStaging.text, 0, PLAYLIST_SONG_SIZE);
        memset(mStaging.format, 0, PLAYLIST_SONG_SIZE);
        mStaging.start = 0;

        ImGui_ImplOpenGL3_DestroyFontsTexture();
    }

    void Winamp::WithConfig(Config *c) { config = c; }

    void Config::Default() {
        useBitmapFont = true;
        useBitmapFontInPlaylist = false;
        preferTimeRemaining = false;
        showClutterbar = true;
        skinTransparency = true;
        filename = "base-2.91.wsz";
        visualizerWinampBands = false;
        visualizerEnable = false;
        visualizerSensitivity = 0.03;
    }

    Config Config::GetDefault() {
        auto c = Config{};
        c.Default();
        return c;
    }

    void Winamp::formatDuration() {
        auto status = &connector->status;
        if (status->Duration > 0) {
            status->PositionPercent = (int)std::ceil(float(status->Elapsed) / float(status->Duration) * 100);
        } else if (status->Duration == 0) {
            minute1, minute2, second1, second2 = 0;
            return;
        } else {
            return;
        }

        int minutes, seconds;
        if (timeRemaining) {
            minutes = (status->Duration - status->Elapsed) / 60;
            seconds = (status->Duration - status->Elapsed) % 60;
            remainingTimeSign = remainingTimeSignMinus;
        } else {
            minutes = status->Elapsed / 60;
            seconds = status->Elapsed % 60;
            remainingTimeSign = remainingTimeSignPlus;
        }

        minute1 = minutes / 10;
        minute2 = minutes % 10;
        second1 = seconds / 10;
        second2 = seconds % 10;
    }

    void Winamp::formatPlaylist() {
        DLOG("formatting\n");

        // playlist songs, crop to playlist width
        int i;
        for (i = 0; i < PLAYLIST_SIZE; i++) {
            auto song = connector->playlist.at(i);
            //            DLOG("%s %s\n", song.Title.c_str(), song.Artist.c_str());
            if (song.Duration < 1) {
                auto d = &playlist[i];
                memset(d->text, 0, PLAYLIST_SONG_SIZE);
                memset(d->duration, 0, PLAYLIST_DURATION_SIZE);
                d->durationSize = 0;
                continue;
            }

            auto d = &playlist[i];
            memset(d->text, 0, PLAYLIST_SONG_SIZE);
            snprintf(d->text, PLAYLIST_SONG_SIZE, "%s. %s - %s", song.Track.c_str(), song.Artist.c_str(), song.Title.c_str());

            ImFont *f = FontRegular;
            if (config->useBitmapFontInPlaylist) {
                f = FontBitmap;
            }

            CropTextToWidth(d->text, f, f->FontSize, playlistSongWidth);
            snprintf(d->duration, PLAYLIST_DURATION_SIZE, "%d:%02d", song.Duration / 60, song.Duration % 60);
            d->durationSize = f->CalcTextSizeA(f->FontSize, FLT_MAX, -1.0, d->duration).x;
        }

        for (; i < PLAYLIST_SIZE; i++) {
            auto d = &playlist[i];
            memset(d->text, 0, PLAYLIST_SONG_SIZE);
            memset(d->duration, 0, PLAYLIST_DURATION_SIZE);
            d->durationSize = 0;
        }
    }

    void Winamp::prepareForMarquee() {
        /* it is possible to make nice marquee without duplicating string when using fixed-width (in characters) window
         like this:
             if (overflow) print ("%s  ***  %s", string[pos:end], string [start:overflow])
         not so nice with ttf:
             if (overflow) {
                leftoverWidth = maxWidth - calcTextSize(maxWidth, text[pos]);
                calcTextSize(leftoverWidth, remainder);
                print("%s *** %s", text[pos], text[0:remainder])
             }
         don't want to calc twice
        */

        mStaging.start = 0;
        // is title long enough for marquee?
        if (config->useBitmapFont) {
            if (utfLen(mStaging.title) > MarqueeMaxLengthBitmap) {
                memset(mStaging.format, 0, sizeof(mStaging.format));
                snprintf(mStaging.text, PLAYLIST_SONG_SIZE, "%s%s%s%s", mStaging.title, separator, mStaging.title, separator);
                titleIsMarquee = true;
            } else {
                titleIsMarquee = false;
                m.start = 0;
                strcpy(m.format, "%s");
                strcpy(m.title, mStaging.title);
                snprintf(m.text, PLAYLIST_SONG_SIZE, "%s", mStaging.title);
            }
        } else {
            float sizeBackup = FontRegular->FontSize;
            if (FontRegular->FontSize != fontSizeTTF) {
                FontRegular->FontSize = fontSizeTTF;
            }
            ImVec2 size = FontRegular->CalcTextSizeA(fontSizeTTF, FLT_MAX, -1.0f, mStaging.title, nullptr, nullptr);
            FontRegular->FontSize = sizeBackup;

            if (size.x > MarqueeTitleWidthFont) {
                memset(mStaging.format, 0, sizeof(mStaging.format));
                snprintf(mStaging.text, PLAYLIST_SONG_SIZE, "%s%s%s%s", mStaging.title, separator, mStaging.title, separator);
                titleIsMarquee = true;
            } else {
                titleIsMarquee = false;
                m.start = 0;
                strcpy(m.format, "%s");
                strcpy(m.title, mStaging.title);
                snprintf(m.text, PLAYLIST_SONG_SIZE, "%s", mStaging.title);
            }
        }

        //        DLOG("bitmap: %d, need marquee: %d\n", config->useBitmapFont, titleIsMarquee);
        mStaging.updated = true;
    }

    void Winamp::Format(bool force) {
        if (loading) {
            return;
        }

        auto n = connector->playlist.at(0);
        if (n.Duration < 1) {
            return;
        }

        formatDuration();

        char currentSongTitleTemp[PLAYLIST_SONG_SIZE]{};
        snprintf(
            currentSongTitleTemp,
            PLAYLIST_SONG_SIZE,
            "%s. %s - %s (%d:%02d)",
            n.Track.c_str(),
            n.Artist.c_str(),
            n.Title.c_str(),
            n.Duration / 60,
            n.Duration % 60
        );

        if (strcmp(m.title, currentSongTitleTemp) == 0) {
            if (!force) {
                //                DLOG("not updating current song\n");
                return;
            }
        }

        //        DLOG("small title %s, cst: %s\n", m.title, currentSongTitleTemp);

        formatPlaylist();

        strcpy(mStaging.title, currentSongTitleTemp);

        prepareForMarquee();
    }

    // File: '/tmp/font/Untitled1.ttf' (1516 bytes)
    // Exported using binary_to_compressed_c.cpp
    static const char empty_compressed_data_base85[1295 + 1] =
        "7])#######kBglo'/###[),##1xL$#Q6>##e@;*>5Rlv1_RUV$cf>11fY;99)M7%#<PUV$@;Bk0'Tx-3lR$$/"
        "7+:GD=RopAqV,-GuS05ST;%2heC3_5<gXG-%eU`N6qF/(D6@?$%J%/G"
        "T_Ae?LUC;Z:6jW%,5LsCqTAM6V@A&,1;('>b';9CBi]EWi^]ooU2cD4?8;X-B4e'&koTS.P-2X.;h^`Iigrr-B:o:d>Z+q&o6Q<BLDhx=oXiJ):6jW%0iR/"
        "GNR4>5-$D2:ReK?-qmnUC"
        "h:jg3e=c'G;K6)*@C=GHco?D3U.xfLK&kB-N#JjLD3+5CmMxs'alVP&4[Rq.q3i@a<QQb%O<p$Db$P(./oUxL]em##0lA,MH:-##.:)=-3>iT.VH0)3:_Y-M?x/"
        ",MYF?##Zc_@-7n7Y-"
        "f:&:)_/"
        "_f14[q'#s)rFreXZV$jA#2T06CX(`5LF%5[)<#8JFgL`bVV$PSltQXl)oCM-m.L&lpWhX8M`38-i3OQT)YLk7?>#hfor-ZTmA#xM,W-XjJe$;A@&,'>6d3_Du'&hF;"
        ";$.qLk+"
        "I;YY#B^i&#R(FcM`=?>#*N2A.&)###?IG&#2&v:rgI`v%3l:^#rQ@m-'C<L#?-39'f0@##`$At[:^/@66RG##Pui/"
        "%Iq8;]`>(C&Okk2$7U$Z$AGs8].#KxtA:u+M^8oO(ZeGH3%duA#"
        "gLrie$&Mu>0)@qLYbKM'/c&EG.(2t-R1`.NS@-##qRj3N((^fLeQW6NUL?##P@S?NVRH##oBH*NWXQ##I@r.N,@,gLjx&-#+MC;$xm,D-erfcMS@-##f/"
        "_1N4(^fL_Fq4NUL?##)wI7N"
        "*4pfL?1YCNWXQ##8*N+Ng3[##+:p*#+mk.#cSq/#TH4.#*,f'0c:F&#Y#S-#S@Rm/"
        "0E-(#5dZ(#V4`T.]gb.#6H5s-750sLT<DP8DX=gG&>.FHMeZ<0-K*.3p6cYH0A2iF_q&##<@f>-"
        "<I7u/YZO.#B*]-#eq*7/"
        "9l7FH1I1qLx`5oLkL#.#Vr.>-Q@sY0BvZ6D.gcdGAbrmL=#d.#CT#<-ok*J-Y4%I-$Ln*.UoYlLE8QlM;i@(#av8kLV,:kL5=Av2GxtLFkG6fGB0E,31-s]5"
        "Hi<@K]8g,3<kUO1I4<nXot0kX+7auGR+p`Fa#`>-(0fJ2Yb-j1ecGL24gCSC7;ZhFhXHL28-IL2<_2;VC7JFIq3n0#v(+GMQDX1^E%i1THCHuug(to7Es''#CQCG)+"
        "#q<1OWf;-b@B[%"
        "I####gtl)mj2_20[G$##q3i@a_C@AtqTU(1";

    Fonts Winamp::addFont(const std::string &ttfFontPath, TextureMapEntry fontNumbers, TextureMapEntry fontRegular) const {
        auto io = ImGui::GetIO();
        io.Fonts->Clear();

        ImFontGlyphRangesBuilder range;
        range.Clear();
        ImVector<ImWchar> gr;
        gr.clear();
        static const ImWchar rr[] = {0x0020, 0x007F, 0};
        range.AddRanges(rr);
        range.AddChar(ImWchar(0x24c8)); // Ⓢ
        range.AddChar(ImWchar(0x2713)); // ✓

        std::vector<uint32_t> allchars;
        getCharRange(&allchars);
        for (const auto c : allchars) {
            range.AddChar(c);
        }

        range.BuildRanges(&gr);

        ImFont *a = io.Fonts->AddFontFromFileTTF(ttfFontPath.c_str(), fontSizeTTF, nullptr, gr.Data);
        ImFont *font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(empty_compressed_data_base85, 13.0f);

        char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\"@   0123456789..:()-'!_+\\/[]^&%,=$#\x0\x0\x0?*";
        char lowercase[] = "abcdefghijklmnopqrstuvwxyz";

        int rect_ids[sizeof(alphabet) + sizeof(lowercase)];
        int i = 0;
        for (char c : alphabet) {
            rect_ids[i] = io.Fonts->AddCustomRectFontGlyph(font, c, 14, (int)fontSizeBitmap, 14);
            i++;
        }

        for (char c : lowercase) {
            rect_ids[i] = io.Fonts->AddCustomRectFontGlyph(font, c, 14, (int)fontSizeBitmap, 14);
            i++;
        }

        char alphabetNum[] = "0123456789 -";

        ImFontConfig font_cfg = ImFontConfig();

        font_cfg.MergeMode = false;
        const ImWchar glyph_ranges[] = {
            0x002D,
            0x0039, // alphabetEx
            0,
        };

        font_cfg.GlyphRanges = glyph_ranges;
        ImFont *fontNumber = io.Fonts->AddFontDefault(&font_cfg);

        int iNum = 0;
        int rect_ids_num[IM_ARRAYSIZE(alphabetNum) - 1];
        int width = 26;
        int height = 37;
        for (i = 0; i < IM_ARRAYSIZE(alphabetNum) - 1; i++) {
            char c = alphabetNum[i];
            if (!isEx && i == (IM_ARRAYSIZE(alphabetNum) - 2)) { // last character, new minus
                width = 14;
                height = 2;
            }
            rect_ids_num[iNum] = io.Fonts->AddCustomRectFontGlyph(fontNumber, c, width, height, 26);
            iNum++;
        }

        char alphabetGen[27];
        strcpy(alphabetGen, lowercase);
        int rect_ids_gen[27];
        ImFontConfig font_cfg_gen = ImFontConfig();

        font_cfg_gen.MergeMode = false;

        font_cfg.GlyphRanges = glyph_ranges;

        ImFont *fontGen = io.Fonts->AddFontFromMemoryCompressedBase85TTF(empty_compressed_data_base85, 12.0f, &font_cfg_gen);

        iNum = 0;
        for (char c : alphabetGen) {
            rect_ids_gen[iNum] = io.Fonts->AddCustomRectFontGlyph(fontGen, c, 14, (int)fontSizeBitmap, 14);
            iNum++;
        }

        // read font bmps
        Magick::Image sourceNum;
        if (fontNumbers.len > 0) {
            Magick::Blob rawblobNum(fontNumbers.data, fontNumbers.len);
            try {
                sourceNum.read(rawblobNum);
            } catch (...) {
                DLOG("failed to read numbers.bmp\n");
            }
        }

        Magick::Image source;
        if (fontRegular.len > 0) {
            Magick::Blob rawblob(fontRegular.data, fontRegular.len);
            try {
                source.read(rawblob);
            } catch (...) {
                DLOG("failed to read text.bmp\n");
            }
        }

        unsigned char *tex_pixels = nullptr;
        int tex_width, tex_height;

        // Builds atlas, all font work must be done before it
        io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_width, &tex_height);

        // insert text font glyphs
        int column = 0;
        int rowIndex = 0;
        for (int rect_n = 0; rect_n < IM_ARRAYSIZE(rect_ids); rect_n++) {
            int rect_id = rect_ids[rect_n];

            if (rowIndex % 31 == 0 && rowIndex != 0) {
                rowIndex = 0;
                column++;
            }

            if (rect_n == sizeof(alphabet)) {
                column = 0;
                rowIndex = 0;
            }

            if (!source.isValid()) {
                continue;
            }
            Magick::Image glyph = source;
            glyph.depth(8);
            glyph.magick("RGBA");
            Magick::Geometry geo{5, 6, rowIndex * 5, column * 6};
            glyph.filterType(MagickCore::PointFilter);
            glyph.crop(geo);

            Magick::Geometry rg(15, (int)fontSizeBitmap);
            rg.fillArea(true);
            glyph.resize(rg);

            if (const ImFontAtlasCustomRect *rect = io.Fonts->GetCustomRectByIndex(rect_id)) {
                for (int y = 0; y < rect->Height; y++) {
                    ImU32 *p = (ImU32 *)tex_pixels + (rect->Y + y) * tex_width + (rect->X);
                    for (int x = 0; x < rect->Width; x++) {
                        auto pc = glyph.pixelColor(x, y);
                        *p++ = IM_COL32((int)pc.quantumRed(), (int)pc.quantumGreen(), (int)pc.quantumBlue(), (int)pc.quantumAlpha());
                    }
                }
            }
            rowIndex++;
        }

        int rowIndexNum = 0;
        if (sourceNum.isValid()) {
            // insert number font glyphs
            if (!isEx) {
                auto withMinus = Magick::Image();
                auto newWidth = sourceNum.size().width() + 5;
                auto oldWidth = sourceNum.size().width();
                withMinus.size({newWidth, sourceNum.size().height()});
                withMinus.copyPixels(sourceNum, sourceNum.size(), {0, 0});
                withMinus.copyPixels(sourceNum, {5, 13, 90, 0}, {(ssize_t)oldWidth, 0});
                withMinus.copyPixels(sourceNum, {5, 1, 20, 6}, {(ssize_t)oldWidth, 6});
                sourceNum = withMinus;
            }
        }

        auto glyphsNum = IM_ARRAYSIZE(rect_ids_num);
        if (isEx) {
            if (sourceNum.isValid()) {
                auto really = (sourceNum.size().width() / 9);
                if (really < glyphsNum) {
                    glyphsNum = (int)really;
                    DLOG(
                        "nums_ex is smaller than expected, %d px, can fit %d glyphs, but want %d\n",
                        sourceNum.size().width(),
                        really,
                        IM_ARRAYSIZE(rect_ids_num)
                    );
                }
            }
        }

        for (int rect_n = 0; rect_n < glyphsNum; rect_n++) {
            if (!sourceNum.isValid()) {
                continue;
            }
            int rect_id = rect_ids_num[rect_n];

            Magick::Image glyph = sourceNum;
            glyph.depth(8);
            glyph.magick("RGBA");

            // handling minus sign
            size_t w = 9;
            int nw = 26;
            size_t h = 13;
            int nh = 37;
            int yOff = 0;
            if (rect_n == IM_ARRAYSIZE(rect_ids_num) - 1 && !isEx) {
                w = 5;
                nw = 14;
                h = 1;
                nh = 2;
                yOff = 6;
            }

            Magick::Geometry geo{w, h, rowIndexNum * 9, yOff};
            glyph.filterType(MagickCore::PointFilter);
            glyph.crop(geo);

            Magick::Geometry rg(nw, nh);
            rg.fillArea(true);
            glyph.resize(rg);

            if (const ImFontAtlasCustomRect *rect = io.Fonts->GetCustomRectByIndex(rect_id)) {
                for (int y = 0; y < rect->Height; y++) {
                    ImU32 *p = (ImU32 *)tex_pixels + (rect->Y + y) * tex_width + (rect->X);
                    for (int x = 0; x < rect->Width; x++) {
                        auto pc = glyph.pixelColor(x, y);
                        *p++ = IM_COL32((int)pc.quantumRed(), (int)pc.quantumGreen(), (int)pc.quantumBlue(), (int)pc.quantumAlpha());
                    }
                }
            }

            rowIndexNum++;
        }

        font->SetGlyphVisible((ImWchar)' ', true);
        return Fonts{a, fontNumber, font};
    }

    // calculates marquee positions using ImGui::CalcTextSize
    // swaps calculated values with displayed ones
    void Winamp::MarqueeCalculate() {
        MarqueeInfo *w = &m;

        if (mStaging.text[0] != '\0') {
            w = &mStaging;
        }

        if (w->text[0] == '\0') {
            return;
        }

        int charLen;
        // shift current index by one utf8 character
        utfCharLen(&w->text[w->start], &charLen);

        w->start += charLen;

        const char *remaining; // points to char in text that won't be rendered
        ImFont *font;
        float fontSize;
        float maxWidth;
        if (config->useBitmapFont) {
            font = FontBitmap;
            fontSize = fontSizeBitmap;
            maxWidth = MarqueeTitleWidthBitmap;
        } else {
            font = FontRegular;
            fontSize = fontSizeTTF;
            maxWidth = MarqueeTitleWidthFont;
        }

        font->CalcTextSizeA(fontSize, maxWidth, -1.0f, &w->text[w->start], nullptr, &remaining);
        //        DLOG("remaining: len %zu, %s\n", strlen(remaining), remaining);
        if (strlen(remaining) < 1) {
            //            DLOG("wrap, small title %s, start %d\n", w->title, w->start);
            w->start = w->start - (int)strlen(separator) - (int)strlen(w->title);
            font->CalcTextSizeA(fontSize, maxWidth, -1.0f, &w->text[w->start], nullptr, &remaining);
        }

        sprintf(w->format, "%%.%lds", remaining - &w->text[w->start]);

        if (mStaging.updated) {
            //            DLOG("swapping real with staging\n");
            strcpy(m.text, mStaging.text);
            strcpy(m.title, mStaging.title);
            strcpy(m.format, mStaging.format);
            m.start = mStaging.start;

            memset(mStaging.text, 0, sizeof(mStaging.text));
            memset(mStaging.format, 0, sizeof(mStaging.format));
            memset(mStaging.title, 0, sizeof(mStaging.title));
            mStaging.start = 0;
            mStaging.updated = false;
        }
    }

    // calculates text offsets for bitmap font marquee without using ImGui::CalculateTextSize
    // bitmap marquee is always MarqueeMaxLengthBitmap (32) characters long
    __attribute__((unused)) void Winamp::MarqueeBitmap() {
        /*        if (currentSongTitleMarquee[0] == '\0') {
                    return;
                }

                int startTemp = m.start;
                int charLen = 0;
                // shift current index by one utf8 character
                utfCharLen(&currentSongTitleMarquee[m.start], &charLen);

                startTemp += charLen;

                // check if rest of the string fits into character limit

                if ((startTemp + MarqueeMaxLengthBitmap) > utfLen(currentSongTitleMarquee)) {
                    DLOG("need wrap\n");
                }

                int endTemp;
                bool fits;
                utfFits(currentSongTitleMarquee, startTemp, MarqueeMaxLengthBitmap, &fits, &endTemp);
                if (fits) {
                    m.start = startTemp;
                    snprintf(m.format, 10, "%%.%ds", endTemp - m.start);

                    char forLog[100]{};
                    strncpy(forLog, currentSongTitleMarquee + m.start, endTemp - m.start);
                    DLOG("start %03d, end %03d, %s\n", m.start, endTemp, forLog);

                    return;
                }

                // doesn't fit, wrap around
                m.start = 0;
                auto newPos = utfLen(currentSongTitle) - MarqueeMaxLengthBitmap + utfLen(separator) + 1;
                // recalculate start point in char
                for (int i = 0; i < newPos; i++) {
                    auto c = currentSongTitleMarquee[m.start];
                    utfCharLen(&c, &charLen);
                    m.start += charLen;
                }

                // recalculate end in char
                utfFits(currentSongTitleMarquee, m.start, MarqueeMaxLengthBitmap, &fits, &endTemp);
                if (fits) {
                    snprintf(m.format, 10, "%%.%ds", endTemp - m.start);

                    char forLog[100]{};
                    strncpy(forLog, currentSongTitleMarquee + m.start, endTemp - m.start);
                    DLOG("start %03d, end %03d, %s\n", m.start, endTemp, forLog);

                    return;
                }

                DLOG("this is not happening\n");
                exit(1);
                */
    }
} // namespace Winamp