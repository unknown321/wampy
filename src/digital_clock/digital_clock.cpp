#include "digital_clock.h"
#include "../skin.h"
#include "imgui_impl_opengl3.h"

namespace DigitalClock {
#ifdef DESKTOP
    const std::string FontPath = "../NotoSansKR-Regular.otf";
    const std::string FontPathCustom = "../font.ttf";
    const std::string FontPathCustom2 = "../font.otf";
    const char *basePath = "../digital_clock";
#else
    const std::string FontPath = "/system/vendor/sony/lib/fonts/NotoSansKR-Regular.otf";
    const std::string FontPathCustom = "/contents/wampy/fonts/font.ttf";
    const std::string FontPathCustom2 = "/contents/wampy/fonts/font.otf";
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

    void DigitalClock::Draw() {
        if (loading) {
            return;
        }
        // no leading zeroes except seconds
        elements.ToggleSettings.Draw();
        if (H1 > 0) {
            elements.NumbersBig.at(H1).DrawAt(304, screenWidth - numberWidth - 12);
        }
        elements.NumbersBig.at(H2).DrawAt(304, screenWidth - numberWidth - 116);
        elements.Colon.DrawAt(344, screenWidth - 30 - 226);
        elements.NumbersBig.at(M1).DrawAt(304, screenWidth - numberWidth - 262);
        elements.NumbersBig.at(M2).DrawAt(304, screenWidth - numberWidth - 366);

        elements.Days.at(timeinfo->tm_wday).DrawAt(34, screenWidth - 140 - 20);

        elements.NumbersMedium.at(Day2).DrawAt(34, screenWidth - 62 - 402);
        if (Day1 > 0) {
            elements.NumbersMedium.at(Day1).DrawAt(34, screenWidth - 62 - 342);
            elements.Minus.DrawAt(66, screenWidth - 34 - 308);
            elements.NumbersMedium.at(Month2).DrawAt(34, screenWidth - 62 - 246);
            if (Month1 > 0) {
                elements.NumbersMedium.at(Month1).DrawAt(34, screenWidth - 62 - 184);
            }
        } else {
            elements.Minus.DrawAt(66, screenWidth - 62 - 342);
            elements.NumbersMedium.at(Month2).DrawAt(34, screenWidth - 62 - 308);
            if (Month1 > 0) {
                elements.NumbersMedium.at(Month1).DrawAt(34, screenWidth - 62 - 246);
            }
        }

        elements.NumbersSmall.at(Sec2).DrawAt(714, screenWidth - 460);
        elements.NumbersSmall.at(Sec1).DrawAt(714, screenWidth - 30 * -1 - 460);

        //        elements.Shoe.DrawAt(670, screenWidth - 132);

        elements.NumbersSmall.at(Year1).DrawAt(714, screenWidth - 30 - 20);
        elements.NumbersSmall.at(Year2).DrawAt(714, screenWidth - 30 * 2 - 20);
        elements.NumbersSmall.at(Year3).DrawAt(714, screenWidth - 30 * 3 - 20);
        elements.NumbersSmall.at(Year4).DrawAt(714, screenWidth - 30 * 4 - 20);
    }

