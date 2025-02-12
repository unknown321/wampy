#ifndef WAMPY_CONNECTOR_H
#define WAMPY_CONNECTOR_H

#include "../INotifiable.h"
#include "../playlist.h"
#include "../sound_settings/sound_settings.h"
#include "../sound_settings_fw/sound_settings_fw.h"
#include "../util/util.h"
#include "hagoromoStatus.h"
#include "song.h"
#include "sound_service_fw.h"
#include <cmath>
#include <mutex>
#include <vector>

#ifdef DESKTOP
#include <thread>
#else
#include <future>
#endif

enum HgrmToggleAction {
    Show = 0,
    Hide = 1,
};

struct statusFields {
    bool Artist;
    bool Album;
    bool Title;
    bool Track;
    bool Audio;
    bool Date;
    bool File;
    bool Duration;
    bool Bitrate;
    bool Elapsed;
    bool Volume;
};

struct Status {
    int Duration{};
    int Elapsed{}; // seconds
    int Bitrate{};
    int Volume{};    // percentage
    int VolumeRaw{}; // as is
    int Shuffle{};
    int Repeat{};
    int songID{};
    int Bits{};
    int Channels{};
    int SampleRate{};
    std::string Codec{};
    int PositionPercent{};
    std::string SampleRateString{};
    std::string BitrateString{};
    std::string Filename{};
    std::string Album{};
    std::string TrackNumber{};
    std::string Date{};
    PlayStateE State{};

    statusFields sf{};

    int Balance{};
};

struct SongInfo {
    std::string Filename;
    std::string Title;
    std::string Artist;
    std::string Album;
    int Year;
    int TrackNumber;
    int Duration;
    int SampleRate;
    int BitDepth;
    int Bitrate;
    int TotalTracks;
    int Codec;
};

struct Connector {
    const char *address{};
    std::vector<Song> playlist{};
    Status status{};
    bool *render{};
    bool power = true;
    bool updateVolume{};
    bool storagePresent{};
    // it takes ~2 status calls to update current track position on hagoromo side
    // without this delay seek button will violently jump to previous position
    unsigned int updateElapsedCounter{};

    std::string stateString;

    std::vector<INotifiable *> clients;

    SoundSettings soundSettings{};

    SoundSettingsFww soundSettingsFw{};

    Connector() {
        for (int i = 0; i < PLAYLIST_SIZE; i++) {
            playlist.emplace_back();
        }

        updateVolume = true;
        updateElapsedCounter = 0;
    }

    virtual void Connect() = 0;

    virtual void ReadLoop() = 0;

    virtual void PollStatus() = 0;

    virtual void SetVolume(int, bool relative) = 0;

    static void SetVolume(void *arg, int i, bool relative) {
        auto *p = (Connector *)arg;
        p->SetVolume(i, relative);
    }

    virtual void SetPosition(int) = 0;

    static void SetPosition(void *arg, int i) {
        auto *p = (Connector *)arg;
        p->SetPosition(i);
    }

    virtual void SetBalance(int) = 0;

    virtual void SetShuffle(int) = 0;

    static void ToggleShuffle(void *arg, void *i) {
        auto *p = (Connector *)arg;
        auto val = *(int *)i;
        if (val == 0) {
            val = 1;
        } else {
            val = 0;
        }

        p->SetShuffle(val);
    }

    virtual void SetRepeat(int) = 0;

    static void SetRepeat(void *arg, void *i) {
        auto *p = (Connector *)arg;
        p->SetRepeat(*(int *)i);
    }

    virtual void PrevTrack() = 0;

    static void Prev(void *arg, void *i) {
        auto *p = (Connector *)arg;
        p->PrevTrack();
    }

    virtual void Play() = 0;

    static void Play(void *arg, void *i) {
        auto *p = (Connector *)arg;
        p->Play();
    }

    virtual void Pause() = 0;

    static void Pause(void *arg, void *i) {
        auto *p = (Connector *)arg;
        p->Pause();
    }

    virtual void Stop() = 0;

    static void Stop(void *arg, void *i) {
        auto *p = (Connector *)arg;
        p->Stop();
    }

    virtual void NextTrack() = 0;

    static void Next(void *arg, void *i) {
        auto *p = (Connector *)arg;
        p->NextTrack();
    }

    virtual void ToggleHgrm(HgrmToggleAction action, bool *render) = 0;

    virtual void volumeLoop() = 0;

    virtual void powerLoop(bool *render, bool *power) = 0;

    virtual void TestCommand() = 0;

    virtual void FeatureBigCover(bool enable) = 0;

    virtual void FeatureShowTime(bool enable) = 0;

    virtual void FeatureSetMaxVolume(bool enable) = 0;

    virtual void SetClearAudio(bool enable) = 0;

    virtual void SetEqBands(std::vector<double>) = 0;

    virtual void SetEqPreset(int preset) = 0;

    virtual void SetVPT(bool enable) = 0;

    virtual void SetVPTPreset(int preset) = 0;

    virtual void SetDsee(bool enable) = 0;

    virtual void SetDCPhase(bool enable) = 0;

    virtual void SetDCPhasePreset(int preset) = 0;

    virtual void SetVinyl(bool enable) = 0;

    virtual void SetDirectSource(bool enable) = 0;

    virtual void SetToneControlValues(const std::vector<int> &v) = 0;

    virtual void SetToneControlOrEQ(int eqType) = 0;

    virtual void SetDseeCust(bool enable) = 0;

    virtual void SetDseeCustMode(int mode) = 0;

    virtual void SetVinylMode(int mode) = 0;

    virtual void getSongData(int entryId, SongInfo *s) = 0;

    virtual void Start() {
        Connect();
        auto exec_run = [this]() { ReadLoop(); };

        std::thread t(exec_run);
        t.detach();
    }
};

#endif