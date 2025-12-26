#ifndef WAMPY_SOUND_SETTINGS_FW_H
#define WAMPY_SOUND_SETTINGS_FW_H

#include "sound_service_fw.h"

#include <libintl.h>

class SoundSettingsFww {
  public:
    sound_settings_fw *s;

    std::vector<std::pair<std::string, int>> eq10bands = {
        {"31", 0}, {"62", 0}, {"125", 0}, {"250", 0}, {"500", 0}, {"1K", 0}, {"2K", 0}, {"4K", 0}, {"8K", 0}, {"16K", 0}};
    std::vector<std::pair<std::string, int>> eqtone = {{gettext("Bass"), 0}, {gettext("Middle"), 0}, {gettext("Treble"), 0}};
    std::vector<std::pair<std::string, int>> eqtoneFreq = {{gettext("Bass"), 2}, {gettext("Middle"), 2}, {gettext("Treble"), 2}};
    std::vector<std::pair<std::string, int>> eq6bands = {
        {"60Hz", 0}, {"400Hz", 0}, {"1kHz", 0}, {"2.5kHz", 0}, {"6.3kHz", 0}, {"16kHz", 0}};
    int vinylizerValue = 1;
    int vptValue = 1;
    int dcValue = 1;
    int eq6Value = 2;

    std::map<std::string, bool> filters;

    const std::string filterInvalid = gettext("invalid filter");

    void Start();

    void Send() const;

    void Print();

    void Update();

    void SetFilter(const std::string &name, bool value) const;

    void SetEq10Band(int band, int value) const;

    void SetEq6Band(int band, int value) const;

    void SetEq6Preset(int value) const;

    void SetEqTone(int type, int value) const;

    void SetEqToneFreq(int type, int value) const;

    void SetVinylizerType(int value) const;

    void SetVptMode(int value) const;

    void SetDcFilterType(int value) const;

    void SetDirectSource(bool value) const;

    void SetClearAudioPlus(bool value) const;
};

#endif // WAMPY_SOUND_SETTINGS_FW_H
