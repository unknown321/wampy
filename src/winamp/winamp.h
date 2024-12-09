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

    struct MarqueeInfo {
        int start;
        char format[10];
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

        int Load(std::string filename, ImFont *FontRegular) override;

        int AddFonts(ImFont *fontRegular);

        static void Stop(void *winamp, void *);

        static void Play(void *winamp, void *);

        static void Pause(void *winamp, void *);

        static void Prev(void *winamp, void *);

        static void Next(void *winamp, void *);

        void Unload();

        void Notify() override;

      private:
        SkinColors colors{};
        TextureMap textures{};
        elements Elements{};
        std::string newFilename{};
        bool MarqueeRunning{};
        bool marqueeThreadStop{};

        ImFont *FontBitmap{};
        ImFont *FontNumbers{};
        ImFont *FontRegular{};

        bool playlistFullscreen{};
        bool isEx{};             // uses nums_ex.bmp?
        bool timeRemaining{};    // current state of time display
        bool timeRemainingSet{}; // option from config, must be set only once on start, skin change does not reset current value
        const char *remainingTimeSign{};
        bool stopped{};
        int activePlaylistID{}; // used to swap currently updated playlist with displayed playlist
        Stopwatch::Stopwatch<> stopwatch;

        PlaylistSong playlist[2][PLAYLIST_SIZE]{};
        bool titleIsMarquee{};
        char currentSongTitle[PLAYLIST_SONG_SIZE]{};
        char currentSongTitleMarquee[PLAYLIST_SONG_SIZE]{}; // holds full text for displayed title
        char systemMessage[256]{};
        int minute1{};
        int minute2{};
        int second1{};
        int second2{};

        std::mutex statusUpdatedM;
        bool statusUpdated{}; // set when connector sends an update notification

        MarqueeInfo m{};

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

        void MarqueeCalculate();

        __attribute__((unused)) void MarqueeBitmap();

        void MarqueeLoop();

        void MarqueeThread();

        void Format();

        void processUpdate();

        void formatDuration();

        void formatPlaylist();

        void prepareForMarquee();
    };

} // namespace Winamp

#endif // WAMPY_WINAMP_H
