#ifndef WAMPY_WINAMP_H
#define WAMPY_WINAMP_H

#include "../skinVariant.h"

#include "../skinElement.h"
#include "imgui.h"
#include <map>
#include <string>

namespace Stopwatch {
    template <typename Clock = std::chrono::high_resolution_clock> class Stopwatch {
        typename Clock::time_point start_point;
        bool running = false;

      public:
        template <typename Rep = typename Clock::duration::rep, typename Units = typename Clock::duration> Rep elapsed_time() const {
            auto counted_time = std::chrono::duration_cast<Units>(Clock::now() - start_point).count();
            return static_cast<Rep>(counted_time);
        }

        void Start() {
            start_point = Clock::now();
            running = true;
        }

        bool Running() { return running; }

        void Stop() { running = false; }
    };

    using precise_stopwatch = Stopwatch<>;
    using system_stopwatch = Stopwatch<std::chrono::system_clock>;
    using monotonic_stopwatch = Stopwatch<std::chrono::steady_clock>;
} // namespace Stopwatch

namespace Winamp {

    struct SkinColors {
        std::string PlaylistNormalText;
        std::string PlaylistCurrentText;
        std::string PlaylistNormalBG;
        std::string PlaylistSelectedBG;
        ImU32 PlaylistNormalTextU32;
        ImU32 PlaylistCurrentTextU32;

        Magick::Color trackTitleBackground = {0, 0, 0, 0};
    };

    struct elements {
        FlatTexture Main;
        FlatTexture Title;
        FlatTexture OptionsIndicators;
        FlatTexture MonoOffIndicator;
        FlatTexture MonoOnIndicator;
        FlatTexture StereoOnIndicator;
        FlatTexture StereoOffIndicator;

        FlatTexture StopIndicator;
        FlatTexture PlayIndicator;
        FlatTexture PauseIndicator;
        FlatTexture BufferingIndicator;

        FlatTexture PlaylistTitleBarLeftCorner;
        FlatTexture PlaylistTitleBarFiller;
        FlatTexture PlaylistTitleBarTitle;
        FlatTexture PlaylistLeftBorder;
        FlatTexture PlaylistRightBorder;
        FlatTexture PlaylistScrollButton;
        FlatTexture PlaylistBG;

        Button ShuffleButton;
        Button EQButton;
        Button RepeatButton;
        Button PlaylistButton;
        Button EjectButton;
        Button PrevButton;
        Button PlayButton;
        Button PauseButton;
        Button StopButton;
        Button NextButton;

        Button PlaylistTitleBarRightCornerButton;

        Button TrackTimeToggle;

        Slider PositionSlider;
        Slider VolumeSlider;
        Slider BalanceSlider;

        void Unload();
    };

    struct Config {
        bool useBitmapFontInPlaylist{};
        bool useBitmapFont{};
        std::string filename{};

        void Default();

        static Config GetDefault();
    };

    class Winamp : public SkinVariant {
      public:
        Winamp() = default;

        Config *config{};

        void WithConfig(Config *c);

        static void SeekPressed(void *winampSkin, void *i);

        static void SeekReleased(void *winampSkin, void *i);

        static void VolumePressed(void *winampSkin, void *i);

        static void VolumePressedMPD(void *winampSkin, void *i);

        static void VolumeReleased(void *winampSkin, void *i);

        static void VolumeReleasedMPD(void *winampSkin, void *i);

        static void VolumePressedHagoromo(void *winampSkin, void *i);

        static void VolumeReleasedHagoromo(void *winampSkin, void *i);

        static void BalancePressed(void *winampSkin, void *i);

        static void BalanceReleased(void *winampSkin, void *i);

        void Draw() override;

        int Load(std::string filename, ImFont *FontRegular) override;

        int AddFonts(ImFont *fontRegular);

        static void Stop(void *winamp, void *);

        static void Play(void *winamp, void *);

        static void Pause(void *winamp, void *);

        static void Prev(void *winamp, void *);

        static void Next(void *winamp, void *);

        void Unload();

      private:
        SkinColors colors{};
        TextureMap textures{};
        std::string newFilename{};
        bool MarqueeRunning{};
        std::thread::native_handle_type marqueeThread{};

        std::string savedTitle{};
        std::string newFull{};

        ImFont *FontBitmap{};
        ImFont *FontNumbers{};
        ImFont *FontRegular{};

        bool playlistFullscreen{};
        bool negativeTime = false;
        bool isEx = false;
        elements Elements{};
        bool stopped{};
        Stopwatch::Stopwatch<> stopwatch;

        int volumeIsBalance();

        void probeTrackTitleBackgroundColor();

        Fonts addFont(const std::string &ttfFontPath, TextureMapEntry fontNumbers, TextureMapEntry fontRegular) const;

        static int unzip(const std::string &filename, TextureMap *textures, std::string *status);

        void freeUnzippedTextures();

        void readPlEdit();

        void drawPlaylist() const;

        int initializeElements();

        int initializeButtons();

        int initializePlaylist();

        int initializeSliders();

        static void notImplemented(void *winampSkin, void *);

        void drawTime();

        void blinkTrackTime();

        static void toggleTrackTime(void *arg, void *i);

        static void togglePlaylistFullscreen(void *arg, void *);

        void MarqueeInFrame();

        void MarqueeBitmap();

        [[noreturn]] void Marquee();

        void MarqueeThread();

        void Format();
    };
} // namespace Winamp

#endif // WAMPY_WINAMP_H
