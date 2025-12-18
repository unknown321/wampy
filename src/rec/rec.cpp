#include "rec.h"

#include "mkpath.h"
#include "mp3.h"
#include "util/dlog.h"
#include "wav.h"

#include <alsa/asoundlib.h>
#include <string>
#include <thread>

int prepare(snd_pcm_t **capture_handle, const char *dev_name, unsigned int sample_rate, const snd_pcm_format_t format, const unsigned int channels) {
    int err;
    snd_pcm_hw_params_t *hw_params;

    if ((err = snd_pcm_open(capture_handle, dev_name, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        DLOG("snd_pcm_open: %s\n", snd_strerror(err));
        return err;
    }

    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
        DLOG("cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
        return err;
    }

    if ((err = snd_pcm_hw_params_any(*capture_handle, hw_params)) < 0) {
        DLOG("cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
        return err;
    }

    if ((err = snd_pcm_hw_params_set_access(*capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        DLOG("cannot set access type (%s)\n", snd_strerror(err));
        return err;
    }

    if ((err = snd_pcm_hw_params_set_format(*capture_handle, hw_params, format)) < 0) {
        DLOG("cannot set sample format (%s)\n", snd_strerror(err));
        return err;
    }

    if ((err = snd_pcm_hw_params_set_rate_near(*capture_handle, hw_params, &sample_rate, nullptr)) < 0) {
        DLOG("cannot set sample rate (%s)\n", snd_strerror(err));
        return err;
    }

    if ((err = snd_pcm_hw_params_set_channels(*capture_handle, hw_params, channels)) < 0) {
        DLOG("cannot set channel count (%s)\n", snd_strerror(err));
        return err;
    }

    if ((err = snd_pcm_hw_params(*capture_handle, hw_params)) < 0) {
        DLOG("cannot set parameters (%s)\n", snd_strerror(err));
        return err;
    }

    snd_pcm_hw_params_free(hw_params);

    if ((err = snd_pcm_prepare(*capture_handle)) < 0) {
        DLOG("cannot prepare audio interface for use (%s)\n", snd_strerror(err));
        return err;
    }

    return 0;
}

void start_rec(const RecordCodec codec, RecordStorage storage, bool *stop) {
    constexpr unsigned int sample_rate = 44100;
    constexpr snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    constexpr unsigned int channels = 2;
    snd_pcm_t *capture_handle;
    snd_pcm_t **hh = &capture_handle;

#ifdef DESKTOP
    const char *dev_name = "hw:0,0";
    std::string path = "/tmp/out.wav";
    std::string mp3path = "/tmp/out.mp3";
#else
    const char *dev_name = "hw:0,1";
    std::string prefix = "/contents";
    if (storage == RecordStorage::SD_CARD) {
        prefix += "_ext";
    }
    mkpath((prefix + "/MUSIC/RECORDINGS/").c_str(), 0755);

    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H.%M.%S", timeinfo);
    auto tn = std::string(buffer);
    std::string path = prefix + "/MUSIC/RECORDINGS/" + tn + ".wav";
    std::string mp3path = prefix + "/MUSIC/RECORDINGS/" + tn + ".mp3";
#endif
    if (prepare(hh, dev_name, sample_rate, format, channels) < 0) {
        exit(1);
    }

    auto rf = [codec, path, capture_handle, stop, mp3path]() {
        if (codec == RecordCodec::WAV) {
            FILE *f = fopen(path.c_str(), "w+");
            DLOG("recording to %s\n", path.c_str());
            if (record_wav(capture_handle, sample_rate, channels, format, f, stop) < 0) {
                DLOG("record wav failed\n");
                return;
            }
        } else {
            FILE *f = fopen(mp3path.c_str(), "w+");
            DLOG("recording to %s\n", mp3path.c_str());
            if (record_mp3(capture_handle, sample_rate, channels, format, 320, f, stop) < 0) {
                DLOG("record mp3 failed\n");
                return;
            }
        }

        snd_pcm_close(capture_handle);
        if (codec == RecordCodec::WAV) {
            DLOG("record finished, path %s\n", path.c_str());
        } else {
            DLOG("record finished, path %s\n", mp3path.c_str());
        }
    };

    std::thread t(rf);
    t.detach();
}

void stop_rec(bool *stop) { *stop = true; }