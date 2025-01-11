#include "digital_clock.h"
#include "../skin.h"
#include "imgui_impl_opengl3.h"

const ImWchar rangesPunctuation[] = {
    0x2000,
    0x206F, // General Punctuation
    0,
};

namespace DigitalClock {
#ifdef DESKTOP
    const std::string FontPath = "../NotoSansKR-Regular.otf";
    const char *basePath = "../digital_clock";
#else
    const std::string FontPath = "/system/vendor/sony/lib/fonts/NotoSansKR-Regular.otf";
    const char *basePath = "/system/vendor/unknown321/usr/share/wampy/skins/digital_clock";
#endif

    const int numberWidth = 104;
    const int screenWidth = 480;

    std::map<std::string, std::string> colorsDigitalClock = {
        {"Silver", "silver"},
        {"Gold", "gold"},
        {"Blue", "blue"},
        {"Blue 2012", "blue_2012"},
        {"Green", "green"},
        {"Yellow", "yellow"},
        {"Pink 2012", "pink_2012"},
        {"Pink", "pink"},
        {"Purple", "purple"},
        {"Red product", "red_product"},
        {"Space Gray", "space_gray"},
    };

    std::string DefaultColor = "silver";
    std::string DefaultColorPreview = "Silver";

    void DigitalClock::drawFromAtlas(uint index, ImVec2 pos) {
        auto vvv = elements.atlas.images.at(index);
        //        DLOG("value %d, index %d, %dx%d, %d %d\n", value, atlasIndex + value, vvv.width, vvv.height, vvv.x, vvv.y);
        ImGui::SetCursorPos(pos);
        ImGui::Image(
            (ImTextureID)elements.atlas.textureID,
            ImVec2(vvv.width, vvv.height),
            ImVec2(vvv.u0, vvv.v0),
            ImVec2(vvv.u1, vvv.v1),
            ImVec4(1, 1, 1, 1)
        );
    }

    void DigitalClock::drawNum(NumType t, uint value, ImVec2 pos) {
        int atlasIndex;
        switch (t) {
        case NUM_BIG:
            atlasIndex = 0;
            break;
        case NUM_MEDIUM:
            atlasIndex = 17;
            break;
        case NUM_SMALL:
            atlasIndex = 31;
            break;
        default:
            atlasIndex = 0;
            break;
        }

        drawFromAtlas(atlasIndex + value, pos);
    }

    void DigitalClock::Draw() {
        if (loading) {
            return;
        }
        // no leading zeroes except seconds
        elements.ToggleSettings.Draw();

        if (H1 > 0) {
            drawNum(NUM_BIG, H1, {304, screenWidth - numberWidth - 12});
        }
        drawNum(NUM_BIG, H2, {304, screenWidth - numberWidth - 116});
        drawFromAtlas(colonIndex, {344, screenWidth - 30 - 226});
        drawNum(NUM_BIG, M1, {304, screenWidth - numberWidth - 262});
        drawNum(NUM_BIG, M2, {304, screenWidth - numberWidth - 366});

        drawFromAtlas(daysIndex + timeinfo->tm_wday, {34, screenWidth - 140 - 20});

        drawNum(NUM_MEDIUM, Day2, {34, screenWidth - 62 - 402});
        if (Day1 > 0) {
            drawNum(NUM_MEDIUM, Day1, {34, screenWidth - 62 - 342});
            drawFromAtlas(minusIndex, {66, screenWidth - 34 - 308});
            drawNum(NUM_MEDIUM, Month2, {34, screenWidth - 62 - 246});
            if (Month1 > 0) {
                drawNum(NUM_MEDIUM, Month1, {34, screenWidth - 62 - 184});
            }
        } else {
            drawFromAtlas(minusIndex, {66, screenWidth - 62 - 342});
            drawNum(NUM_MEDIUM, Month2, {34, screenWidth - 62 - 308});
            if (Month1 > 0) {
                drawNum(NUM_MEDIUM, Month1, {34, screenWidth - 62 - 246});
            }
        }

        drawNum(NUM_SMALL, Sec2, {714, screenWidth - 460});
        drawNum(NUM_SMALL, Sec1, {714, screenWidth - 30 * -1 - 460});

        //        elements.Shoe.DrawAt(670, screenWidth - 132);

        drawNum(NUM_SMALL, Year1, {714, screenWidth - 30 - 20});
        drawNum(NUM_SMALL, Year2, {714, screenWidth - 30 * 2 - 20});
        drawNum(NUM_SMALL, Year3, {714, screenWidth - 30 * 3 - 20});
        drawNum(NUM_SMALL, Year4, {714, screenWidth - 30 * 4 - 20});
    }

