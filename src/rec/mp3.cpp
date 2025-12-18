#include "mp3.h"
extern "C" {
#include "shine/layer3.h"
}
#include "util/dlog.h"
#include <alsa/asoundlib.h>

int record_mp3(snd_pcm_t *capture_handle, const int sample_rate, const unsigned int channels, const snd_pcm_format_t format, const int bitrate, FILE *f,
    const bool *stop) {
    shine_config_t config;
    config.wave.samplerate = sample_rate;
    config.wave.channels = PCM_STEREO;
    shine_set_config_mpeg_defaults(&config.mpeg);
    config.mpeg.bitr = bitrate;
    config.mpeg.mode = STEREO;

    int err;
    if ((err = shine_check_config(config.wave.samplerate, config.mpeg.bitr)) < 0) {
        DLOG("Unsupported samplerate/bitrate configuration.");
        return err;
    }

    const auto s = shine_initialise(&config);

    snd_pcm_uframes_t buffer_frames = SHINE_MAX_SAMPLES;
    const size_t bsize = buffer_frames * snd_pcm_format_width(format) * channels / 8;
    void *buffer = malloc(bsize);

    int written = 0;
    int16_t shine_buf[2 * SHINE_MAX_SAMPLES];

    for (;;) {
        snd_pcm_sframes_t res;
        if ((res = snd_pcm_readi(capture_handle, buffer, buffer_frames)) != buffer_frames) {
            DLOG("read from audio interface failed (%ld) %s\n", res, snd_strerror(static_cast<int>(res)));
            return static_cast<int>(res);
        }

        memset(shine_buf, 0, sizeof(shine_buf));
        memcpy(shine_buf, buffer, bsize);

        const auto data = shine_encode_buffer_interleaved(s, shine_buf, &written);
        fwrite(data, 1, written, f);

        if (*stop) {
            break;
        }
    }

    const auto data = shine_flush(s, &written);
    fwrite(data, written, 1, f);
    shine_close(s);

    fclose(f);
    free(buffer);

    return 0;
}