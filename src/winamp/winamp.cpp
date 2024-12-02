#include "winamp.h"
#include "../Version.h"
#include "../skin.h"
#include "../unzip/unzip.cpp"
#include "../wstring.h"
#include "imgui_impl_opengl3.h"
#include <thread>

static const ImWchar rangesPunctuation[] = {
    0x2000, 0x206F, // General Punctuation
};

namespace Winamp {

    std::string const ColorBlack = "#000000";
    static const float ttfFontSize = 33.0f;
    static const float TitleWidth = 595.0f;
    static const int MarqueeMaxLengthBitmap = 32;
    static const float MaxTitleWidth = 1024.0f;

    static const int blinkInterval = 1200 * 1000; // microseconds, got it via screen recording, winamp 2.95
    static const int marqueeInterval = 200 * 1000;

    static const float PlaylistYRegular = 337.0;
    static float PlaylistY = 337.0;
    static const int PlaylistTitleHeight = 58;
    static const int VolumeBarCount = 28;
    static const int BalanceBarCount = 28;

#ifdef DESKTOP
    //    static const std::string FontPath = "../SSTJpPro-Regular.otf";
    static const std::string FontPath = "../NotoSansKR-Regular.otf";
#else
    static const std::string FontPath = "/system/vendor/sony/lib/fonts/NotoSansKR-Regular.otf";
#endif

    static const std::list<const char *> filenames = {
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
        "balance.bmp",
        "pledit.bmp",
        "pledit.txt"};

    int Winamp::AddFonts(ImFont *fontRegular) {
        auto numFontData = textures["numbers.bmp"];
        if (textures["nums_ex.bmp"].len != 0) {
            numFontData = textures["nums_ex.bmp"];
            isEx = true;
        }

        auto res = addFont(FontPath, numFontData, textures["text.bmp"]);
        FontBitmap = res.bitmap;
        FontNumbers = res.number;
        FontRegular = res.regular;
        fontRegular = res.regular;

        ImGui_ImplOpenGL3_DestroyFontsTexture();
        ImGui_ImplOpenGL3_CreateFontsTexture();

        fontRegular->FontSize = 25; // this will scale down elements in settings

        return 0;
    }

    int Winamp::volumeIsBalance() {
        if (textures["balance.bmp"].len != 0) {
            return 0;
        }

        if (textures["volume.bmp"].len == 0) {
            return -1;
        }

        DLOG("balance.bmp is missing, using volume.bmp as replacement\n");

        textures["balance.bmp"].data = (char *)malloc(textures["volume.bmp"].len);
        memcpy((void *)textures["balance.bmp"].data, (void *)textures["volume.bmp"].data, textures["volume.bmp"].len);
        textures["balance.bmp"].len = textures["volume.bmp"].len;

        return 0;
    }

    void Winamp::probeTrackTitleBackgroundColor() {
        FlatTexture t;
        t.FromPair(textures["text.bmp"]);
        colors.trackTitleBackground = t.GetColor(150, 5);
    }

    int Winamp::Load(std::string filename, ImFont *fontRegular) {
        loading = true;
        isEx = false;
        loadStatusStr = "";
        DLOG("Unzip %s\n", filename.c_str());

        for (auto &f : filenames) {
            textures[f];
        }

        if (unzip(filename, &textures, &loadStatusStr) < 0) {
            newFilename = filename;
            loading = false;
            return -1;
        }

        if (volumeIsBalance() < 0) {
            newFilename = filename;
            loading = false;
            return -1;
        }

        readPlEdit();

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

        MarqueeThread();

        return 0;
    }