    void DigitalClock::loadTextures(const std::string &c) {
        DLOG("loading clock %s\n", c.c_str());

        std::ifstream f;
        for (int i = 0; i < 10; i++) {
            char p[256];
            sprintf(p, "%s/%s/%d_big.jpg", basePath, c.c_str(), i);
            f.open(p);
            std::string contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

            auto ft = FlatTexture();

            ft.WithMagick("JPEG")->FromData((char *)contents.c_str(), contents.size())->WithRatio(1.0f)->Load();

            elements.NumbersBig.emplace_back(ft);
            f.close();
        }

        for (int i = 0; i < 10; i++) {
            char p[256];
            sprintf(p, "%s/%s/%d_medium.jpg", basePath, c.c_str(), i);
            f.open(p);
            std::string contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

            auto ft = FlatTexture();

            ft.WithMagick("JPEG")->FromData((char *)contents.c_str(), contents.size())->WithRatio(1.0f)->Load();
            elements.NumbersMedium.emplace_back(ft);
            f.close();
        }

        for (int i = 0; i < 10; i++) {
            char p[256];
            sprintf(p, "%s/%s/%d_small.jpg", basePath, c.c_str(), i);
            f.open(p);
            std::string contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

            auto ft = FlatTexture();

            ft.WithMagick("JPEG")->FromData((char *)contents.c_str(), contents.size())->WithRatio(1.0f)->Load();
            elements.NumbersSmall.emplace_back(ft);
            f.close();
        }

        for (int i = 0; i < 7; i++) {
            char p[256];
            sprintf(p, "%s/%s/day_%d.jpg", basePath, c.c_str(), i);
            f.open(p);
            std::string contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

            auto ft = FlatTexture();

            ft.WithMagick("JPEG")->FromData((char *)contents.c_str(), contents.size())->WithRatio(1.0f)->Load();
            elements.Days.emplace_back(ft);
            f.close();
        }

        // ======
        char p[256];
        sprintf(p, "%s/%s/colon.jpg", basePath, c.c_str());
        f.open(p);
        std::string contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

        auto ft = FlatTexture();

        ft.WithMagick("JPEG")->FromData((char *)contents.c_str(), contents.size())->WithRatio(1.0f)->Load();
        elements.Colon = ft;
        f.close();

        // ======
        sprintf(p, "%s/%s/dot.jpg", basePath, c.c_str());
        f.open(p);
        std::string contents2((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

        ft.Reset();
        ft = FlatTexture();

        ft.WithMagick("JPEG")->FromData((char *)contents2.c_str(), contents2.size())->WithRatio(1.0f)->Load();
        elements.Dot = ft;
        f.close();

        // ======
        sprintf(p, "%s/%s/shoe.jpg", basePath, c.c_str());
        f.open(p);
        std::string contents3((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

        ft.Reset();
        ft = FlatTexture();

        ft.WithMagick("JPEG")->FromData((char *)contents3.c_str(), contents3.size())->WithRatio(1.0f)->Load();
        elements.Shoe = ft;
        f.close();

        // ======
        sprintf(p, "%s/%s/minus.jpg", basePath, c.c_str());
        f.open(p);
        std::string contents4((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

        ft.Reset();
        ft = FlatTexture();

        ft.WithMagick("JPEG")->FromData((char *)contents4.c_str(), contents4.size())->WithRatio(1.0f)->Load();
        elements.Minus = ft;
        f.close();
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
        loadTextures(filename);

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

        for (auto &e : elements.NumbersBig) {
            e.Unload();
        }

        for (auto &e : elements.NumbersMedium) {
            e.Unload();
        }

        for (auto &e : elements.NumbersSmall) {
            e.Unload();
        }

        for (auto &e : elements.Days) {
            e.Unload();
        }

        elements.Colon.Unload();
        elements.Dot.Unload();
        elements.Shoe.Unload();
        elements.Minus.Unload();

        elements.Days.clear();
        elements.NumbersSmall.clear();
        elements.NumbersMedium.clear();
        elements.NumbersBig.clear();

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

        // no need to pull chars from db, skin has no text
        // ascii is enough for settings

        range.AddRanges(io.Fonts->GetGlyphRangesDefault());
        range.AddChar(ImWchar(0x24c8)); // Ⓢ
        range.AddChar(ImWchar(0x2713)); // ✓

        range.BuildRanges(&gr);

        std::string fp = FontPath;
        if (exists(FontPathCustom)) {
            fp = FontPathCustom;
        }
        if (exists(FontPathCustom2)) {
            fp = FontPathCustom2;
        }

        *fontRegular = io.Fonts->AddFontFromFileTTF(fp.c_str(), fontSizeTTF, nullptr, gr.Data);

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