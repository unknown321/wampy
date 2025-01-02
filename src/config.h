#ifndef WAMPY_CONFIG_H
#define WAMPY_CONFIG_H

#include "cassette/cassette.h"
#include "digital_clock/digital_clock.h"
#include "fontranges.h"
#include "mini/ini.h"
#include "w1/w1.h"
#include "winamp/winamp.h"
#include <string>

enum ESkinVariant {
    EMPTY,
    WINAMP,
    CASSETTE,
    DIGITAL_CLOCK,
};

namespace AppConfig {
    struct Misc {
        bool swapTrackButtons = false;
    };

    struct Features {
        bool bigCover = false;
        bool showTime = false;
        bool limitVolume = false;
        bool touchscreenStaysOFF = false;
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
        FontRanges fontRanges;
        int badBoots{};
        bool limitFPS = false;
        std::string forceConnector{};

        int FindConfig();

        int Load();

        int Create();

        int Save();

        void Default();

        void ToIni();
    };
} // namespace AppConfig

#endif