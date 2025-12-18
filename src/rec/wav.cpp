#include "wav.h"
#include "util/dlog.h"
#include <alsa/asoundlib.h>
#include <cstring>

void fill_wav_header(wav_header_t *header, uint32_t sample_rate, uint16_t num_channels, uint16_t bits_per_sample, uint32_t data_size) {
    // RIFF chunk
    memcpy(header->riff, "RIFF", 4);
    header->file_size = data_size;
    memcpy(header->wave, "WAVE", 4);

    // Format chunk
    memcpy(header->fmt, "fmt ", 4);
    header->chunk_size = 16;
    header->audio_format = 1; // PCM
    header->num_channels = num_channels;
    header->sample_rate = sample_rate;
    // Calculate derived values
    header->byte_rate = sample_rate * num_channels * (bits_per_sample / 8);
    header->block_align = num_channels * (bits_per_sample / 8);
    header->bits_per_sample = bits_per_sample;

    // Data chunk
    memcpy(header->data, "data", 4);
    header->data_size = data_size;
}

int record_wav(snd_pcm_t *capture_handle, const uint32_t sample_rate, const unsigned int channels, const snd_pcm_format_t format, FILE *f, const bool *stop) {
    constexpr snd_pcm_uframes_t buffer_frames = 128;
    const size_t bsize = buffer_frames * snd_pcm_format_width(format) * channels / 8;
    void *buffer = malloc(bsize);

    wav_header_t header;
    fill_wav_header(&header, sample_rate, channels, snd_pcm_format_width(format), 0);
    fwrite(&header, sizeof(header), 1, f);

    uint32_t datasize = 0;
    for (;;) {
        snd_pcm_sframes_t res;
        if ((res = snd_pcm_readi(capture_handle, buffer, buffer_frames)) != buffer_frames) {
            DLOG("read from audio interface failed (%ld) %s\n", res, snd_strerror(static_cast<int>(res)));
            return static_cast<int>(res);
        }

        fwrite(buffer, 1, bsize, f);
        datasize += bsize;
        if (*stop) {
            break;
        }
    }

    fseek(f, 0, SEEK_SET);
    fill_wav_header(&header, sample_rate, channels, snd_pcm_format_width(format), datasize);
    fwrite(&header, sizeof(header), 1, f);

    fclose(f);
    free(buffer);

    return 0;
}