    void DigitalClock::loadAtlas(const std::string &c) {
        DLOG("loading atlas for clock %s\n", c.c_str());
        elements.atlas = LoadAtlas(std::string(basePath) + "/" + c + "/atlas.pkm", std::string(basePath) + "/" + c + "/atlas.txt");
        DLOG("%s\n", (std::string(basePath) + "/" + c + "/atlas.txt").c_str());
        if (elements.atlas.images.empty()) {
            DLOG("no images in atlas %s\n", c.c_str());
            return;
        }

        DLOG("%zu images in atlas\n", elements.atlas.images.size());
    }

    int DigitalClock::Load(std::string filename, ImFont **fontRegular) {
        loading = true;
        childThreadsStop = false;

        if (filename.empty()) {
            filename = DefaultColor;
        }

        bool found;
        for (const auto &v : colorsDigitalClock) {
            if (v.second == filename) {
                found = true;
                break;
            }
        }

        if (!found) {
            DLOG("unexpected color %s, setting to default\n", filename.c_str());
            filename = DefaultColor;
        }

        initializeButton();
        addFonts(fontRegular);
        loadAtlas(filename);

        color = filename;

        time(&rawtime);
        timeinfo = gmtime(&rawtime);
        formatTime();

        auto update = [this]() { ticker(); };
        std::thread v(update);
        v.detach();

        loading = false;

        return 0;
    }

    void DigitalClock::formatTime() {
        H1 = timeinfo->tm_hour / 10;
        H2 = timeinfo->tm_hour % 10;

        M1 = timeinfo->tm_min / 10;
        M2 = timeinfo->tm_min % 10;

        Year1 = (timeinfo->tm_year + 1900) / 1000;
        Year2 = ((timeinfo->tm_year + 1900) / 100) % 10;
        Year3 = ((timeinfo->tm_year + 1900) / 10) % 100;
        Year4 = (timeinfo->tm_year + 1900) % 10;

        Month1 = (timeinfo->tm_mon + 1) / 10;
        Month2 = (timeinfo->tm_mon + 1) % 10;
        Day1 = timeinfo->tm_mday / 10;
        Day2 = timeinfo->tm_mday % 10;

        Sec1 = timeinfo->tm_sec / 10;
        Sec2 = timeinfo->tm_sec % 10;
    }

    void DigitalClock::ticker() {
        tickerThreadRunning = true;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (childThreadsStop) {
                break;
            }

            if (!*render) {
                continue;
            }

            time(&rawtime);
            timeinfo = gmtime(&rawtime);
            formatTime();
        }

        tickerThreadRunning = false;
        DLOG("ticker thread stopped\n");
    }

    void DigitalClock::LoadNewSkin() {
        if (color == newColor) {
            DLOG("not updating current skin %s\n", color.c_str());
            return;
        }

        Unload();
        Load(newColor, &FontRegular);
        config->color = color;
    }

    ImFont *DigitalClock::GetFont() { return FontRegular; }

    void DigitalClock::Unload() {
        childThreadsStop = true;

        elements.ToggleSettings.Unload();

        ImGui_ImplOpenGL3_DestroyFontsTexture();

        UnloadTexture(elements.atlas.textureID);
        elements.atlas.images.clear();

        while (tickerThreadRunning) {
            // wait for threads to stop
        }
    }

    void DigitalClock::WithConfig(Config *c) { config = c; };

    void DigitalClock::initializeButton() {
        FlatTexture flatTexture;
        ImGui::ButtonTexture bt;
        ImGui::ButtonTextures bts;

        bt.active = flatTexture.FromColor({80, 80}, {255.0f, 0.0f, 0.0f, 0.0f});
        bt.size = flatTexture.GetSize();
        bt.pressed = bt.active;
        bts[0] = bt;
        elements.ToggleSettings.WithID("toggleSettings")
            ->WithPosition(800 - bt.size.x, 0.0f)
            ->WithTextures(bts)
            ->WithCallback(Skin::Skin::ToggleDrawSettings, skin, nullptr);
    }

    int DigitalClock::addFonts(ImFont **fontRegular) {
        auto io = ImGui::GetIO();
        io.Fonts->Clear();

        ImFontGlyphRangesBuilder range;
        range.Clear();
        ImVector<ImWchar> gr;
        gr.clear();

        range.AddRanges(io.Fonts->GetGlyphRangesDefault());
        range.AddRanges(rangesPunctuation);

        range.BuildRanges(&gr);
        *fontRegular = io.Fonts->AddFontFromFileTTF(FontPath.c_str(), fontSizeTTF, nullptr, gr.Data);

        ImGui_ImplOpenGL3_DestroyFontsTexture();
        ImGui_ImplOpenGL3_CreateFontsTexture();

        return 0;
    }

    std::string DigitalClock::GetActiveColorPreview() {
        for (const auto &v : colorsDigitalClock) {
            if (v.second == color) {
                return v.first;
            }
        }

        return DefaultColorPreview;
    }

    std::string DigitalClock::GetColorPreview(const std::string &c) {
        for (const auto &v : colorsDigitalClock) {
            if (v.second == c) {
                return v.first;
            }
        }

        return DefaultColorPreview;
    }
} // namespace DigitalClock