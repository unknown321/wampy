#include "command.pb.h"
#include "command_names.h"
#include <cstdio>
#include <cstdlib>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

// #include "alsa/asoundlib.h"
#include "asoundlib.h"
#include "hagoromo.h"
#include "sqlite3.h"
#include "wampy.h"
#include <fcntl.h>

#include <algorithm>
#include <sys/mman.h>

namespace Hagoromo {

    std::map<int, std::string> codecToStr = {
        {0, "mp3"}, // bad codec value
        {1, "pcm"},
        {2, "mp3"},
        {3, "aac"},
        {4, "wma"},
        {5, "atrac"},
        {6, "atrac"},
        {7, "atrac"}, // ATRAC Advanced Lossless
        {8, "flac"},
        {9, "alac"},
        {10, "dsf"},
        {12, "ape"},
        {14, "mqa"},
    };

    std::map<int, int> eq6PresetToHgrmIndex = {
        {0, 0},  // off
        {5, 1},  // bright
        {7, 2},  // excited
        {8, 3},  // mellow
        {6, 4},  // relaxed
        {2, 5},  // vocal
        {9, 6},  // custom 1
        {10, 7}, // custom 2
    };

    std::map<int, int> vptA50SmallToHgrmIndex = {
        {0, 0}, // bad index
        {1, 0}, // studio
        {2, 1}, // club
        {3, 2}, // concert hall
        {4, 3}, // matrix
    };

    std::map<int, int> dcFilterToHgrmIndex = {
        {0, 0}, // bad index
        {1, 0}, // a low
        {2, 1}, // a standard
        {3, 2}, // a high
        {4, 3}, // b low
        {5, 4}, // b standard
        {6, 5}, // b high
    };

    std::map<int, int> vinylTypeToHgrmIndex = {
        {0, 1}, // bad index
        {1, 1}, // standard
        {2, 2}, // arm resonance
        {3, 3}, // turntable resonance
        {4, 4}, // surface noise
        {5, 1}, // ?
        {6, 1}, // ?
        {7, 1}, // nw-a50 default? it's unclear how many user presets there are, from quick glance it's 4, but stock fw sets it to 7
    };

    std::map<int, int> dseeModeToHgrmIndex = {
        {0, 0}, // bad index, may come from another model
        {1, 0}, // ai
        {2, 1}, // female vocal
        {3, 2}, // male vocal
        {4, 3}, // percussion
        {5, 4}, // strings
    };

    void HagoromoConnector::sendData(char *data, size_t len, std::string *res) {
        int server_socket;
        struct sockaddr_un server_addr {};
        int connection_result;

        server_socket = socket(AF_UNIX, SOCK_STREAM, 0);

        server_addr.sun_family = AF_UNIX;
        strcpy(server_addr.sun_path, WAMPY_SOCKET);

        connection_result = connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

        if (connection_result == -1) {
            perror("failed to connect\n");
            return;
        }

        ssize_t sent = send(server_socket, data, len, MSG_NOSIGNAL);
        if (sent < 0) {
            perror("send failed\n");
            return;
        }

        //        DLOG("sent %d bytes\n", sent);

        while (true) {
            char buf[1024];
            auto count = read(server_socket, buf, sizeof buf);

            if (count < 0) {
                perror("read failed\n");
                return;
            }

            if (count == 0) {
                break;
            }
            //            DLOG("read %d bytes\n", count);
            for (int i = 0; i < count; i++) {
                *res += buf[i];
            }
        }

        //        for (auto c: *res) {
        //            printf("0x%02x ", c);
        //        }
        //        printf("\n");

        close(server_socket);
    }

    bool HagoromoConnector::sendCMD(Command::Command *c) {
        c->set_code(Command::FAIL);
        auto s = c->SerializeAsString();
        char data[s.length()];
        memcpy(data, s.data(), s.length());
        std::string res{};

        sendData(data, s.length(), &res);

        if (!c->ParseFromString(res)) {
            DLOG("parsing failed\n");
            return false;
        }

        switch (c->type()) {
        case Command::CMD_GET_STATUS:
            break;
        default:
            DLOG("received response %s\n", commandNames[c->type()].c_str());
            break;
        }

        return true;
    }

    void HagoromoConnector::enableTouchscreen() const {
#ifdef DESKTOP
        return;
#endif
        const char *tp = touchscreenPath;
        if (!exists(tp)) {
            tp = touchscreenPath2;
            if (!exists(tp)) {
                DLOG("we are out of touch\n");
                return;
            }
        }

        char data[3] = "0\n";
        int tsFd = open(tp, O_RDWR);
        if (write(tsFd, &data, sizeof(data)) < 0) {
            perror("touchscreen enable failed");
            close(tsFd);
            return;
        }
        close(tsFd);
    }

    void HagoromoConnector::disableTouchscreen() const {
        char data[3] = "1\n";

        const char *tp = touchscreenPath;
        if (!exists(tp)) {
            tp = touchscreenPath2;
            if (!exists(tp)) {
                DLOG("we are out of touch\n");
                return;
            }
        }

        int tsfd = open(tp, O_RDWR);
        if (write(tsfd, &data, sizeof(data)) < 0) {
            perror("touchscreen disable failed");
            close(tsfd);
            return;
        }
        close(tsfd);
    }

