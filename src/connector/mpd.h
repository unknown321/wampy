#ifndef WAMPY_MPD_H
#define WAMPY_MPD_H

#include "connector.h"

#define MPDDefaultAddress "/home/unknown/.local/mpd/socket";

namespace MPD {
    struct MPDConnector : Connector {
        int fd = 0;

        void Connect() override;

        void Send(const char *cmd) const;

        static void resetEmptyFields(Song *song, statusFields *sf);

        static void parseCurrentSong(Song *song, const std::vector<std::string> &words, statusFields *sf);

        static void parseStatus(Status *status, const std::vector<std::string> &words);

        static bool parsePlaylistWord(const std::vector<std::string> &words, Song *song);

        static void parseResponse(char *buf, int BUF_SIZE, Status *status);

        void parsePlaylist(char *buf, int BUF_SIZE, std::vector<Song> *playlist);

        void TestCommand() override;

        void ToggleHgrm(HgrmToggleAction action, bool *render) override;

        void ReadLoop() override;

        void PollStatus() override;

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

        __attribute__((unused)) void volumeLoop() override;

        void FeatureBigCover(bool enable) override{};

        void FeatureShowTime(bool enable) override{};
    };
} // namespace MPD

#endif // WAMPY_MPD_H
