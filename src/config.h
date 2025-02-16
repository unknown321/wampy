#ifndef WAMPY_CONFIG_H
#define WAMPY_CONFIG_H

#include "cassette/config.h"
#include "digital_clock/config.h"
#include "mini/ini.h"
#include "w1/w1.h"
#include "winamp/config.h"
#include <string>

enum ESkinVariant {
    EMPTY,
    WINAMP,
    CASSETTE,
    DIGITAL_CLOCK,
};

enum EWindowOffset {
    EWindowOffset_UNKNOWN = 0,
    EWindowOffset_LEFT = 1,
    EWindowOffset_CENTER = 2,
    EWindowOffset_RIGHT = 3,
};

extern std::map<EWindowOffset, std::string> WindowOffsetToString;

namespace AppConfig {
    struct Misc {
        bool swapTrackButtons = false;
    };

    struct Features {
        bool bigCover = false;
        bool showTime = false;
        bool limitVolume = false;
        bool touchscreenStaysOFF = false;
        bool eqPerSong = false;
    };

    class AppConfig {
      public:
        AppConfig() = default;

        const char *filePath = "";
        mINI::INIStructure ini{};
        std::string MPDSocketPath{};
        ESkinVariant activeSkin{};
        Winamp::Config winamp{};
        Cassette::Config cassette{};
        DigitalClock::Config digitalClock{};
        Misc misc{};
        Features features{};
        W1::W1Options w1Options{};
        bool debug{};
        int badBoots{};
        bool limitFPS = false;
        std::string forceConnector{};
        EWindowOffset windowOffset = EWindowOffset_LEFT;
        std::vector<int> fmPresets{};
        bool showFmInSettings = true;

        int FindConfig();

        int Load();

        int Create();

        int Save();

        void Default();

        void ToIni();
    };
} // namespace AppConfig

#endif