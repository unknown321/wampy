#ifndef WAMPY_MP3_H
#define WAMPY_MP3_H
#include <alsa/asoundlib.h>

int record_mp3(snd_pcm_t *capture_handle, int sample_rate, unsigned int channels, snd_pcm_format_t format, int bitrate, FILE *f, const bool *stop);

#endif // WAMPY_MP3_H
