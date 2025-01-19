#ifndef WAMPY_WINAMP_H
#define WAMPY_WINAMP_H

#include "../skinVariant.h"

#include "../playlist.h"
#include "../skinElement.h"
#include "imgui.h"
#include <map>
#include <string>

#define PLAYLIST_SONG_SIZE 2048
#define PLAYLIST_DURATION_SIZE 10

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
    const float fontSizeTTF = 33.0f;

    struct MarqueeInfo {
        int start;
        char format[10];
        char text[PLAYLIST_SONG_SIZE];  // text that will be displayed on screen (duplicated song title plus separators if marquee)
        char title[PLAYLIST_SONG_SIZE]; // song title
        bool updated{};
    };

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
        FlatTexture RegionMask;
        FlatTexture ClutterBar;
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
        bool preferTimeRemaining{};
        bool showClutterbar = true;
        bool skinTransparency = true;
        std::string filename{};

        void Default();

        static Config GetDefault();
    };

    struct PlaylistSong {
        char text[PLAYLIST_SONG_SIZE]{};
        char duration[PLAYLIST_DURATION_SIZE]{};
        float durationSize{};
    };

    class Winamp : public SkinVariant, public INotifiable {
      public:
        Config *config{};
        bool *eqEnabled{};

        Winamp();
        Winamp(Winamp const &other);
        Winamp &operator=(Winamp const &other);

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

        int Load(std::string filename, ImFont **FontRegular) override;

        void AddFonts(ImFont **fontRegular);

        static void Stop(void *winamp, void *);

        static void Play(void *winamp, void *);

        static void Pause(void *winamp, void *);

        static void Prev(void *winamp, void *);

        static void Next(void *winamp, void *);

        void Unload();

        void Notify() override;

        void Format(bool force = false);

        void changeSkin(const std::string &newSkinPath);

        void loadNewSkin(bool force = false);

        ImFont *GetFont();

        std::string GetCurrentSkin();

      private:
        SkinColors colors{};
        TextureMap textures{};
        elements Elements{};
        bool MarqueeRunning{};
        bool childThreadsStop{};
        bool marqueeThreadRunning{};
        bool updateThreadRunning{};

        ImFont *FontBitmap{};
        ImFont *FontNumbers{};
        ImFont *FontRegular{};

        bool playlistFullscreen{};
        bool isEx{};             // uses nums_ex.bmp?
        bool timeRemaining{};    // current state of time display
        bool timeRemainingSet{}; // option from config, must be set only once on changing skinVariant (cassette-winamp), wsz skin change
                                 // does not reset current value
        const char *remainingTimeSign{};
        bool stopped{};
        Stopwatch::Stopwatch<> stopwatch;

        PlaylistSong playlist[PLAYLIST_SIZE]{};
        bool titleIsMarquee{};
        char systemMessage[256]{};
        int minute1{};
        int minute2{};
        int second1{};
        int second2{};

        std::mutex statusUpdatedM;
        bool statusUpdated{}; // set when connector sends an update notification

        // marquee thread runs independently of update thread
        // because of that we need two marquee info instances and a lot of logic
        // I'm so sorry
        MarqueeInfo m{};
        MarqueeInfo mStaging{};

        std::string currentSkin{};
        std::string newSkin{};

        std::vector<int> pointList{};

        int volumeTextureIsBalance();

        void probeTrackTitleBackgroundColor();

        Fonts addFont(const std::string &ttfFontPath, TextureMapEntry fontNumbers, TextureMapEntry fontRegular) const;

        static int unzip(const std::string &filename, TextureMap *textures, std::string *status);

        void freeUnzippedTextures();

        void readPlEdit();
        void readRegionTxt();

        void drawPlaylist() const;

        void initializeElements();

        void initializeButtons();

        void initializePlaylist();

        void initializeSliders();

        static void notImplemented(void *winampSkin, void *);

        void drawTime();

        void blinkTrackTime();

        static void toggleTrackTime(void *arg, void *i);

        static void togglePlaylistFullscreen(void *arg, void *);

        void MarqueeCalculate();

        __attribute__((unused)) void MarqueeBitmap();

        void MarqueeLoop();

        void StartThreads();

        void processUpdate();

        void formatDuration();

        void formatPlaylist();

        void prepareForMarquee();
    };

} // namespace Winamp

#endif // WAMPY_WINAMP_H
