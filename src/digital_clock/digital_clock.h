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
        Button ToggleSettings;
        Atlas atlas;
    };

    enum NumType { NUM_BIG = 1, NUM_MEDIUM = 2, NUM_SMALL = 3 };

    const int dotIndex = 27;
    const int minusIndex = 28;
    const int shoeIndex = 29;
    const int colonIndex = 30;

    const int daysIndex = 10;

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

        void loadAtlas(const std::string &c);

        void ticker();

        void formatTime();

        void drawNum(NumType t, uint value, ImVec2 pos);

        void drawFromAtlas(uint index, ImVec2 pos);

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