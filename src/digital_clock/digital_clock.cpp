#include "digital_clock.h"
#include "../skin.h"
#include "imgui_impl_opengl3.h"
#include "langToString/langToString.h"
#include "wstring.h"
#include <memory>
#include <stdexcept>

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

    std::map<std::string, std::string> colorsDigitalClock = {{"Silver", "silver"},
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
        {"Segments Amber LCD", "segments-amberlcd"},
        {"Segments Green LCD", "segments-greenlcd"},
        {"Segments Grey LCD", "segments-greylcd"},
        {"Segments Amber LCD Regular", "segments-amberlcd-regular"},
        {"Segments Green LCD Regular", "segments-greenlcd-regular"},
        {"Segments Grey LCD Regular", "segments-greylcd-regular"},
        {"Segments Default", "segments-default"},
        {"Segments Peach", "segments-peach"},
        {"Segments Red", "segments-red"},
        {"Segments Blue", "segments-blue"},
        {"Segments Green", "segments-green"},
        {"Segments Default Regular", "segments-default-regular"},
        {"Segments Peach Regular", "segments-peach-regular"},
        {"Segments Red Regular", "segments-red-regular"},
        {"Segments Blue Regular", "segments-blue-regular"},
        {"Segments Green Regular", "segments-green-regular"}};
    std::string DefaultColor = "silver";
    std::string DefaultColorPreview = "Silver";

    void DigitalClock::Draw() {
        if (loading) {
            return;
        }

        if (!loadStatusStr.empty()) {
            ImGui::TextWrapped("%s", loadStatusStr.c_str());
            if (ImGui::Button("Settings")) Skin::Skin::ToggleDrawSettings(skin, nullptr);
            return;
        }
        time_t now=time(nullptr);
        if(now!=rawtime){rawtime=now;gmtime_r(&rawtime,&timeSnapshot);timeinfo=&timeSnapshot;formatTime();}
        if (hasBackground) {
            elements.Background.DrawAt(0, 0);
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
        const std::string dir = std::string(basePath) + "/" + c + "/";
        auto load = [&](const std::string &name) {
            const std::string path = dir + name + ".jpg";
            std::ifstream file(path, std::ios::binary);
            if (!file) throw std::runtime_error("missing clock image: " + name);
            std::string bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            if (bytes.empty()) throw std::runtime_error("empty clock image: " + name);
            std::unique_ptr<Magick::Image> image(new Magick::Image());
            image->magick("JPEG"); image->read(Magick::Blob(bytes.data(), bytes.size()));
            if (!image->isValid() || !image->columns() || !image->rows())
                throw std::runtime_error("invalid clock image: " + name);
            FlatTexture texture; texture.image=image.get(); texture.ratio=1.0f;
            try {
                texture.Upscale(); texture.LoadTexture();
            } catch (...) {
                texture.Unload(); throw;
            }
            texture.image=nullptr; // image stays owned by this scope, GL texture transfers to elements
            return texture;
        };
        hasBackground=false;
        if (exists(dir+"background.jpg")) { elements.Background=load("background"); hasBackground=true; }
        // Reserve before creating GL textures so vector growth cannot orphan one.
        elements.NumbersBig.reserve(10); elements.NumbersMedium.reserve(10);
        elements.NumbersSmall.reserve(10); elements.Days.reserve(7);
        for (int i=0;i<10;++i) elements.NumbersBig.push_back(load(std::to_string(i)+"_big"));
        for (int i=0;i<10;++i) elements.NumbersMedium.push_back(load(std::to_string(i)+"_medium"));
        for (int i=0;i<10;++i) elements.NumbersSmall.push_back(load(std::to_string(i)+"_small"));
        for (int i=0;i<7;++i) elements.Days.push_back(load("day_"+std::to_string(i)));
        elements.Colon=load("colon"); elements.Minus=load("minus");
        // dot and shoe are not drawn; their absence must not fail the clock.
    }

    int DigitalClock::Load(std::string filename, ImFont **fontRegular) {
        Unload();
        loading = true;
        loadStatusStr.clear();
        FontRegular = *fontRegular;

        if (filename.empty()) {
            filename = DefaultColor;
        }

        bool found=false;
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

        try {
            initializeButton();
            addFonts(fontRegular);
            FontRegular = *fontRegular;
            loadTextures(filename);
        } catch (const std::exception &error) {
            loadStatusStr = "Clock resources could not be loaded. Open Settings to change skin.";
            DLOG("clock load failed: %s\n", error.what());
            Unload(); loading = false; return -1;
        }

        color = filename;

        time(&rawtime);
        gmtime_r(&rawtime,&timeSnapshot);
        timeinfo=&timeSnapshot;
        formatTime();

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
        Year3 = ((timeinfo->tm_year + 1900) / 10) % 10;
        Year4 = (timeinfo->tm_year + 1900) % 10;

        Month1 = (timeinfo->tm_mon + 1) / 10;
        Month2 = (timeinfo->tm_mon + 1) % 10;
        Day1 = timeinfo->tm_mday / 10;
        Day2 = timeinfo->tm_mday % 10;

        Sec1 = timeinfo->tm_sec / 10;
        Sec2 = timeinfo->tm_sec % 10;
    }

    void DigitalClock::LoadNewSkin() {
        if (color == newColor) {
            DLOG("not updating current skin %s\n", color.c_str());
            return;
        }

        Load(newColor, &FontRegular);
        config->color = color;
    }

    ImFont *DigitalClock::GetFont() { return FontRegular; }

    void DigitalClock::Unload() {

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

        if (hasBackground) {
            elements.Background.Unload();
        }
        hasBackground=false;

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

        // current tl must have all characters
        auto res = ReadFile(localeDir + "/" + config->language + "/LC_MESSAGES/range");
        size_t index = 0;
        while (index < res.size()) {
            auto v = utfToPoint(res, index);
            range.AddChar(v);
        }

        // make sure language name is displayed correctly
        for (const auto &q : LangToString) {
            size_t index2 = 0;
            while (index2 < q.second.size()) {
                auto v = utfToPoint(q.second, index2);
                range.AddChar(v);
            }
        }

        range.BuildRanges(&gr);

        std::string fp = FontPath;
        if (exists(FontPathCustom)) {
            DLOG("using custom font %s\n", FontPathCustom.c_str());
            fp = FontPathCustom;
        }

        if (exists(FontPathCustom2)) {
            DLOG("using custom font %s\n", FontPathCustom2.c_str());
            fp = FontPathCustom2;
        }

        *fontRegular = exists(fp) ? io.Fonts->AddFontFromFileTTF(fp.c_str(), fontSizeTTF, nullptr, gr.Data) : nullptr;
        if (!*fontRegular) *fontRegular = io.Fonts->AddFontDefault();

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