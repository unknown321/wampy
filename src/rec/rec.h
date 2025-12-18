#ifndef WAMPY_REC_H
#define WAMPY_REC_H

#include <alsa/asoundlib.h>

enum class RecordCodec {
    MP3,
    WAV,
};

enum class RecordStorage {
    INTERNAL,
    SD_CARD,
};

int prepare(snd_pcm_t **capture_handle, const char *dev_name, unsigned int sample_rate, snd_pcm_format_t format, unsigned int channels);

void start_rec(const RecordCodec codec, RecordStorage storage, bool *stop);

#endif // WAMPY_REC_H
