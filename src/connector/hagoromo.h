#ifndef WAMPY_HAGOROMO_H
#define WAMPY_HAGOROMO_H

#include "../config.h"
#include "../dac/cxd3778gf_table.h"
#include "asoundlib.h"
#include "command.pb.h"
#include "connector.h"
#include "hagoromoStatus.h"

#define ALSA_ANALOG_INPUT_OFF 0
#define ALSA_ANALOG_INPUT_TUNER 1
#define HAGOROMO_DEFAULT_VOLUME_MAX 120

namespace Hagoromo {
    // lconvert HgrmMediaPlayerApp_en_US.qm
    // FUN_001611a0 hgrm
    extern std::map<int, std::string> codecToStr;
    extern std::map<int, int> eq6PresetToHgrmIndex;
    extern std::map<int, int> vptA50SmallToHgrmIndex;
    extern std::map<int, int> dcFilterToHgrmIndex;
    extern std::map<int, int> vinylTypeToHgrmIndex;
    extern std::map<int, int> dseeModeToHgrmIndex;

    class HagoromoConnector : public Connector {
      private:
        std::mutex *m{}; // locks playlist while filtering to prevent double write on duplicate events

        void updateSoundSettings();

        void pauseIfNeeded();

        void restorePlayState();

      public:
        const char *touchscreenPath = "/sys/devices/platform/mt-i2c.1/i2c-1/1-0048/sleep";  // nw-a50/40/30/zx300
        const char *touchscreenPath2 = "/sys/devices/platform/mt-i2c.1/i2c-1/1-0020/sleep"; // wm1z
        const char *brightnessPath = "/sys/class/leds/lcd-backlight/brightness";
        const char *procMounts = "/proc/mounts";
        //        const char *thermalPath = "/sys/class/thermal/thermal_zone0/temp";

        bool *touchscreenStaysOFF{};
        bool *featureBigCover{};
        bool *featureShowTime{};
        bool *featureLimitVolume{};
        bool *featureEqPerSong{};
        bool *visualizerEnabled{};
        bool *visualizerWinampBands{};
        int prevEntryID{};
        std::string prevFilename{};

        master_volume masterVolume{};
        std::string masterVolumePath;
        master_volume_dsd masterVolumeDSD{};
        std::string masterVolumeDSDPath;
        tone_control toneControl{};
        std::string toneControlPath;
        bool tablesApplied = false;
        bool filtersApplied = false;
        AppConfig::Filters *filters;
        bool *controlFilters{};

        HagoromoStatus *hagoromoStatus = nullptr;

        PlayStateE playStateBeforeSoundSettingsUpdate = UNKNOWN;

        static void sendData(char *data, size_t len, std::string *res);

        static bool sendCMD(Command::Command *c);

        void enableTouchscreen() const;

        void disableTouchscreen() const;

        bool isOn() const;

        void TestCommand() override;

        void ToggleHgrm(HgrmToggleAction action, bool *render) override;

        void Connect() override;

        void ReadLoop() override;

        void PollStatus() override;

        __attribute__((unused)) void setVolumeALSA(int i) {};

        void SetVolume(int i, bool relative) override;

        void SetPosition(int percent) override;

        void SetBalance(int i) override;

        void SetShuffle(int i) override;

        void SetRepeat(int i) override;

        void PrevTrack() override;

        void Play() override;

        void Pause() override;

        void Stop() override;

        void NextTrack() override;

        void powerLoop(bool *render, bool *power) override;

        void storageLoop();

        void volumeLoop() override;

        void FeatureBigCover(bool enable) override;

        void FeatureShowTime(bool enable) override;

        void FeatureSetMaxVolume(bool enable) override;

        void SetClearAudio(bool enable) override;

        void SetEqBands(std::vector<double> bandValueList) override;

        void SetEqPreset(int index) override;

        void SetVPT(bool enable) override;

        void SetVPTPreset(int preset) override;

        void SetDsee(bool enable) override;

        void SetDCPhase(bool enable) override;

        void SetDCPhasePreset(int preset) override;

        void SetVinyl(bool enable) override;

        void SetDirectSource(bool enable) override;

        void SetToneControlValues(const std::vector<int> &v) override;

        void SetToneControlOrEQ(int eqType) override;

        void SetDseeCust(bool enable) override;

        void SetDseeCustMode(int mode) override;

        void SetVinylMode(int mode) override;

        void Start() override;

        void getSongData(int entryid, SongInfo *s) override;
    };
} // namespace Hagoromo

#endif // WAMPY_HAGOROMO_H
