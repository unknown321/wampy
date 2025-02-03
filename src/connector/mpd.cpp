#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

#include "../util/util.h"

#include "mpd.h"

const char *commandStatus = "command_list_begin\n"
                            "status\n"
                            "currentsong\n"
                            "command_list_end\n";

const char *changed = "changed";

const char *commandIdle = "idle\n";
const char *commandNoIdle = "noidle\n";

std::string lineDelimeter = "\n";
std::string fieldDelimeter = ": ";

int RESP_BUF_SIZE = 8192;

namespace MPD {
    void MPDConnector::Connect() {
        int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un server_addr {};

        if (strcmp(address, "") == 0) {
            address = MPDDefaultAddress;
        }

        server_addr.sun_family = AF_UNIX;
        strcpy(server_addr.sun_path, address);
        int connection_result;

        for (;;) {
            DLOG("connecting to mpd using %s\n", server_addr.sun_path);
            connection_result = connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
            if (connection_result == 0) {
                fd = server_socket;
                break;
            }

            sleep(1);
        }

        // read and discard hello string
        char buf[RESP_BUF_SIZE];
        recv(fd, buf, RESP_BUF_SIZE, 0);
    }

    void MPDConnector::Send(const char *cmd) const {
        if (send(fd, cmd, strlen(cmd), 0) == -1) {
            perror("send\n");
            exit(1);
        }
    }

    void MPDConnector::resetEmptyFields(Song *song, statusFields *sf) {
        if (!sf->Artist)
            song->Artist = "";
        if (!sf->Track)
            song->Track = "";
        if (!sf->Title)
            song->Title = "";
        if (!sf->Album)
            song->Album = "";
        //            if (!sf->Elapsed)
        //                song->Elapsed = 0;
        //            if (!sf->Duration)
        //                song->Duration = 0;
        //            if (!sf->Audio) {
        //                song->Channels = 1;
        //                song->SampleRate = 22000;
        //                song->Bits = 16;
        //            }
        //            if (!sf->Bitrate)
        //                song->Bitrate = 0;

        if (!sf->Date)
            song->Date = "1985";

        if (!sf->File)
            song->File = "unknown file";
    }

    void MPDConnector::parseCurrentSong(Song *song, const std::vector<std::string> &words, statusFields *sf) {
        switch (hash(words[0].c_str())) {
        case hash("Artist"):
            song->Artist = join(words, 1);
            sf->Artist = true;
            break;
        case hash("Album"):
            song->Album = join(words, 1);
            sf->Album = true;
            break;
        case hash("Title"):
            song->Title = join(words, 1);
            sf->Title = true;
            break;
        case hash("Date"):
            song->Date = join(words, 1);
            sf->Date = true;
            break;
        case hash("Track"):
            song->Track = join(words, 1);
            sf->Track = true;
            break;
        case hash("file"):
            song->File = join(words, 1);
            sf->File = true;
            break;
        case hash("Id"):
            try {
                song->songID = std::stoi(join(words, 1));
            } catch (...) {
                song->songID = 0;
            }
            break;
        }
    }

