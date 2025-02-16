#ifndef WAMPY_TAPE_TYPE_H
#define WAMPY_TAPE_TYPE_H

namespace Tape {
    enum TapeType {
        MP3_128 = 0,
        MP3_160,
        MP3_256,
        MP3_320,
        FLAC_ALAC_APE_MQA,
        AIFF,
        PCM,
        FLAC_MQA_ALAC_PCM_AIFF_APE_HIRES,
        DSD,
        TAPE_OTHER,
    };
}

#endif // WAMPY_TAPE_TYPE_H
