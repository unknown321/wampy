#ifndef WAMPY_CONNECTOR_H
#define WAMPY_CONNECTOR_H

#include "../INotifiable.h"
#include "../playlist.h"
#include "../util/util.h"
#include "song.h"
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
    int Elapsed{};
    int Bitrate{};
    int Volume{}; // percentage
    int Shuffle{};
    int Repeat{};
    int songID{};
    int Bits{};
    int Channels{};
    int SampleRate{};
    std::string Codec;
    int PositionPercent{};
    std::string SampleRateString;
    std::string BitrateString;
    std::string State;
    int PlaylistUpdateTimestamp{};

    statusFields sf{};

    int Balance{};
};

struct Connector {
    const char *address{};
    std::vector<Song> playlist{};
    Status status{};
    bool *render{};
    bool power = true;
    bool serverReady{};
    bool updateVolume{};
    // it takes ~2 status calls to update current track position on hagoromo side
    // without this delay seek button will violently jump to previous position
    unsigned int updateElapsedCounter{};

    std::string stateString;

    std::vector<INotifiable *> clients;

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

    virtual void Start() {
        Connect();
        auto exec_run = [this]() { ReadLoop(); };

        std::thread t(exec_run);
        t.detach();
    }
};

#endif