    void MPDConnector::parseStatus(Status *status, const std::vector<std::string> &words) {
        std::string tempState;

        switch (hash(words[0].c_str())) {
        case hash("bitrate"): {
            auto brWord = join(words, 1);
            status->sf.Bitrate = true;

            try {
                status->Bitrate = std::stoi(brWord);
            } catch (...) {
                status->Bitrate = 0;
            }

            status->BitrateString = brWord;
            if (brWord.length() > 3) {
                status->BitrateString = brWord.substr(brWord.length() - 3, 3);
            }

            break;
        }
        case hash("duration"):
            status->sf.Duration = true;
            try {
                status->Duration = std::stoi(join(words, 1));
            } catch (...) {
                status->Duration = 0;
            }

            break;
        case hash("elapsed"):
            status->sf.Elapsed = true;
            try {
                status->Elapsed = std::stoi(join(words, 1));
            } catch (...) {
                status->Elapsed = 0;
            }
            break;
        case hash("state"):
            tempState = join(words, 1);
            break;
        case hash("volume"):
            status->sf.Volume = true;
#ifdef DESKTOP
            try {
                status->Volume = std::stoi(join(words, 1));
            } catch (...) {
                status->Volume = 0;
            }
            status->VolumeRaw = status->Volume;
#endif
            break;
        case hash("audio"): {
            status->sf.Audio = true;
            auto parts = split(words[1], ":");
            try {
                status->SampleRate = std::stoi(parts[0]);
            } catch (...) {
                status->SampleRate = 0;
            }

            status->SampleRateString = std::to_string((int)status->SampleRate / 1000);
            if ((parts[0].length() > 2) && (status->SampleRateString.length() > 2)) {
                status->SampleRateString = status->SampleRateString.substr(status->SampleRateString.length() - 2, 2);
            }

            try {
                status->Bits = std::stoi(parts[1]);
            } catch (...) {
                status->Bits = 0;
            }

            try {
                status->Channels = std::stoi(parts[2]);
            } catch (...) {
                status->Channels = 1;
            }
            break;
        }
        case hash("repeat"): {
            try {
                status->Repeat = std::stoi(join(words, 1));
            } catch (...) {
                status->Repeat = 0;
            }
            break;
        }
        case hash("random"): {
            try {
                status->Shuffle = std::stoi(join(words, 1));
            } catch (...) {
                status->Shuffle = 0;
            }
            break;
        }
        case hash("song"): {
            try {
                status->songID = std::stoi(join(words, 1));
            } catch (...) {
                status->songID = 0;
            }
            break;
        }
        case hash("file"): {
            status->Codec = split(join(words, 1), ".").back();
            status->Filename = join(words, 1);
            break;
        }
        default:
            break;
        }

        if (!tempState.empty()) {
            switch (hash(tempState.c_str())) {
            case hash("play"):
                status->State = PlayStateE::PLAYING;
                break;
            case hash("pause"):
                status->State = PlayStateE::PAUSED;
                break;
            case hash("stop"):
            default:
                status->State = PlayStateE::STOPPED;
                break;
            }
        }
    }

    bool MPDConnector::parsePlaylistWord(const std::vector<std::string> &words, Song *song) {
        switch (hash(words[0].c_str())) {
        case hash("Artist"):
            song->Artist = join(words, 1);
            break;
        case hash("Album"):
            song->Album = join(words, 1);
            break;
        case hash("Title"):
            song->Title = join(words, 1);
            break;
        case hash("Track"):
            song->Track = join(words, 1);
            break;
        case hash("Date"):
            song->Date = join(words, 1);
            break;
        case hash("file"):
            song->File = join(words, 1);
            break;
        case hash("duration"):
            try {
                song->Duration = std::stoi(join(words, 1));
            } catch (...) {
                song->Duration = 0;
            }
            break;
        case hash("Id"): {
            return true;
        }
        }

        return false;
    }

    void MPDConnector::parseResponse(char *buf, int BUF_SIZE, Status *status) {
        if (buf == nullptr) {
            DLOG("cannot parse response, buf is null\n");
            return;
        }

        auto s = std::string(buf, BUF_SIZE);

        if (s.substr(0, 9) != "OK\nvolume") {
            return;
        }
        //            statusFields sf = statusFields{};

        auto lines = split(s, lineDelimeter);
        // remove OK from `noidle` command
        if (lines[0] == "OK") {
            lines.erase(lines.begin());
        }

        for (const auto &line : lines) {
            auto words = split(line, fieldDelimeter);
            if (words.empty()) {
                continue;
            }

            // end of status
            if (words[0] == "OK") {
                break;
            }

            parseStatus(status, words);
        }

        memset(buf, 0, BUF_SIZE);
    }

    void MPDConnector::parsePlaylist(char *buf, int BUF_SIZE, std::vector<Song> *playlist) {
        if (buf == nullptr) {
            DLOG("cannot parse response, buf is null\n");
            return;
        }

        auto s = std::string(buf, BUF_SIZE);

        if (s.substr(0, 7) != "OK\nfile") {
            return;
        }

        //        status.busy = true;

        for (auto &ss : *playlist) {
            ss.Reset();
        }

        auto lines = split(s, lineDelimeter);
        int playlistIndex = 0;
        bool endOfSong;

        for (const auto &line : lines) {
            auto words = split(line, fieldDelimeter);
            if (words.empty()) {
                continue;
            }

            endOfSong = parsePlaylistWord(words, &playlist->at(playlistIndex));
            if (endOfSong) {
                playlistIndex++;
                if (playlistIndex > playlist->size() - 1) {
                    break;
                }
            }
        }
        //        status.busy = false;

        memset(buf, 0, BUF_SIZE);
    }

    __attribute__((unused)) void MPDConnector::volumeLoop(){};

