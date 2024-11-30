#ifndef HAGOROMO_CPP
#define HAGOROMO_CPP

#include "command.pb.h"
#include "command_names.h"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

// #include "alsa/asoundlib.h"
#include "hagoromo.h"
#include "wampy.h"
#include <fcntl.h>

#include <algorithm>

namespace Player {

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

        int sent = send(server_socket, data, len, 0);
        if (sent < 0) {
            perror("send failed\n");
            return;
        }

        //        DLOG("sent %d bytes\n", sent);

        while (true) {
            char buf[1024];
            int count = read(server_socket, buf, sizeof buf);

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
        char data[3];
        strcpy(data, "0\n");
        int tsFd = open(touchscreenPath, O_RDWR);
        if (write(tsFd, &data, sizeof(data)) < 0) {
            perror("touchscreen enable failed");
            close(tsFd);
            return;
        }
        close(tsFd);
    }

    void HagoromoConnector::disableTouchscreen() const {
        char data[3];
        strcpy(data, "1\n");
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

        Command::WindowVisible expected;
        DLOG("visible %d,  action %s\n", visible, action == 1 ? "Hide" : "Show");
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

        visible = false;
        if (c.windowstatus().visible() == Command::VISIBILITY_YES) {
            visible = true;
        }

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
            enableTouchscreen();
        }
    }

    void HagoromoConnector::Connect() {}

    [[noreturn]] void HagoromoConnector::ReadLoop() {
        for (;;) {
            if (*render && serverReady) {
                PollStatus();
                status.formatted = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    struct TrackComparer {
        bool operator()(Command::Track a, Command::Track b) const { return a.track() < b.track(); };
    };

    void HagoromoConnector::PollStatus() {
        auto c = Command::Command();
        c.set_type(Command::CMD_GET_STATUS);
        if (!sendCMD(&c)) {
            DLOG("cannot send command\n");
            return;
        }

        if (c.code() != Command::OK) {
            DLOG("failed getting status\n");
            return;
        }

        auto tracks = c.mutable_status()->mutable_playlist()->mutable_track();
        std::sort(tracks->begin(), tracks->end(), TrackComparer());

        for (auto &i : playlist) {
            i.Reset();
        }

        int i = 0;
        bool activeFound;
        for (const auto &track : *tracks) {
            //            DLOG("track %d %s %d\n", track.track(), track.title().c_str(), track.active());

            if (track.active()) {
                activeFound = true;
            }

            if (!activeFound) {
                continue;
            }

            auto song = &playlist.at(i);
            song->Track = std::to_string(track.track());
            song->Artist = track.artist();
            song->Title = track.title();
            song->Duration = track.duration();
            i++;
            if (i == playlist.size()) {
                break;
            }
        }

        status.Channels = 2;

        auto activeSong = &playlist.at(0);
        activeSong->File = activeSong->Title + activeSong->Artist + activeSong->Track + c.status().codec();

        if (c.status().playstate() == 1) {
            status.State = "play";
        } else {
            status.State = "pause";
        }

        status.Repeat = c.status().repeat();
        status.Shuffle = c.status().shuffle();

        if (updateVolume) {
            status.Volume = c.status().volume();
        }

        status.Duration = playlist.at(0).Duration;
        //        status.Elapsed = c.status().elapsed() / 1000;
        //        DLOG("elapsed %d %d\n", status.Elapsed, c.status().elapsed());
        if (updateElapsedCounter < 1) {
            status.Elapsed = c.status().elapsed() / 1000;
        } else {
            updateElapsedCounter--;
        }
        //
        //
        //        parseCodecString(c.status().codec());
        status.Codec = c.status().codec();
        status.Bitrate = c.status().bitrate();

        if (status.Bitrate > 1000) {
            status.BitrateString = "1k+";
        } else {
            status.BitrateString = std::to_string(status.Bitrate);
        }

        status.SampleRate = (int)c.status().samplerate();
        if (status.SampleRate > 100) {
            status.SampleRateString = "HR";
        } else {
            status.SampleRateString = std::to_string(status.SampleRate);
        }

        status.Bits = c.status().bitdepth();

        status.formatted = false;
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
        int vol;
        if (relative) {
            vol = status.Volume + i;
        } else {
            vol = i;
        }

        auto c = Command::Command();
        c.set_type(Command::CMD_SET_VOLUME);
        c.mutable_setvolume()->set_valuepercent(vol);
        DLOG("%d from i=%d  -> %d\n", vol, i, c.setvolume().valuepercent());
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

} // namespace Player
#endif // HAGOROMO_CPP