    bool HagoromoConnector::isOn() const {
        char data[5];
        int brfd = open(brightnessPath, O_RDONLY);
        if (brfd < 0) {
            perror("cannot open brightness");
            return false;
        }

        if (read(brfd, data, sizeof data) < 0) {
            printf("failed to read from brightness");
            return false;
        }

        if (data[0] == '0') {
            return false;
        }

        return true;
    }

    void HagoromoConnector::TestCommand() {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_TEST);

        sendCMD(&c);
    }

    void HagoromoConnector::FeatureBigCover(bool enable) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_FEATURE_BIG_COVER);
        c.mutable_featurebigcover()->set_enabled(enable);
        sendCMD(&c);
    }

    void HagoromoConnector::FeatureShowTime(bool enable) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_FEATURE_SHOW_CLOCK);
        c.mutable_featureshowclock()->set_enabled(enable);
        sendCMD(&c);
    }

    void HagoromoConnector::FeatureSetMaxVolume(bool enable) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_FEATURE_SET_MAX_VOLUME);
        c.mutable_featuresetmaxvolume()->set_enabled(enable);
        sendCMD(&c);
    }

    void HagoromoConnector::ToggleHgrm(HgrmToggleAction action, bool *render) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_GET_WINDOW_STATUS);
        if (!sendCMD(&c)) {
            DLOG("get window status fail\n");
            return;
        }

        if (c.code() != Command::OK) {
            DLOG("cannot get window status, not ok\n");
            return;
        }

        bool visible = false;
        if (c.windowstatus().visible() == Command::VISIBILITY_YES) {
            visible = true;
        }

        Command::WindowVisible expected = Command::VISIBILITY_UNKNOWN;
        DLOG("visible %d, action %s\n", visible, action == 1 ? "Hide" : "Show");
        if (!visible && (action == Show)) {
            DLOG("hgrm status: %d, showing hgrm\n", c.windowstatus().visible());
            c.set_type(Command::CMD_SHOW_WINDOW);
            expected = Command::VISIBILITY_YES;
        }

        if (visible && (action == Hide)) {
            DLOG("hgrm status: %d, hiding hgrm\n", c.windowstatus().visible());
            c.set_type(Command::CMD_HIDE_WINDOW);
            expected = Command::VISIBILITY_NO;
        }

        if (expected != Command::VISIBILITY_UNKNOWN) {
            if (!sendCMD(&c)) {
                DLOG("toggle command send failed\n");
                return;
            }

            if (c.code() != Command::OK) {
                DLOG("toggle command failed\n");
                *render = true;
                enableTouchscreen();
                // It would be nice to add restart button, but touchscreen device doesn't emit inotify events,
                // unless there is an application reading from it.
                // You can run `cat /dev/input/event0` in background, but it's not worth it.
                // Just prompt user to restart.
                stateString = "No connection, restart device";
                status.Duration = 0;
                return;
            }

            // hide/show command is async, wait for expected status
            bool ok = false;
            for (int i = 0; i < 10; i++) {
                c.set_type(Command::CMD_GET_WINDOW_STATUS);
                sendCMD(&c);
                if (c.code() == Command::UNKNOWN) {
                    DLOG("cannot get status\n");
                    *render = true;
                    enableTouchscreen();
                    stateString = "No connection, restart device";
                    status.Duration = 0;
                    return;
                }

                DLOG("hgrm is %d\n", c.windowstatus().visible());
                if (c.windowstatus().visible() == expected) {
                    ok = true;
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            if (!ok) {
                DLOG("waited for expected window status, but it took too long\n");
                return;
            }

            DLOG("hgrm window status: %d\n", c.windowstatus().visible());
        }

        visible = c.windowstatus().visible() == Command::VISIBILITY_YES;

        if (visible) {
            DLOG("visible\n");
        } else {
            DLOG("hidden\n");
        }

        stateString = "";
        if (visible) {
            *render = false;
        } else {
            *render = true;

            if (!*touchscreenStaysOFF || status.Volume == 100) {
                enableTouchscreen();
            }

            FeatureBigCover(*featureBigCover);
            FeatureShowTime(*featureShowTime);
            FeatureSetMaxVolume(*featureLimitVolume);
        }

        soundSettings.Update();
        soundSettingsFw.Update();
    }

    void HagoromoConnector::Connect() {}

    void HagoromoConnector::ReadLoop() {
        auto fd = shm_open(HAGOROMO_STATUS_SHM_PATH, O_RDWR, 0);
        if (fd == -1) {
            DLOG("shm_open: %s\n", strerror(errno));
            return;
        }

        hagoromoStatus = static_cast<HagoromoStatus *>(mmap(nullptr, sizeof(*hagoromoStatus), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        if (hagoromoStatus == MAP_FAILED) {
            DLOG("mmap: %s\n", strerror(errno));
            return;
        }

        PollStatus();

        while (true) {
            if (sem_wait(&hagoromoStatus->sem1) == -1) {
                DLOG("sem_wait: %s\n", strerror(errno));
                return;
            }

            PollStatus();
            if (status.Filename != prevFilename) {
                updateSoundSettings();
                sleep(1);
                soundSettingsFw.Update();
            }
        }
    }

    void HagoromoConnector::updateSoundSettings() {
        prevFilename = status.Filename;
        if (!*featureEqPerSong) {
            return;
        }

        soundSettings.Update();

        sound_settings ss{};
        if (SoundSettings::Exists(prevFilename)) {
            if (SoundSettings::Get(prevFilename, &ss) != 0) {
                DLOG("cannot get settings for %s\n", prevFilename.c_str());
                return;
            }
        } else if (SoundSettings::ExistsDir(prevFilename)) {
            if (SoundSettings::GetDir(prevFilename, &ss) != 0) {
                DLOG("cannot get settings for dir of %s\n", prevFilename.c_str());
                return;
            }
        } else {
            DLOG("getting default settings\n");
            if (SoundSettings::Get("default", &ss) != 0) {
                DLOG("cannot get default settings\n");
                return;
            }
        }

        DLOG("updating sound settings\n");

        if (soundSettings.s->status.clearAudioAvailable) {
            if (soundSettings.s->status.clearAudioOn != ss.status.clearAudioOn) {
                pauseIfNeeded();
                SetClearAudio(ss.status.clearAudioOn);
                DLOG("ca: %d\n", ss.status.clearAudioOn);
            }

            if (ss.status.clearAudioOn == 1) {
                // ignore everything else
                restorePlayState();
                return;
            }
        }

        if (soundSettings.s->status.directSourceAvailable) {
            if (soundSettings.s->status.directSourceOn != ss.status.directSourceOn) {
                pauseIfNeeded();
                SetDirectSource(ss.status.directSourceOn);
                DLOG("Direct Source: %d\n", ss.status.directSourceOn);
            }

            if (ss.status.directSourceOn == 1) {
                // ignore everything else
                restorePlayState();
                return;
            }
        }

        if (soundSettings.s->status.vptOn != ss.status.vptOn) {
            pauseIfNeeded();
            SetVPT(ss.status.vptOn);
        }

        if (soundSettings.s->status.vptMode != ss.status.vptMode) {
            DLOG(
                "vpt mode %d -> %d (index %d)\n",
                soundSettings.s->status.vptMode,
                ss.status.vptMode,
                vptA50SmallToHgrmIndex.at(ss.status.vptMode)
            );
            pauseIfNeeded();
            SetVPTPreset(vptA50SmallToHgrmIndex.at(ss.status.vptMode));
        }

        if (soundSettings.s->status.dseeHXOn != ss.status.dseeHXOn) {
            pauseIfNeeded();
            SetDsee(ss.status.dseeHXOn);
        }

        if (soundSettings.s->status.eq6Preset != ss.status.eq6Preset) {
            DLOG(
                "eq6preset %d -> %d (index %d)\n",
                soundSettings.s->status.eq6Preset,
                ss.status.eq6Preset,
                eq6PresetToHgrmIndex.at(ss.status.eq6Preset)
            );
            pauseIfNeeded();
            SetEqPreset(eq6PresetToHgrmIndex.at(ss.status.eq6Preset));
            soundSettings.Update();
        }

        if (!std::equal(
                std::begin(soundSettings.s->status.eq6Bands), std::end(soundSettings.s->status.eq6Bands), std::begin(ss.status.eq6Bands)
            )) {
            std::vector<double> bands;
            for (auto v : ss.status.eq6Bands) {
                bands.push_back(v);
            }
            pauseIfNeeded();
            SetEqBands(bands);
            soundSettings.Update();
        }

        if (soundSettings.s->status.eqUse != ss.status.eqUse) {
            DLOG("eq use: %d -> %d\n", soundSettings.s->status.eqUse, ss.status.eqUse);
            pauseIfNeeded();
            SetToneControlOrEQ(ss.status.eqUse);
        }

        if (ss.status.eqUse == 2) {
            if (!std::equal(
                    std::begin(soundSettings.s->status.eq10Bands),
                    std::end(soundSettings.s->status.eq10Bands),
                    std::begin(ss.status.eq10Bands)
                )) {
                std::vector<double> bands;
                for (auto v : ss.status.eq10Bands) {
                    bands.push_back(int(v / 2));
                }
                pauseIfNeeded();
                SetEqBands(bands);
            }
        }

        if (soundSettings.s->status.toneControlLow != ss.status.toneControlLow ||
            soundSettings.s->status.toneControlMid != ss.status.toneControlMid ||
            soundSettings.s->status.toneControlHigh != ss.status.toneControlHigh) {
            std::vector<int> values;
            values.push_back(ss.status.toneControlLow);
            values.push_back(ss.status.toneControlMid);
            values.push_back(ss.status.toneControlHigh);
            DLOG(
                "tone low: %d -> %d\ntone mid: %d -> %d\ntone high: %d -> %d\n",
                soundSettings.s->status.toneControlLow,
                ss.status.toneControlLow,
                soundSettings.s->status.toneControlMid,
                ss.status.toneControlMid,
                soundSettings.s->status.toneControlHigh,
                ss.status.toneControlHigh
            );
            pauseIfNeeded();
            SetToneControlValues(values);
        }

        if (ss.status.dseeCustOn == true) {
            if (soundSettings.s->status.dseeCustOn == true) {
                if (ss.status.dseeCustMode != soundSettings.s->status.dseeCustMode) {
                    // just change mode
                    DLOG("dsee custom mode: %d (index %d)\n", ss.status.dseeCustMode, dseeModeToHgrmIndex.at(ss.status.dseeCustMode));
                    pauseIfNeeded();
                    SetDseeCustMode(dseeModeToHgrmIndex.at(ss.status.dseeCustMode));
                }
            } else {
                // changing mode will turn dsee on
                DLOG("dsee custom mode: %d (index %d)\n", ss.status.dseeCustMode, dseeModeToHgrmIndex.at(ss.status.dseeCustMode));
                pauseIfNeeded();
                SetDseeCustMode(dseeModeToHgrmIndex.at(ss.status.dseeCustMode));
            }
        } else {
            if (soundSettings.s->status.dseeCustOn == false) {
                // do nothing
            } else {
                // turn off
                DLOG("dsee custom: %d\n", ss.status.dseeCustOn);
                pauseIfNeeded();
                SetDseeCust(ss.status.dseeCustOn);
            }
        }

        if (ss.status.dcLinearOn == true) {
            if (soundSettings.s->status.dcLinearOn == true) {
                if (soundSettings.s->status.dcLinearFilter != ss.status.dcLinearFilter) {
                    // just change mode
                    DLOG(
                        "dc phase preset: %d -> %d (index %d)\n",
                        soundSettings.s->status.dcLinearFilter,
                        ss.status.dcLinearFilter,
                        dcFilterToHgrmIndex.at(ss.status.dcLinearFilter)
                    );
                    pauseIfNeeded();
                    SetDCPhasePreset(dcFilterToHgrmIndex.at(ss.status.dcLinearFilter));
                }
            } else {
                // changing mode will turn dc on
                DLOG(
                    "dc phase preset: %d -> %d (index %d)\n",
                    soundSettings.s->status.dcLinearFilter,
                    ss.status.dcLinearFilter,
                    dcFilterToHgrmIndex.at(ss.status.dcLinearFilter)
                );
                pauseIfNeeded();
                SetDCPhasePreset(dcFilterToHgrmIndex.at(ss.status.dcLinearFilter));
            }
        } else {
            if (soundSettings.s->status.dcLinearOn == false) {
                // do nothing
            } else {
                // turn off
                DLOG("dc phase: %d\n", ss.status.dcLinearOn);
                pauseIfNeeded();
                SetDCPhase(ss.status.dcLinearOn);
            }
        }

        if (ss.status.vinylOn == true) {
            if (soundSettings.s->status.vinylOn == true) {
                if (soundSettings.s->status.vinylType != ss.status.vinylType) {
                    // just change mode
                    DLOG(
                        "vinyl mode: %d -> %d (index %d)\n",
                        soundSettings.s->status.vinylType,
                        ss.status.vinylType,
                        vinylTypeToHgrmIndex.at(ss.status.vinylType)
                    );
                    if (ss.status.vinylType == 7) {
                        DLOG("ignoring vinyl type 7\n");
                    } else {
                        pauseIfNeeded();
                        SetVinylMode(vinylTypeToHgrmIndex.at(ss.status.vinylType));
                    }
                }
            } else {
                // changing mode will turn vinyl on
                DLOG(
                    "vinyl mode: %d -> %d (index %d)\n",
                    soundSettings.s->status.vinylType,
                    ss.status.vinylType,
                    vinylTypeToHgrmIndex.at(ss.status.vinylType)
                );
                pauseIfNeeded();
                SetVinylMode(vinylTypeToHgrmIndex.at(ss.status.vinylType));
            }
        } else {
            if (soundSettings.s->status.vinylOn == false) {
                // do nothing
            } else {
                // turn off
                DLOG("vinyl: %d\n", ss.status.vinylOn);
                pauseIfNeeded();
                SetVinyl(ss.status.vinylOn);
            }
        }

        restorePlayState();

        soundSettingsFw.Update();
    }

    struct TrackComparer {
        //        bool operator()(const Command::Track &a, const Command::Track &b) const { return a.track() < b.track(); };
        bool operator()(const Track &a, const Track &b) const { return a.TrackNumber < b.TrackNumber; };
    };

    void HagoromoConnector::PollStatus() {
        auto localStatus = *hagoromoStatus;
        if (prevEntryID != localStatus.entryId) {
            auto si = SongInfo{};
            DLOG("entry id is %x\n", localStatus.entryId);
            getSongData(localStatus.entryId & 0xffffff, &si);
            prevEntryID = localStatus.entryId;

            status.Elapsed = 0;
            localStatus.elapsed = 0;
            status.Bits = si.BitDepth;
            status.Bitrate = si.Bitrate / 1000;
            if (status.Bitrate > 1000) {
                status.BitrateString = "1k+";
            } else {
                status.BitrateString = std::to_string(status.Bitrate);
            }

            status.SampleRate = si.SampleRate / 1000;
            if (status.SampleRate > 100) {
                status.SampleRateString = "HR";
            } else {
                status.SampleRateString = std::to_string(status.SampleRate);
            }

            if (codecToStr.find(si.Codec) == codecToStr.end()) {
                auto parts = split(si.Filename, ".");
                status.Codec = parts.at(parts.size() - 1);
            } else {
                status.Codec = codecToStr.at(si.Codec);
            }

            status.Filename = si.Filename;
            status.Album = si.Album;
            status.TrackNumber = std::to_string(si.TrackNumber);
            status.Date = std::to_string(si.Year);

            if (m == nullptr) {
                m = new std::mutex();
            }

            m->lock();

            std::sort(&localStatus.playlist[0], &localStatus.playlist[PLAYLIST_SIZE], TrackComparer());
            for (auto &i : playlist) {
                i.Reset();
            }

            int i = 0;
            bool activeFound = false;
            for (const auto &track : localStatus.playlist) {
                if (track.Active) {
                    activeFound = true;
                    //                DLOG("track ----->>>> %d %d %s %d, found %d\n", i, track.TrackNumber, track.Title, track.Active,
                    //                activeFound);
                }

                if (!activeFound) {
                    continue;
                }

                //            DLOG("track %d %d %s %d, found %d\n", i, track.TrackNumber, track.Title, track.Active, activeFound);
                playlist[i].Track = std::to_string(track.TrackNumber);
                playlist[i].Artist = track.Artist;
                playlist[i].Title = track.Title;
                playlist[i].Duration = track.Duration;
                i++;
                if (i == playlist.size()) {
                    break;
                }
            }

            if (!activeFound) {
                DLOG("active not found, inserting first track from database to playlist\n");
                auto si = SongInfo{};
                getSongData(localStatus.entryId & 0xffffff, &si);
                playlist[0].Track = std::to_string(si.TrackNumber);
                playlist[0].Artist = si.Artist;
                playlist[0].Title = si.Title;
                playlist[0].Duration = si.Duration / 1000;
            }

            m->unlock();

            status.Channels = 2;
        }

        status.State = localStatus.playState;
        status.Shuffle = localStatus.shuffleOn;
        status.Repeat = localStatus.repeatMode;

        if (status.State == PLAYING && soundSettings.s->fmStatus.state == 2) {
            RadioOff();
            soundSettings.SetFM(0);
        }

        status.Duration = playlist.at(0).Duration;
        if (updateElapsedCounter < 1) {
            status.Elapsed = localStatus.elapsed / 1000;
        } else {
            updateElapsedCounter--;
        }

        if (updateVolume) {
            status.Volume = localStatus.volume;
            status.VolumeRaw = localStatus.volumeRaw;
        }

        for (const auto &client : clients) {
            if (client->active) {
                client->Notify();
                //                DLOG("notify\n");
            }
        }
    }

    /* ALSA volume != hgrmvolume
     despite sharing same value there is a noticeable difference between alsa volume level and application level
        __attribute__((unused)) void HagoromoConnector::setVolumeALSA(int i) {
            const char *master = "master volume";
            static char card[64] = "default";
            int err;
            snd_mixer_t *handle;
            if ((err = snd_mixer_open(&handle, 0)) < 0) {
                printf("Mixer %s open error: %s", card, snd_strerror(err));
                return;
            }

            if ((err = snd_mixer_attach(handle, card)) < 0) {
                printf("Mixer attach %s error: %s", card, snd_strerror(err));
                snd_mixer_close(handle);
                return;
            }

            if ((err = snd_mixer_selem_register(handle, nullptr, nullptr)) < 0) {
                printf("Mixer register error: %s", snd_strerror(err));
                snd_mixer_close(handle);
                return;
            }

            err = snd_mixer_load(handle);
            if (err < 0) {
                printf("Mixer %s load error: %s", card, snd_strerror(err));
                snd_mixer_close(handle);
                return;
            }

            snd_mixer_selem_id_t *sid;
            snd_mixer_selem_id_alloca(&sid);
            snd_mixer_selem_id_set_index(sid, 0);
            snd_mixer_selem_id_set_name(sid, master);
            auto elem = snd_mixer_find_selem(handle, sid);
            if (elem == nullptr) {
                fprintf(stderr, "failed to find element\n");
                return;
            }

            snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_MONO, i);
            Volume = i;
            snd_mixer_close(handle);

        }
    */

    // volume must be set only on slider release to prevent gui hiccups
    void HagoromoConnector::SetVolume(int i, bool relative) {
        auto c = Command::Command();
        c.set_type(Command::CMD_SET_VOLUME);

        c.mutable_setvolume()->set_relative(relative);
        if (relative) {
            c.mutable_setvolume()->set_relativevalue(i);
        } else {
            c.mutable_setvolume()->set_relativevalue(0);
        }

        c.mutable_setvolume()->set_valuepercent(i);
        DLOG("%d from i=%d  -> %d\n", status.Volume, i, c.setvolume().valuepercent());
        sendCMD(&c);
    }

    void HagoromoConnector::SetPosition(int percent) {
        int newPos = playlist.at(0).Duration * percent / 100 * 1000;
        DLOG("%d (%d%%)\n", newPos, percent);
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SEEK);
        c.mutable_seek()->set_value(newPos);
        sendCMD(&c);
    }

    void HagoromoConnector::SetBalance(int i) {}

    void HagoromoConnector::SetShuffle(int i) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_TOGGLE_SHUFFLE);
        sendCMD(&c);
    }

    void HagoromoConnector::SetRepeat(int i) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_TOGGLE_REPEAT);
        sendCMD(&c);
    }

    void HagoromoConnector::PrevTrack() {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_PREV_TRACK);
        sendCMD(&c);
    }

    void HagoromoConnector::Play() {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_PLAY);
        sendCMD(&c);
    }

    void HagoromoConnector::Pause() {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_PAUSE);
        sendCMD(&c);
    }

    void HagoromoConnector::Stop() {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_STOP);
        sendCMD(&c);
    }

    void HagoromoConnector::NextTrack() {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_NEXT_TRACK);
        sendCMD(&c);
    }

    void HagoromoConnector::storageLoop() {
        storagePresent = exists("/contents/MUSIC/");

        int max_events = 1;
        struct epoll_event evProc, events[max_events];
        int procMountsFd, nfds, epollfd;

        if ((procMountsFd = open(procMounts, O_RDONLY)) < 0) {
            DLOG("cannot open %s\n", procMounts);
            exit(EXIT_FAILURE);
        }

        epollfd = epoll_create1(0);
        if (epollfd == -1) {
            perror("epoll_create1");
            exit(EXIT_FAILURE);
        }

        evProc.events = POLLERR | POLLPRI;
        evProc.data.fd = procMountsFd;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, procMountsFd, &evProc) == -1) {
            perror("epoll_ctl: procMounts");
            exit(EXIT_FAILURE);
        }

        for (;;) {
            nfds = epoll_wait(epollfd, events, max_events, -1);
            if (nfds == -1) {
                perror("epoll_wait");
                exit(EXIT_FAILURE);
            }

            for (int n = 0; n < nfds; ++n) {
                auto event = events[n];
                if (event.data.fd == procMountsFd) {
                    storagePresent = exists("/contents/MUSIC/");
                    DLOG("storagePresent: %d\n", storagePresent);
                }
            }
        }
    }

    void HagoromoConnector::powerLoop(bool *render, bool *power) {
#ifdef DESKTOP
        return;
#endif
        int fd, poll_num;
        ssize_t len;
        const struct inotify_event *event;
        char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
        struct pollfd fds[1];

        fd = inotify_init1(IN_NONBLOCK);
        if (fd == -1) {
            perror("inotify_init1");
            exit(EXIT_FAILURE);
        }

        auto brfd = inotify_add_watch(fd, brightnessPath, IN_CLOSE_WRITE);
        if (brfd < 0) {
            perror("inotify_add_watch");
            exit(EXIT_FAILURE);
        }

        fds[0].fd = fd; /* Inotify input */
        fds[0].events = POLLIN;

        while (true) {
            poll_num = poll(fds, 1, -1);
            if (poll_num == -1) {
                if (errno == EINTR)
                    continue;
                perror("failed to poll brightness");
                exit(EXIT_FAILURE);
            }

            if (poll_num > 0) {
                if (fds[0].revents & POLLIN) {
                    /* Inotify events are available. */
                    memset(buf, 0, sizeof buf);

                    for (;;) {
                        /* Read some events. */
                        len = read(fd, buf, sizeof(buf));
                        if (len == -1 && errno != EAGAIN) {
                            perror("read");
                            exit(EXIT_FAILURE);
                        }

                        /* If the nonblocking read() found no events to read, then
                           it returns -1 with errno set to EAGAIN. In that case,
                           we exit the loop. */

                        if (len <= 0)
                            break;

                        /* Loop over all events in the buffer. */

                        for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
                            event = (const struct inotify_event *)ptr;
                            if (event->mask & IN_CLOSE_WRITE) {
                                if (event->wd == brfd) {
                                    int ff = open(brightnessPath, O_RDONLY);
                                    if (ff < 0) {
                                        perror("brightness file");
                                        exit(EXIT_FAILURE);
                                    }

                                    char b[1];
                                    if (read(ff, b, sizeof b) < 0) {
                                        perror("failed to read inotified changes\n");
                                        exit(EXIT_FAILURE);
                                    }

                                    if (b[0] == '0') {
                                        DLOG("power off\n");
                                        *power = false;
                                        if (soundSettings.s->fmStatus.state == 2) {
                                        }
                                    } else {
                                        DLOG("power on\n");
                                        *power = true;
                                    }

                                    close(ff);
                                }
                            }

                            if (event->len)
                                DLOG("%s", event->name);
                        }
                    }
                }
            }
        }
    }

    void HagoromoConnector::Start() {
        Connector::Start();

        auto stor = [this]() { storageLoop(); };
        std::thread storT(stor);
        storT.detach();

        auto pwr = [this]() { powerLoop(render, &power); };
        std::thread powert(pwr);
        powert.detach();

        auto snd = [this]() { volumeLoop(); };
        std::thread voll(snd);
        voll.detach();
    }

    void HagoromoConnector::SetClearAudio(bool enable) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SET_CLEAR_AUDIO);
        c.mutable_clearaudio()->set_enabled(enable);
        sendCMD(&c);
    }

    void HagoromoConnector::SetEqBands(std::vector<double> bandValueList) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SET_EQ_BANDS);
        for (auto v : bandValueList) {
            c.mutable_eqbands()->add_bandvalue(v);
        }
        sendCMD(&c);
    }

    void HagoromoConnector::SetEqPreset(int index) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SET_EQ_PRESET);
        c.mutable_eqpreset()->set_preset(index);
        sendCMD(&c);
    }

    void HagoromoConnector::SetVPT(bool enable) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SET_VPT);
        c.mutable_vpt()->set_enabled(enable);
        sendCMD(&c);
    }

    void HagoromoConnector::SetVPTPreset(int preset) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SET_VPT_PRESET);
        c.mutable_vptpreset()->set_preset(preset);
        sendCMD(&c);
    }

    void HagoromoConnector::SetDsee(bool enable) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SET_DSEE);
        c.mutable_dsee()->set_enabled(enable);
        sendCMD(&c);
    }

    void HagoromoConnector::SetDCPhase(bool enable) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SET_DCPHASE);
        c.mutable_dcphase()->set_enabled(enable);
        sendCMD(&c);
    }

    void HagoromoConnector::SetDCPhasePreset(int preset) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SET_DCPHASE_PRESET);
        c.mutable_dcphasepreset()->set_preset(preset);
        sendCMD(&c);
    }

    void HagoromoConnector::SetVinyl(bool enable) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SET_VINYL);
        c.mutable_vinyl()->set_enabled(enable);
        sendCMD(&c);
    }

    void HagoromoConnector::SetDirectSource(bool enable) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SET_DIRECT_SOURCE);
        c.mutable_directsource()->set_enabled(enable);
        sendCMD(&c);
    }

    void HagoromoConnector::SetToneControlValues(const std::vector<int> &v) {
        auto c = Command::Command();
        if (v.size() < 3) {
            DLOG("not enough values\n");
            return;
        }

        c.set_type(Command::Type::CMD_SET_TONE_CONTROL_VALUES);
        c.mutable_tonecontrolvalues()->set_low(v.at(0) / 2);
        c.mutable_tonecontrolvalues()->set_middle(v.at(1) / 2);
        c.mutable_tonecontrolvalues()->set_high(v.at(2) / 2);
        sendCMD(&c);
    }

    void HagoromoConnector::SetToneControlOrEQ(int eqType) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SET_TONE_CONTROL_OR_EQ);
        c.mutable_tonecontroloreq()->set_eqid(eqType);
        sendCMD(&c);
    }

    void HagoromoConnector::SetDseeCust(bool enable) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SET_DSEE_CUST);
        c.mutable_dseecust()->set_enabled(enable);
        sendCMD(&c);
    }

    void HagoromoConnector::SetDseeCustMode(int index) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SET_DSEE_CUST_MODE);
        c.mutable_dseecustmode()->set_mode(index);
        sendCMD(&c);
    }

    void HagoromoConnector::SetVinylMode(int mode) {
        auto c = Command::Command();
        c.set_type(Command::Type::CMD_SET_VINYL_MODE);
        c.mutable_vinylmode()->set_mode(mode);
        sendCMD(&c);
    }

    void HagoromoConnector::getSongData(int entryId, SongInfo *s) {
        sqlite3 *db;
        int rc;
#ifdef DESKTOP
        auto path = "../MTPDB.dat";
#else
        auto path = "/db/MTPDB.dat";
#endif
        const char *query =
            "WITH RECURSIVE path_cte AS (\n"
            "    SELECT\n"
            "        ob.object_id,\n"
            "        parent_id,\n"
            "        filename,\n"
            "\t\ttitle,\n"
            "\t\tCOALESCE(a.value, \"\") as artist,\n"
            "\t\tCOALESCE(alb.value, \"\") as album,\n"
            "\t\tCOALESCE(ob.series_no, -1) as track,\n"
            "\t\tCOALESCE(duration.value, -1) as duration,\n"
            "\t\tCOALESCE(sample_rate.value, -1) as sample_rate,\n"
            "\t\tCOALESCE(bit_depth.value, -1) as bit_depth,\n"
            "\t\tCOALESCE(bitrate.value, -1) as bitrate,\n"
            "\t\tCOALESCE(total_tracks.value, -1) as total_tracks,\n"
            "\t\tCOALESCE(codec.value, -1) as codec,\t\t\n"
            "\t\tCOALESCE(releaseyear.value, -1) as release_year\t"
            "    FROM\n"
            "        object_body ob\n"
            "\tjoin artists a on a.id = ob.artist_id \n"
            "\tjoin albums alb on alb.id = ob.album_id \n"
            "\tleft join object_ext_int duration on duration.object_id = ob.object_id and duration.akey = 12\n"
            "\tleft join object_ext_int sample_rate on sample_rate.object_id = ob.object_id and sample_rate.akey = 16\n"
            "\tleft join object_ext_int bitrate on bitrate.object_id = ob.object_id and bitrate.akey = 19\n"
            "\tleft join object_ext_int bit_depth on bit_depth.object_id = ob.object_id and bit_depth.akey = 78\n"
            "\tleft join object_ext_int total_tracks on total_tracks.object_id = ob.object_id and total_tracks.akey = 81\n"
            "\tleft join object_ext_int codec on codec.object_id = ob.object_id and codec.akey = 41\n"
            "\tleft join releaseyears releaseyear on releaseyear.id = ob.releaseyear_id\n"
            "    WHERE\n"
            "        ob.object_id = ?\n"
            "\tAND\n"
            "\t\tob.object_type = 2\n"
            "    \n"
            "    UNION ALL\n"
            "    \n"
            "    SELECT\n"
            "        t.object_id,\n"
            "        t.parent_id,\n"
            "        t.filename || '/' || p.filename AS filename,\n"
            "\t\tp.title,\n"
            "\t\tp.artist,\n"
            "\t\tp.album,\n"
            "\t\tp.track,\n"
            "\t\tp.duration,\n"
            "\t\tp.sample_rate,\n"
            "\t\tp.bit_depth,\n"
            "\t\tp.bitrate,\n"
            "\t\tp.total_tracks,\n"
            "\t\tp.codec,\n"
            "\t\tp.release_year\n"
            "    FROM\n"
            "        object_body t\n"
            "    INNER JOIN\n"
            "        path_cte p ON t.object_id = p.parent_id\n"
            ")\n"
            "\n"
            "SELECT\n"
            "    filename, title, artist, album, track, duration, sample_rate, bit_depth, bitrate, total_tracks, codec, release_year\n"
            "FROM\n"
            "    path_cte\n"
            "WHERE\n"
            "    parent_id == 0;";

        rc = sqlite3_open_v2(path, &db, SQLITE_OPEN_READONLY, nullptr);
        if (rc) {
            DLOG("Can't open database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        sqlite3_stmt *select_stmt = nullptr;
        rc = sqlite3_prepare_v2(db, query, -1, &select_stmt, nullptr);
        if (rc) {
            DLOG("Can't prepare select statement %s (%i): %s\n", query, rc, sqlite3_errmsg(db));
            sqlite3_finalize(select_stmt);
            sqlite3_close(db);
            return;
        }

        rc = sqlite3_bind_int(select_stmt, 1, entryId);
        if (SQLITE_OK != rc) {
            fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
            sqlite3_finalize(select_stmt);
            sqlite3_close(db);
            return;
        }

        int rows = 0;
        while (SQLITE_ROW == (rc = sqlite3_step(select_stmt))) {
            rows++;
            s->Filename = (char *)sqlite3_column_text(select_stmt, 0);
            s->Title = (char *)sqlite3_column_text(select_stmt, 1);
            s->Artist = (char *)sqlite3_column_text(select_stmt, 2);
            s->Album = (char *)sqlite3_column_text(select_stmt, 3);
            s->TrackNumber = sqlite3_column_int(select_stmt, 4);
            s->Duration = sqlite3_column_int(select_stmt, 5);
            s->SampleRate = sqlite3_column_int(select_stmt, 6);
            s->BitDepth = sqlite3_column_int(select_stmt, 7);
            s->Bitrate = sqlite3_column_int(select_stmt, 8);
            s->TotalTracks = sqlite3_column_int(select_stmt, 9);
            s->Codec = sqlite3_column_int(select_stmt, 10);
            s->Year = sqlite3_column_int(select_stmt, 11);
        }

        sqlite3_finalize(select_stmt);
        sqlite3_close(db);

        DLOG("found %d rows for id %x\n", rows, entryId);
    }

    void HagoromoConnector::pauseIfNeeded() {
        return;
        playStateBeforeSoundSettingsUpdate = hagoromoStatus->playState;
        if (hagoromoStatus->playState != PAUSED) {
            Pause();
        }
    }

    void HagoromoConnector::restorePlayState() {
        return;
        DLOG("restore to %d\n", playStateBeforeSoundSettingsUpdate);
        switch (playStateBeforeSoundSettingsUpdate) {
        case UNKNOWN:
            break;
        case PLAYING:
            Play();
            break;
        case PAUSED:
            break;
        case STOPPED:
            Stop();
            break;
        }

        playStateBeforeSoundSettingsUpdate = UNKNOWN;
    }

    /* alsa uses ioctl to update events, so polling with `poll` doesn't work
     volume is provided by hagoromo
      */
    void HagoromoConnector::volumeLoop() {
        snd_mixer_t *mhandle;
        static struct snd_mixer_selem_regopt smixer_options;
        static int smixer_level = 0;
        int err;

        if ((err = snd_mixer_open(&mhandle, 0)) < 0) {
            DLOG("Mixer open error: %s\n", snd_strerror(err));
            return;
        }

        char card[] = "hw:0";
        if (smixer_level == 0 && (err = snd_mixer_attach(mhandle, card)) < 0) {
            DLOG("Mixer attach %s error: %s\n", card, snd_strerror(err));
            snd_mixer_close(mhandle);
            return;
        }

        if ((err = snd_mixer_selem_register(mhandle, smixer_level > 0 ? &smixer_options : nullptr, nullptr)) < 0) {
            DLOG("Mixer register error: %s\n", snd_strerror(err));
            snd_mixer_close(mhandle);
            return;
        }

        err = snd_mixer_load(mhandle);
        if (err < 0) {
            DLOG("Mixer load %s error: %s\n", card, snd_strerror(err));
            snd_mixer_close(mhandle);
            return;
        }

        DLOG("Simple ctrls: %i\n", snd_mixer_get_count(mhandle));

        snd_mixer_elem_t *elem = snd_mixer_first_elem(mhandle);
        while (true) {
            const char *name = snd_mixer_selem_get_name(elem);
            //            DLOG("%s\n", snd_mixer_selem_get_name(elem));
            if (strcmp(name, "analog input device") == 0) {
                break;
            }
            elem = snd_mixer_elem_next(elem);
            if (!elem) {
                break;
            }
        }

        if (!elem) {
            DLOG("element not found\n");
            return;
        }

        DLOG("volume loop started\n");

        uint fmOutputState;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (power) {
                continue;
            }

            if (soundSettings.s->fmStatus.state != 2) {
                continue;
            }

            snd_mixer_selem_get_enum_item(elem, static_cast<snd_mixer_selem_channel_id_t>(0), &fmOutputState);
            if (fmOutputState == ALSA_ANALOG_INPUT_OFF) {
                DLOG("set analog input to tuner\n");
                snd_mixer_selem_set_enum_item(elem, static_cast<snd_mixer_selem_channel_id_t>(0), ALSA_ANALOG_INPUT_TUNER);
            }
        }
    }
} // namespace Hagoromo