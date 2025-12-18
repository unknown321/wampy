#ifndef WAMPY_WAV_H
#define WAMPY_WAV_H

#include <alsa/asoundlib.h>
#include <cstdint>

#pragma pack(push, 1)
typedef struct {
    // RIFF chunk
    char riff[4];       // "RIFF"
    uint32_t file_size; // Total file size - 8
    char wave[4];       // "WAVE"

    // Format chunk
    char fmt[4];              // "fmt "
    uint32_t chunk_size;      // Format chunk size (16 for PCM)
    uint16_t audio_format;    // Audio format (1 = PCM)
    uint16_t num_channels;    // Number of channels
    uint32_t sample_rate;     // Sample rate (e.g., 44100)
    uint32_t byte_rate;       // Bytes per second
    uint16_t block_align;     // Bytes per sample frame
    uint16_t bits_per_sample; // Bits per sample

    // Data chunk
    char data[4];       // "data"
    uint32_t data_size; // Size of audio data in bytes
} wav_header_t;
#pragma pack(pop)

void fill_wav_header(wav_header_t *header, uint32_t sample_rate, uint16_t num_channels, uint16_t bits_per_sample, uint32_t data_size);

int record_wav(snd_pcm_t *capture_handle, uint32_t sample_rate, unsigned int channels, snd_pcm_format_t format, FILE *f, const bool *stop);

#endif // WAMPY_WAV_H
