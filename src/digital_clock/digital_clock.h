#ifndef WAMPY_DIGITAL_CLOCK_H
#define WAMPY_DIGITAL_CLOCK_H

#include "../skinElement.h"
#include "../skinVariant.h"
#include <thread>
#include <utility>

#define FIELD_SIZE 2048

namespace DigitalClock {

    extern std::map<std::string, std::string> colorsDigitalClock;
    extern std::string DefaultColor;

    const float fontSizeTTF = 34.0f;

    struct Config {
        std::string color{};
        void Default() { color = "silver"; };
    };

    struct DigitalClockElements {
        std::vector<FlatTexture> NumbersBig;
        std::vector<FlatTexture> NumbersMedium;
        std::vector<FlatTexture> NumbersSmall;
        std::vector<FlatTexture> Days;
        FlatTexture Colon;
        FlatTexture Dot;
        FlatTexture Shoe;
        FlatTexture Minus;
        Button ToggleSettings;
    };

    class DigitalClock : public SkinVariant {
      private:
        DigitalClockElements elements;
        std::string color;
        std::string newColor;
        bool tickerThreadRunning{};
        bool childThreadsStop{};
        bool loading{};
        ImFont *FontRegular{};
        Config *config{};
        time_t rawtime{};
        tm *timeinfo{};

        uint H1{};
        uint H2{};
        uint M1{};
        uint M2{};
        uint Day1{};
        uint Day2{};
        uint Month1{};
        uint Month2{};
        uint Year1{};
        uint Year2{};
        uint Year3{};
        uint Year4{};
        uint Sec1{};
        uint Sec2{};

        void initializeButton();

        int addFonts(ImFont **fontRegular);

        void loadTextures(const std::string &color);

        void ticker();

        void formatTime();

      public:
        std::string GetColor() { return color; };

        void SetColor(std::string s) { newColor = std::move(s); };

        std::string GetActiveColorPreview();
        static std::string GetColorPreview(const std::string &);

        DigitalClock() = default;

        void Draw() override;

        int Load(std::string filename, ImFont **FontRegular) override;

        void Unload();

        void LoadNewSkin();

        ImFont *GetFont();

        void WithConfig(Config *c);
    };
} // namespace DigitalClock
#endif // WAMPY_DIGITAL_CLOCK_H