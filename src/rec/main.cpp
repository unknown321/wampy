#include "mp3.h"
#include "rec.h"
#include "util/dlog.h"
#include "wav.h"

#include <alsa/asoundlib.h>
#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <thread>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <mp3/wav> <seconds>\n", argv[0]);
        exit(1);
    }

    constexpr unsigned int sample_rate = 44100;
    constexpr snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    constexpr unsigned int channels = 2;
    snd_pcm_t *capture_handle;
    snd_pcm_t **hh = &capture_handle;
#ifdef DESKTOP
    const char *dev_name = "hw:0,0";
    const char *path = "/tmp/out.wav";
    const char *mp3path = "/tmp/out.mp3";
#else
    const char *dev_name = "hw:0,1";
    const char *path = "/contents/out.wav";
    const char *mp3path = "/contents/out.mp3";
#endif
    if (prepare(hh, dev_name, sample_rate, format, channels) < 0) {
        exit(1);
    }

    bool stop = false;
    char *s_end{};

    auto seconds = std::strtol(argv[2], &s_end, 10);
    if (seconds <= 0) {
        DLOG("invalid seconds %s\n", argv[2]);
        exit(1);
    }
    const bool range_error = errno == ERANGE;
    if (range_error) {
        DLOG("range error\n");
        exit(1);
    }

    auto wav = [&seconds, &stop]() {
        for (;;) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            seconds--;
            DLOG("tick %ld\n", seconds);
            if (seconds <= 0) {
                stop = true;
                DLOG("stopping\n");
                break;
            }
        }
    };
    std::thread t(wav);
    t.detach();

    DLOG("recording %s\n", argv[1]);
    if (strstr(argv[1], "wav")) {
        FILE *f = fopen(path, "w+");
        if (record_wav(capture_handle, sample_rate, channels, format, f, &stop) < 0) {
            exit(1);
        }
    } else {
        FILE *f = fopen(mp3path, "w+");
        if (record_mp3(capture_handle, sample_rate, channels, format, 128, f, &stop) < 0) {
            exit(1);
        }
    }

    snd_pcm_close(capture_handle);

    exit(0);
}