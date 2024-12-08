#ifndef WAMPY_HAGOROMO_H
#define WAMPY_HAGOROMO_H

#include "command.pb.h"
#include "connector.h"

namespace Hagoromo {
    struct featureConfig {
        bool cover{};
        bool coverOK{};
        bool time{};
        bool timeOK{};
    };

    class HagoromoConnector : public Connector {
      public:
        const char *touchscreenPath = "/sys/devices/platform/mt-i2c.1/i2c-1/1-0048/sleep";
        const char *brightnessPath = "/sys/class/leds/lcd-backlight/brightness";
        featureConfig config{};
        //        const char *thermalPath = "/sys/class/thermal/thermal_zone0/temp";

        static void sendData(char *data, size_t len, std::string *res);

        static bool sendCMD(Command::Command *c);

        void enableTouchscreen() const;

        void disableTouchscreen() const;

        bool isOn() const;

        void TestCommand() override;

        void ToggleHgrm(HgrmToggleAction action, bool *render) override;

        void Connect() override;

        [[noreturn]] void ReadLoop() override;

        void PollStatus() override;

        __attribute__((unused)) void setVolumeALSA(int i){};

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

        __attribute__((unused)) void volumeLoop() override{};

        void FeatureBigCover(bool enable) override;

        void FeatureShowTime(bool enable) override;

        void Start() override;

        void featuresLoop();
    };
} // namespace Hagoromo

#endif // WAMPY_HAGOROMO_H
