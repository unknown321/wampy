#ifndef WAMPY_CONFIG_H
#define WAMPY_CONFIG_H

#include "cassette/cassette.h"
#include "fontranges.h"
#include "mini/ini.h"
#include "winamp/winamp.h"
#include <string>

enum ESkinVariant {
    EMPTY,
    WINAMP,
    CASSETTE,
};

namespace AppConfig {
    struct Debug {
        bool enabled = false;
    };

    struct Misc {
        bool swapTrackButtons = false;
    };

    struct Features {
        bool bigCover = false;
        bool showTime = false;
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
        Misc misc{};
        Features features{};
        Debug debug{};
        FontRanges fontRanges;

        int FindConfig();

        int Load();

        int Create();

        int Save();

        void Default();

        void ToIni();
    };
} // namespace AppConfig

#endif