    int Winamp::unzip(const std::string &filename, TextureMap *textures, std::string *status) {
        int err = unzipFiles(filename.c_str(), textures);
        if (err != UNZ_OK) {
            *status = "unzip error " + std::to_string(err);
            DLOG("%s\n", status->c_str());
            return -1;
        }

        // are all files present?
        for (auto const &k : *textures) {
            if (k.first == "nums_ex.bmp" || k.first == "numbers.bmp" || k.first == "balance.bmp") {
                continue;
            }

            if (k.second.len == 0) {
                *status = k.first + " is empty or doesn't exist";
                DLOG("%s\n", status->c_str());
                return -1;
            }
        }

        if (textures->at("nums_ex.bmp").len == 0 && textures->at("numbers.bmp").len == 0) {
            *status = "numbers.bmp and nums_ex.bmp are missing\n";
            DLOG("%s\n", status->c_str());
            return -1;
        }

        if (textures->at("balance.bmp").len == 0 && textures->at("volume.bmp").len == 0) {
            *status = "balance.bmp and volume.bmp are missing\n";
            DLOG("%s\n", status->c_str());
            return -1;
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
        char buff[100];
        snprintf(buff, sizeof(buff), "Seek To: %02d:%02d/%02d:%02d (%d%%)", newMin, newSec, totalMin, totalSec, val);
        std::string buffAsStdStr = buff;

        w->connector->playlist.at(0).TitleMarquee = buffAsStdStr;
    }

    void Winamp::SeekReleased(void *winampSkin, void *i) {
        auto w = (Winamp *)winampSkin;

        auto val = *(int *)i;
        w->connector->SetPosition(val);
        w->MarqueeRunning = true;
        w->connector->status.Elapsed = w->connector->status.Duration * val / 100;
#ifndef DESKTOP
        w->connector->updateElapsedCounter = 2;
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

        char buff[100];
        snprintf(buff, sizeof(buff), "Volume: %d%%", val);
        std::string buffAsStdStr = buff;

        w->connector->playlist.at(0).TitleMarquee = buffAsStdStr;
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
        w->MarqueeRunning = true;
    }

    void Winamp::VolumePressedHagoromo(void *winampSkin, void *i) {
        auto w = (Winamp *)winampSkin;
        w->MarqueeRunning = false;

        auto val = *(int *)i;
        w->connector->updateVolume = false;

        char buff[100];
        snprintf(buff, sizeof(buff), "Volume: %d%%", val);
        std::string buffAsStdStr = buff;

        w->connector->playlist.at(0).TitleMarquee = buffAsStdStr;
    }

    void Winamp::VolumeReleasedHagoromo(void *winampSkin, void *i) {
        auto w = (Winamp *)winampSkin;
        w->MarqueeRunning = true;

        auto val = *(int *)i;
        w->connector->status.Volume = val;
        w->connector->SetVolume(val, false);
        w->connector->updateVolume = true;
    }

    void Winamp::BalancePressed(void *winampSkin, void *i) {
        notImplemented(winampSkin, i);
        /*
                     if (i == 0) {
                        format = "Balance: Center";
                    } else if (i < 0) {
                        format = "Balance: %d%% left";
                    } else {
                        format = "Balance: %d%% right";
                    }
            snprintf(buff, sizeof(buff), format, abs(i));
            std::string buffAsStdStr = buff;
        */
    }

    void Winamp::BalanceReleased(void *winampSkin, void *i) {
        auto w = (Winamp *)winampSkin;
        w->MarqueeRunning = true;
    }

    void Winamp::Draw() {
        Format();

        if (playlistFullscreen) {
            drawPlaylist();
            return;
        }

        Elements.Main.Draw();
        Elements.Title.Draw();
        Elements.ClutterBar.Draw();

        switch (hash(connector->status.State.c_str())) {
        case hash("play"):
            Elements.PlayIndicator.Draw();
            Elements.BufferingIndicator.Draw();
            break;
        case hash("pause"):
            if (stopped) {
                Elements.StopIndicator.Draw();
            } else {
                Elements.PauseIndicator.Draw();
            }
            break;
        case hash("stop"):
        default:
            Elements.StopIndicator.Draw();
            break;
        }

        if (connector->status.Channels == 1) {
            Elements.MonoOnIndicator.Draw();
            Elements.StereoOffIndicator.Draw();
        } else {
            Elements.MonoOffIndicator.Draw();
            Elements.StereoOnIndicator.Draw();
        }

        Elements.ShuffleButton.Draw(connector->status.Shuffle);
        Elements.RepeatButton.Draw(connector->status.Repeat);
        Elements.EQButton.Draw();
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

        if (connector->playlist.at(0).TitleMarquee.empty()) {
            ImGui::Text("Wampy %s", SOFTWARE_VERSION);
        } else {
            ImGui::Text("%s", connector->playlist.at(0).TitleMarquee.c_str());
        }

        if (!config->useBitmapFont) {
            ImGui::PopStyleColor(1);
        }

        ImGui::PopFont();

        ImGui::PushFont(FontBitmap);
        ImGui::SetCursorPos(ImVec2(323, 125));
        ImGui::Text("%s", connector->status.BitrateString.c_str());

        ImGui::SetCursorPos(ImVec2(455, 125));
        ImGui::Text("%s", connector->status.SampleRateString.c_str());
        ImGui::PopFont();

        if (stopped || connector->stateString == "stop") {
            // do nothing
        } else {
            blinkTrackTime();
        }

        Elements.TrackTimeToggle.Draw();
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

        for (int i = 0; i < connector->playlist.size(); i++) {
            if (!playlistFullscreen) {
                if (i > 2) {
                    break;
                }
            }

            auto s = &connector->playlist.at(i);
            if (s->Duration <= 0) {
                continue;
            }

            // FIXME recalculates on every update in mpd
            if ((!s->PlaylistStringsCalculated)) {
                auto format = "%s. %s - %s";
                auto size = std::snprintf(nullptr, 0, format, s->Track.c_str(), s->Artist.c_str(), s->Title.c_str());
                std::string output(size + 1, '\0');
                std::sprintf(&output[0], format, s->Track.c_str(), s->Artist.c_str(), s->Title.c_str());
                s->PlaylistTitle = CalculateTextWidth(output);

                format = "%02d:%02d";
                size = std::snprintf(nullptr, 0, format, s->Duration / 60, s->Duration % 60);
                std::string outputDuration(size + 1, '\0');
                std::sprintf(&outputDuration[0], format, s->Duration / 60, s->Duration % 60);
                s->PlaylistDuration = outputDuration;

                ImVec2 res = ImGui::CalcTextSize(s->PlaylistDuration.c_str());
                s->PlaylistDurationTextSize = res.x;

                s->PlaylistStringsCalculated = true;
            }

            if (i == 0) {
                ImGui::PushStyleColor(ImGuiCol_Text, colors.PlaylistCurrentTextU32);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, colors.PlaylistNormalTextU32);
            }

            ImGui::SetCursorPos(ImVec2(38, PlaylistY + 60 + ((float)i * 30)));
            ImGui::Text("%s", s->PlaylistTitle.c_str());

            ImGui::SetCursorPos(ImVec2(800 - s->PlaylistDurationTextSize - 60, PlaylistY + 60 + ((float)i * 30)));
            ImGui::Text("%s", s->PlaylistDuration.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::PopFont();
    }

    int Winamp::initializeElements() {
        Elements.Main.FromPair(textures["main.bmp"])
            ->WithCrop(Magick::RectangleInfo{275, 116, 0, 0})
            ->WithFilledRectangle({770, 96, 323, 78}, colors.trackTitleBackground)
            ->Load();
        Elements.Title.FromPair(textures["titlebar.bmp"])->WithCrop(Magick::RectangleInfo{275, 14, 27, 0})->Load();
        Elements.ClutterBar.FromPair(textures["titlebar.bmp"])
            ->WithCrop(Magick::RectangleInfo{8, 43, 304, 0})
            ->WithPosition(ImVec2(29.0f, 64.0f))
            ->Load();
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
        Elements.StopIndicator.FromPair(textures["playpaus.bmp"])->WithCrop({9, 9, 18, 0})->WithPosition(ImVec2(79.0f, 81.0f))->Load();
        Elements.PlayIndicator.FromPair(textures["playpaus.bmp"])->WithCrop({9, 9, 0, 0})->WithPosition(ImVec2(79.0f, 81.0f))->Load();
        Elements.PauseIndicator.FromPair(textures["playpaus.bmp"])->WithCrop({9, 9, 9, 0})->WithPosition(ImVec2(79.0f, 81.0f))->Load();
        Elements.BufferingIndicator.FromPair(textures["playpaus.bmp"])
            ->WithCrop({3, 9, 36, 0})
            ->WithPosition(ImVec2(70.0f, 81.0f))
            ->WithScale({8, 24}, false)
            ->Load();

        this->initializeButtons();
        this->initializePlaylist();
        this->initializeSliders();

        return 0;
    }

    int Winamp::initializePlaylist() {
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

        return 0;
    }

    int Winamp::initializeButtons() {
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
        Elements.EQButton.WithPosition(637.0f, 168.0f)->WithTextures(bts)->WithCallback(notImplemented, this, nullptr)->WithID("eq");

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

        //=========
        bts.clear();
        flatTexture.Reset();
        bt.active = flatTexture.FromColor({183, 37}, {255.0f, 0.0f, 0.0f, 0.0f});
        bt.size = flatTexture.GetSize();
        bt.pressed = bt.active;
        bts[0] = bt;
        Elements.TrackTimeToggle.WithID("trackTimeToggle")
            ->WithPosition(105.0f, 76.0f)
            ->WithTextures(bts)
            ->WithCallback(toggleTrackTime, this, nullptr);

        return 0;
    }

    int Winamp::initializeSliders() {
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

        butT.active = ft.FromPair(textures["volume.bmp"])->WithCrop({14, 11, 15, 422})->WithScale({40, 31}, false)->Load();

        ft.Reset();
        butT.pressed = ft.FromPair(textures["volume.bmp"])->WithCrop({14, 11, 0, 422})->WithScale({40, 31}, false)->Load();
        butT.size = ft.GetSize();
        butTs[0] = butT;

        for (int i = 0; i < VolumeBarCount; i++) {
            ft.Reset();
            barT.textureId = ft.FromPair(textures["volume.bmp"])->WithCrop({68, 13, 0, i * 15})->WithScale({198, 37}, true)->Load();
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
        butT.active = ft.FromPair(textures["balance.bmp"])->WithCrop({14, 11, 15, 422})->WithScale({40, 31}, false)->Load();
        ft.Reset();
        butT.pressed = ft.FromPair(textures["balance.bmp"])->WithCrop({14, 11, 0, 422})->WithScale({40, 31}, false)->Load();
        butT.size = ft.GetSize();
        butTs[0] = butT;
        ft.Reset();

        for (int i = 0; i < BalanceBarCount; i++) {
            ft.Reset();
            barT.textureId = ft.FromPair(textures["balance.bmp"])->WithCrop({38, 13, 9, i * 14 + i})->Load();
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
        return 0;
    }

    void Winamp::notImplemented(void *winampSkin, void *) {
        auto w = (Winamp *)winampSkin;
        w->MarqueeRunning = false;

        w->connector->playlist.at(0).TitleMarquee = "Not implemented";
    }

    void Winamp::drawTime() {
        auto s = connector->playlist.at(0);
        if (s.Duration < 1) {
            return;
        }

        if (s.DurDisplay.Minute1 < 1 && s.DurDisplay.Minute2 < 1 && s.DurDisplay.Second1 < 1 && s.DurDisplay.Second2 < 1) {
            return;
        }

        ImGui::PushFont(FontNumbers);

        // minus sign
        if (timeRemaining) {
            if (isEx) {
                ImGui::SetCursorPos(ImVec2(111, 76));
            } else {
                ImGui::SetCursorPos(ImVec2(113, 92));
            }
            ImGui::Text("%s", remainingTimeSign.c_str());
        }

        ImGui::SetCursorPos(ImVec2(140, 76));
        ImGui::Text("%d", s.DurDisplay.Minute1);

        ImGui::SetCursorPos(ImVec2(175, 76));
        ImGui::Text("%d", s.DurDisplay.Minute2);

        ImGui::SetCursorPos(ImVec2(227, 76));
        ImGui::Text("%d", s.DurDisplay.Second1);

        ImGui::SetCursorPos(ImVec2(262, 76));
        ImGui::Text("%d", s.DurDisplay.Second2);
        ImGui::PopFont();
    }

    void Winamp::blinkTrackTime() {
        if (connector->status.State == "pause") {
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

    void Winamp::toggleTrackTime(void *arg, void *i) {
        auto *w = (Winamp *)arg;
        if (w->timeRemaining) {
            w->timeRemaining = false;
        } else {
            w->timeRemaining = true;
        }
    }

    void Winamp::togglePlaylistFullscreen(void *arg, void *) {
        auto *w = (Winamp *)arg;
        if (w->playlistFullscreen) {
            w->playlistFullscreen = false;
        } else {
            w->playlistFullscreen = true;
        }
    }

    void Winamp::MarqueeInFrame() {
        if (savedTitle != connector->playlist.at(0).TitleFormatted) {
            savedTitle = connector->playlist.at(0).TitleFormatted;
            newFull = savedTitle + "  ***  ";
        }

        const char *end = nullptr;
        auto ctx = ImGui::GetCurrentContext();
        ImVec2 size =
            FontRegular->CalcTextSizeA(ttfFontSize, MaxTitleWidth, -1.0f, connector->playlist.at(0).TitleFormatted.c_str(), end, nullptr);
        if (size.x <= TitleWidth) {
            connector->playlist.at(0).TitleMarquee = connector->playlist.at(0).TitleFormatted;
            return;
        }

        char shifted[newFull.length()];
        utfShift(newFull, shifted);
        newFull = shifted;

        std::string ff = newFull;
        int len = 0;
        char cut[ff.length()];
        memset(cut, 0, sizeof cut);

        while (size.x > TitleWidth) {
            len = utfLen(ff);
            memset(cut, 0, sizeof cut);
            utfCut(ff, len - 1, cut);
            ff = std::string(cut);
            len = len - 1;
            size = FontRegular->CalcTextSizeA(ttfFontSize, MaxTitleWidth, -1.0f, ff.c_str());
        }

        connector->playlist.at(0).TitleMarquee = cut;
    }

    void Winamp::MarqueeBitmap() {
        int len = utfLen(connector->playlist.at(0).TitleFormatted);
        if (len <= MarqueeMaxLengthBitmap) {
            connector->playlist.at(0).TitleMarquee = connector->playlist.at(0).TitleFormatted;
            return;
        }

        if (savedTitle != connector->playlist.at(0).TitleFormatted) {
            savedTitle = connector->playlist.at(0).TitleFormatted;
            newFull = savedTitle + "  ***  ";
        }

        char shifted[newFull.length()];
        utfShift(newFull, shifted);
        newFull = shifted;

        char cut[newFull.length()];
        utfCut(newFull, MarqueeMaxLengthBitmap, cut);

        connector->playlist.at(0).TitleMarquee = cut;
    }

    void Winamp::Marquee() {
        for (;;) {
            std::this_thread::sleep_for(std::chrono::microseconds(marqueeInterval));
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
                connector->playlist.at(0).TitleMarquee = connector->stateString;
                connector->playlist.at(0).TitleFormatted = connector->stateString;
            }

            if (config->useBitmapFont) {
                MarqueeBitmap();
            } else {
                MarqueeInFrame();
            }
        }
    }

    void Winamp::MarqueeThread() {
        if (marqueeThread)
            pthread_cancel(marqueeThread);

        auto exec = [this]() { Marquee(); };
        std::thread t(exec);

        marqueeThread = t.native_handle();
        t.detach();
        MarqueeRunning = true;
    }

    void Winamp::Stop(void *winamp, void *) {
        auto w = (Winamp *)winamp;
        w->stopped = true;
        Connector::Stop(w->connector, nullptr);
    }

    void Winamp::Play(void *winamp, void *) {
        auto w = (Winamp *)winamp;
        w->stopped = false;
        Connector::Play(w->connector, nullptr);
    }

    void Winamp::Pause(void *winamp, void *) {
        auto w = (Winamp *)winamp;
        w->stopped = false;
        Connector::Pause(w->connector, nullptr);
    }

    void Winamp::Prev(void *winamp, void *) {
        auto w = (Winamp *)winamp;
        w->stopped = false;
        Connector::Prev(w->connector, nullptr);
    }

    void Winamp::Next(void *winamp, void *) {
        auto w = (Winamp *)winamp;
        w->stopped = false;
        Connector::Next(w->connector, nullptr);
    }

    void elements::Unload() {
        Main.Unload();
        Title.Unload();

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

        TrackTimeToggle.Unload();

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
        if (marqueeThread) {
            auto r = pthread_cancel(marqueeThread);
            if (r != 0) {
                DLOG("cannot cancel marquee thread\n");
            }
        }

        MarqueeRunning = false;
    }

    void Winamp::WithConfig(Config *c) { config = c; }

    void Config::Default() {
        useBitmapFont = true;
        useBitmapFontInPlaylist = false;
        filename = "base-2.91.wsz";
    }

    Config Config::GetDefault() {
        auto c = Config{};
        c.Default();
        return c;
    }

    void Winamp::Format() {
        auto status = &connector->status;
        if (status->formatted) {
            return;
        }

        if (status->Duration > 0) {
            status->PositionPercent = (int)std::ceil(float(status->Elapsed) / float(status->Duration) * 100);
        } else {
            status->formatted = true;
            return;
        }

        int minutes, seconds;
        if (timeRemaining) {
            minutes = (status->Duration - status->Elapsed) / 60;
            seconds = (status->Duration - status->Elapsed) % 60;
            remainingTimeSign = "-";
        } else {
            minutes = status->Elapsed / 60;
            seconds = status->Elapsed % 60;
            remainingTimeSign = "";
        }

        auto playlist = &connector->playlist;

        playlist->at(0).DurDisplay.Minute1 = minutes / 10;
        playlist->at(0).DurDisplay.Minute2 = minutes % 10;
        playlist->at(0).DurDisplay.Second1 = seconds / 10;
        playlist->at(0).DurDisplay.Second2 = seconds % 10;

        char secs[3]{};
        int fullSeconds = playlist->at(0).Duration % 60;
        std::snprintf(secs, 3, "%02d", fullSeconds);
        playlist->at(0).TitleFormatted = playlist->at(0).Track + ". " + playlist->at(0).Artist + " - " + playlist->at(0).Title + " (" +
                                         std::to_string(playlist->at(0).Duration / 60) + ":" + std::string(secs) + ")";

        status->formatted = true;
    }

    Fonts Winamp::addFont(const std::string &ttfFontPath, TextureMapEntry fontNumbers, TextureMapEntry fontRegular) const {
        auto io = ImGui::GetIO();
        io.Fonts->Clear();

        static ImFontGlyphRangesBuilder range;
        range.Clear();
        static ImVector<ImWchar> gr;
        gr.clear();
        range.AddRanges(io.Fonts->GetGlyphRangesDefault());
        range.AddRanges(&rangesPunctuation[0]);

        if (fontRanges) {
            if (fontRanges->Japanese)
                range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
            if (fontRanges->ChineseFull)
                range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
            if (fontRanges->Cyrillic)
                range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
            if (fontRanges->Greek)
                range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesGreek());
            if (fontRanges->Korean)
                range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesKorean());
            if (fontRanges->Thai)
                range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesThai());
            if (fontRanges->Vietnamese)
                range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesVietnamese());
        }

        range.BuildRanges(&gr);

        ImFont *a = io.Fonts->AddFontFromFileTTF(ttfFontPath.c_str(), ttfFontSize, nullptr, gr.Data);
        ImFont *font = io.Fonts->AddFontDefault();

        char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\"@   0123456789..:()-'!_+\\/[]^&%,=$#\x0\x0\x0?*";
        char lowercase[] = "abcdefghijklmnopqrstuvwxyz";

        int rect_ids[sizeof(alphabet) + sizeof(lowercase)];
        int i = 0;
        for (char c : alphabet) {
            rect_ids[i] = io.Fonts->AddCustomRectFontGlyph(font, c, 14, 17, 14);
            i++;
        }

        for (char c : lowercase) {
            rect_ids[i] = io.Fonts->AddCustomRectFontGlyph(font, c, 14, 17, 14);
            i++;
        }

        //    std::pair<char *, size_t> data;
        char alphabetNum[] = "0123456789 -";
        size_t len = sizeof(alphabetNum) - 1;

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
        int rect_ids_num[len];
        int width = 26;
        int height = 37;
        for (i = 0; i < IM_ARRAYSIZE(alphabetNum); i++) {
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
        //    const ImWchar glyph_ranges_gen[] = {
        //            0x002D, 0x0039, // alphabetEx
        //            0,
        //    };

        font_cfg.GlyphRanges = glyph_ranges;
        ImFont *fontGen = io.Fonts->AddFontDefault(&font_cfg_gen);

        iNum = 0;
        for (char c : alphabetGen) {
            rect_ids_gen[iNum] = io.Fonts->AddCustomRectFontGlyph(fontGen, c, 14, 17, 14);
            iNum++;
        }

        // read font bmps
        Magick::Blob rawblobNum(fontNumbers.data, fontNumbers.len);
        Magick::Image sourceNum;
        try {
            sourceNum.read(rawblobNum);
        } catch (...) {
            DLOG("failed to read numbers.bmp\n");
        }

        Magick::Blob rawblob(fontRegular.data, fontRegular.len);
        Magick::Image source;
        try {
            source.read(rawblob);
        } catch (...) {
            DLOG("failed to read text.bmp\n");
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

            Magick::Image glyph = source;
            glyph.depth(8);
            glyph.magick("RGBA");
            Magick::Geometry geo{5, 6, rowIndex * 5, column * 6};
            glyph.filterType(MagickCore::PointFilter);
            glyph.crop(geo);

            Magick::Geometry rg(15, 17);
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

        // insert number font glyphs
        int rowIndexNum = 0;
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

        for (int rect_n = 0; rect_n < IM_ARRAYSIZE(rect_ids_num); rect_n++) {
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
} // namespace Winamp