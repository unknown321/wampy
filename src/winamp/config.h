#ifndef WAMPY_WINAMP_CONFIG_H
#define WAMPY_WINAMP_CONFIG_H
#include "string"

#define WINAMP_VISUALIZER_SENS_MIN 0.001f
#define WINAMP_VISUALIZER_SENS_MAX 0.1f
#define WINAMP_VISUALIZER_SENS_DEFAULT 0.03f;

namespace Winamp {
    struct Config {
        bool useBitmapFontInPlaylist{};
        bool useBitmapFont{};
        bool preferTimeRemaining{};
        bool showClutterbar = true;
        bool skinTransparency = true;
        bool visualizerEnable = false;
        bool visualizerWinampBands = false;
        float visualizerSensitivity = WINAMP_VISUALIZER_SENS_DEFAULT;
        std::string filename{};

        void Default();

        static Config GetDefault();
    };
} // namespace Winamp
#endif // WAMPY_CONFIG_H