    void MPDConnector::powerLoop(bool *render, bool *power){};

    void MPDConnector::TestCommand(){};

    void MPDConnector::ToggleHgrm(HgrmToggleAction action, bool *render){};

    void MPDConnector::ReadLoop() {
        char buf[RESP_BUF_SIZE];
        int numbytes;

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            Send(commandNoIdle);
            Send(commandStatus);
            Send(commandIdle);

            if ((numbytes = recv(fd, buf, RESP_BUF_SIZE, 0)) == -1) {
                DLOG("mpd failed receive\n");
                exit(1);
            }

            // "changed" event emitted by mpd, ignore
            int i;
            for (i = 0; i < strlen(changed); i++) {
                if (changed[i] != buf[i]) {
                    break;
                }
            }

            if (i == (strlen(changed))) {
                continue;
            }

            if (numbytes > 0) {
                parseResponse(buf, RESP_BUF_SIZE, &status);
                //                    resetEmptyFields(&playlist.at(0), &status.sf);
            } else {
                continue;
            }

            memset(buf, 0, RESP_BUF_SIZE);

            char commandPlaylist[50];
            sprintf(commandPlaylist, "playlistinfo %d:%d\n", status.songID, status.songID + PLAYLIST_SIZE);
            Send(commandNoIdle);
            Send(commandPlaylist);
            Send(commandIdle);

            if ((numbytes = recv(fd, buf, RESP_BUF_SIZE, 0)) == -1) {
                DLOG("mpd failed receive\n");
                exit(1);
            }

            if (numbytes > 0) {
                parsePlaylist(buf, RESP_BUF_SIZE, &playlist);
                status.Date = playlist.at(0).Date;
                status.TrackNumber = playlist.at(0).Track;
            }

            for (const auto &client : clients) {
                if (client->active) {
                    client->Notify();
                }
            }
        }
    }

    void MPDConnector::PollStatus() {}

    void MPDConnector::SetVolume(int i, bool relative) {
        char c[14];
        int vol = 0;
        if (relative) {
            vol = status.Volume + i;
        } else {
            vol = i;
        }
        sprintf(c, "setvol %d\n", vol);
        Send(commandNoIdle);
        Send(c);
        Send(commandIdle);
    }

    void MPDConnector::SetPosition(int i) {
        char c[14];
        int newpos = (status.Duration * i / 100);
        sprintf(c, "seekcur %d\n", newpos);
        Send(commandNoIdle);
        Send(c);
        Send(commandIdle);
    }

    void MPDConnector::SetBalance(int i) { DLOG("not implemented\n"); }

    void MPDConnector::SetShuffle(int i) {
        char c[10];
        sprintf(c, "random %d\n", i);
        Send(commandNoIdle);
        Send(c);
        Send(commandIdle);
    }

    void MPDConnector::SetRepeat(int i) {
        char c[10];
        if (i == 0) {
            i = 1;
        } else {
            i = 0;
        }
        sprintf(c, "repeat %d\n", i);
        Send(commandNoIdle);
        Send(c);
        Send(commandIdle);
    }

    void MPDConnector::PrevTrack() {
        Send(commandNoIdle);
        Send("play\nprevious\n");
        Send(commandIdle);
    }

    void MPDConnector::Play() {
        if (status.State == PlayStateE::PAUSED) {
            Send(commandNoIdle);
            Send("pause 0\n");
            Send(commandIdle);
            return;
        }

        if (status.State == PlayStateE::PLAYING) {
            Send(commandNoIdle);
            Send("seekcur 0\n");
            Send(commandIdle);
            return;
        }

        if (status.State == PlayStateE::STOPPED) {
            Send(commandNoIdle);
            Send("play\n");
            Send(commandIdle);
            return;
        }
    }

    void MPDConnector::Pause() {
        if (status.State == PlayStateE::PAUSED) {
            Send(commandNoIdle);
            Send("pause 0\n");
            Send(commandIdle);
            return;
        }

        if (status.State == PlayStateE::PLAYING) {
            Send(commandNoIdle);
            Send("pause 1\n");
            Send(commandIdle);
            return;
        }
    }

    void MPDConnector::Stop() {
        Send(commandNoIdle);
        Send("stop\n");
        Send(commandIdle);
    }

    void MPDConnector::NextTrack() {
        Send(commandNoIdle);
        Send("play\nnext\n");
        Send(commandIdle);
    }
} // namespace MPD