#include "command.pb.h"
#include "command_names.h"
#include <cstdio>
#include <cstdlib>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

// #include "alsa/asoundlib.h"
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

        char data[3] = "0\n";
        int tsFd = open(touchscreenPath, O_RDWR);
        if (write(tsFd, &data, sizeof(data)) < 0) {
            perror("touchscreen enable failed");
            close(tsFd);
            return;
        }
        close(tsFd);
    }

    void HagoromoConnector::disableTouchscreen() const {
        char data[3] = "1\n";
        int tsfd = open(touchscreenPath, O_RDWR);
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
                serverReady = false;
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
                    serverReady = false;
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
            serverReady = false;
        } else {
            *render = true;
            serverReady = true;

            if (!*touchscreenStaysOFF || status.Volume == 100) {
                enableTouchscreen();
            }

            FeatureBigCover(*featureBigCover);
            FeatureShowTime(*featureShowTime);
            FeatureSetMaxVolume(*featureLimitVolume);
        }

        soundSettings.Update();
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

        if (soundSettings.s->clearAudioAvailable) {
            if (soundSettings.s->clearAudioOn != ss.clearAudioOn) {
                pauseIfNeeded();
                SetClearAudio(ss.clearAudioOn);
                DLOG("ca: %d\n", ss.clearAudioOn);
            }

            if (ss.clearAudioOn == 1) {
                // ignore everything else
                restorePlayState();
                return;
            }
        }

        if (soundSettings.s->directSourceAvailable) {
            if (soundSettings.s->directSourceOn != ss.directSourceOn) {
                pauseIfNeeded();
                SetDirectSource(ss.directSourceOn);
                DLOG("Direct Source: %d\n", ss.directSourceOn);
            }

            if (ss.directSourceOn == 1) {
                // ignore everything else
                restorePlayState();
                return;
            }
        }

        if (soundSettings.s->vptOn != ss.vptOn) {
            pauseIfNeeded();
            SetVPT(ss.vptOn);
        }

        if (soundSettings.s->vptMode != ss.vptMode) {
            DLOG("vpt mode %d -> %d (index %d)\n", soundSettings.s->vptMode, ss.vptMode, vptA50SmallToHgrmIndex.at(ss.vptMode));
            pauseIfNeeded();
            SetVPTPreset(vptA50SmallToHgrmIndex.at(ss.vptMode));
        }

        if (soundSettings.s->dseeHXOn != ss.dseeHXOn) {
            pauseIfNeeded();
            SetDsee(ss.dseeHXOn);
        }

        if (soundSettings.s->eq6Preset != ss.eq6Preset) {
            DLOG("eq6preset %d -> %d (index %d)\n", soundSettings.s->eq6Preset, ss.eq6Preset, eq6PresetToHgrmIndex.at(ss.eq6Preset));
            pauseIfNeeded();
            SetEqPreset(eq6PresetToHgrmIndex.at(ss.eq6Preset));
            soundSettings.Update();
        }

        if (!std::equal(std::begin(soundSettings.s->eq6Bands), std::end(soundSettings.s->eq6Bands), std::begin(ss.eq6Bands))) {
            std::vector<double> bands;
            for (auto v : ss.eq6Bands) {
                bands.push_back(v);
            }
            pauseIfNeeded();
            SetEqBands(bands);
            soundSettings.Update();
        }

        if (soundSettings.s->eqUse != ss.eqUse) {
            DLOG("eq use: %d -> %d\n", soundSettings.s->eqUse, ss.eqUse);
            pauseIfNeeded();
            SetToneControlOrEQ(ss.eqUse);
        }

        if (ss.eqUse == 2) {
            if (!std::equal(std::begin(soundSettings.s->eq10Bands), std::end(soundSettings.s->eq10Bands), std::begin(ss.eq10Bands))) {
                std::vector<double> bands;
                for (auto v : ss.eq10Bands) {
                    bands.push_back(int(v / 2));
                }
                pauseIfNeeded();
                SetEqBands(bands);
            }
        }

        if (soundSettings.s->toneControlLow != ss.toneControlLow || soundSettings.s->toneControlMid != ss.toneControlMid ||
            soundSettings.s->toneControlHigh != ss.toneControlHigh) {
            std::vector<int> values;
            values.push_back(ss.toneControlLow);
            values.push_back(ss.toneControlMid);
            values.push_back(ss.toneControlHigh);
            DLOG(
                "tone low: %d -> %d\ntone mid: %d -> %d\ntone high: %d -> %d\n",
                soundSettings.s->toneControlLow,
                ss.toneControlLow,
                soundSettings.s->toneControlMid,
                ss.toneControlMid,
                soundSettings.s->toneControlHigh,
                ss.toneControlHigh
            );
            pauseIfNeeded();
            SetToneControlValues(values);
        }

        if (ss.dseeCustOn == true) {
            if (soundSettings.s->dseeCustOn == true) {
                if (ss.dseeCustMode != soundSettings.s->dseeCustMode) {
                    // just change mode
                    DLOG("dsee custom mode: %d (index %d)\n", ss.dseeCustMode, dseeModeToHgrmIndex.at(ss.dseeCustMode));
                    pauseIfNeeded();
                    SetDseeCustMode(dseeModeToHgrmIndex.at(ss.dseeCustMode));
                }
            } else {
                // changing mode will turn dsee on
                DLOG("dsee custom mode: %d (index %d)\n", ss.dseeCustMode, dseeModeToHgrmIndex.at(ss.dseeCustMode));
                pauseIfNeeded();
                SetDseeCustMode(dseeModeToHgrmIndex.at(ss.dseeCustMode));
            }
        } else {
            if (soundSettings.s->dseeCustOn == false) {
                // do nothing
            } else {
                // turn off
                DLOG("dsee custom: %d\n", ss.dseeCustOn);
                pauseIfNeeded();
                SetDseeCust(ss.dseeCustOn);
            }
        }

        if (ss.dcLinearOn == true) {
            if (soundSettings.s->dcLinearOn == true) {
                if (soundSettings.s->dcLinearFilter != ss.dcLinearFilter) {
                    // just change mode
                    DLOG(
                        "dc phase preset: %d -> %d (index %d)\n",
                        soundSettings.s->dcLinearFilter,
                        ss.dcLinearFilter,
                        dcFilterToHgrmIndex.at(ss.dcLinearFilter)
                    );
                    pauseIfNeeded();
                    SetDCPhasePreset(dcFilterToHgrmIndex.at(ss.dcLinearFilter));
                }
            } else {
                // changing mode will turn dc on
                DLOG(
                    "dc phase preset: %d -> %d (index %d)\n",
                    soundSettings.s->dcLinearFilter,
                    ss.dcLinearFilter,
                    dcFilterToHgrmIndex.at(ss.dcLinearFilter)
                );
                pauseIfNeeded();
                SetDCPhasePreset(dcFilterToHgrmIndex.at(ss.dcLinearFilter));
            }
        } else {
            if (soundSettings.s->dcLinearOn == false) {
                // do nothing
            } else {
                // turn off
                DLOG("dc phase: %d\n", ss.dcLinearOn);
                pauseIfNeeded();
                SetDCPhase(ss.dcLinearOn);
            }
        }

        if (ss.vinylOn == true) {
            if (soundSettings.s->vinylOn == true) {
                if (soundSettings.s->vinylType != ss.vinylType) {
                    // just change mode
                    DLOG(
                        "vinyl mode: %d -> %d (index %d)\n", soundSettings.s->vinylType, ss.vinylType, vinylTypeToHgrmIndex.at(ss.vinylType)
                    );
                    if (ss.vinylType == 7) {
                        DLOG("ignoring vinyl type 7\n");
                    } else {
                        pauseIfNeeded();
                        SetVinylMode(vinylTypeToHgrmIndex.at(ss.vinylType));
                    }
                }
            } else {
                // changing mode will turn vinyl on
                DLOG("vinyl mode: %d -> %d (index %d)\n", soundSettings.s->vinylType, ss.vinylType, vinylTypeToHgrmIndex.at(ss.vinylType));
                pauseIfNeeded();
                SetVinylMode(vinylTypeToHgrmIndex.at(ss.vinylType));
            }
        } else {
            if (soundSettings.s->vinylOn == false) {
                // do nothing
            } else {
                // turn off
                DLOG("vinyl: %d\n", ss.vinylOn);
                pauseIfNeeded();
                SetVinyl(ss.vinylOn);
            }
        }

        restorePlayState();
    }

    struct TrackComparer {
        //        bool operator()(const Command::Track &a, const Command::Track &b) const { return a.track() < b.track(); };
        bool operator()(const Track &a, const Track &b) const { return a.TrackNumber < b.TrackNumber; };
    };

    void HagoromoConnector::PollStatus() {
        auto localStatus = *hagoromoStatus;
        if (prevEntryID != localStatus.entryId) {
            auto si = SongInfo{};
            getSongData(localStatus.entryId - 0x10000000, &si);
            prevEntryID = localStatus.entryId;

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
        }

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
        m->unlock();
        //        for (; i < playlist.size(); i++) {
        //            playlist[i].Reset();
        //        }

        status.Channels = 2;
        status.State = localStatus.playState;
        status.Shuffle = localStatus.shuffleOn;
        status.Repeat = localStatus.repeatMode;

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

        if (inotify_add_watch(fd, brightnessPath, IN_CLOSE_WRITE) < 0) {
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
                                    printf("power off\n");
                                    *power = false;
                                } else {
                                    printf("power on\n");
                                    *power = true;
                                }

                                close(ff);
                            }

                            if (event->len)
                                printf("%s", event->name);
                        }
                    }
                }
            }
        }
    }

    void HagoromoConnector::Start() {
        Connector::Start();
        auto pwr = [this]() { powerLoop(render, &power); };
        std::thread powert(pwr);
        powert.detach();
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
        const char *query = "WITH RECURSIVE path_cte AS (\n"
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
                            "\t\tCOALESCE(codec.value, -1) as codec\t\t\n"
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
                            "\t\tp.codec\n"
                            "    FROM\n"
                            "        object_body t\n"
                            "    INNER JOIN\n"
                            "        path_cte p ON t.object_id = p.parent_id\n"
                            ")\n"
                            "\n"
                            "SELECT\n"
                            "    filename, title, artist, album, track, duration, sample_rate, bit_depth, bitrate, total_tracks, codec\n"
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

        while (SQLITE_ROW == (rc = sqlite3_step(select_stmt))) {
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
        }

        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
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

    /* alsa doesn't receive events (see `amixer sevents`)
     alsa handle doesn't update either
     volume is provided by hagoromo!
        __attribute__((unused)) void HagoromoConnector::volumeLoop() {
            long pvol = 0;
            const char *master = "master volume";
            static char card[64] = "default";
            int err;
            snd_mixer_t *handle;

            for (;;) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                if ((err = snd_mixer_open(&handle, 0)) < 0) {
                    printf("Mixer %s open error: %s", card, snd_strerror(err));
                    break;
                }

                if ((err = snd_mixer_attach(handle, card)) < 0) {
                    printf("Mixer attach %s error: %s", card, snd_strerror(err));
                    snd_mixer_close(handle);
                    break;
                }

                if ((err = snd_mixer_selem_register(handle, nullptr, nullptr)) < 0) {
                    printf("Mixer register error: %s", snd_strerror(err));
                    snd_mixer_close(handle);
                    break;
                }

                err = snd_mixer_load(handle);
                if (err < 0) {
                    printf("Mixer %s load error: %s", card, snd_strerror(err));
                    snd_mixer_close(handle);
                    continue;
                }

                snd_mixer_selem_id_t *sid;
                snd_mixer_selem_id_alloca(&sid);
                snd_mixer_selem_id_set_index(sid, 0);
                snd_mixer_selem_id_set_name(sid, master);
                auto elem = snd_mixer_find_selem(handle, sid);
                if (elem == nullptr) {
                    fprintf(stderr, "failed to find element\n");
                    continue;
                }

                snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_MONO, &pvol);
                Volume = (int) pvol;
                snd_mixer_close(handle);
            }
            fprintf(stderr, "volume loop failed\n");
        }
    */

} // namespace Hagoromo