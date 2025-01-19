#ifndef WAMPY_HAGOROMOSTATUS_H
#define WAMPY_HAGOROMOSTATUS_H

#include <semaphore.h>
#define HAGOROMO_STATUS_SHM_PATH "/hagoromo_status"
#define PLAYLIST_TRACK_FIELD_SIZE 512
#define PLAYLIST_LENGTH 20

struct Track {
    char Artist[PLAYLIST_TRACK_FIELD_SIZE]{0};
    char Title[PLAYLIST_TRACK_FIELD_SIZE]{0};
    int TrackNumber = 0;
    int Duration = 0;
    bool Active = false;
};

enum PlayStateE {
    UNKNOWN = 0,
    PLAYING = 1,
    PAUSED = 2,
    STOPPED = 3,
};

struct HagoromoStatus {
    sem_t sem1{}; /* POSIX unnamed semaphore */
    int entryId{};
    int prevEntryId{};
    int shuffleOn{};
    int repeatMode{};
    int elapsed{};
    PlayStateE playState{};
    int volume{};
    int volumeRaw{};

    Track playlist[PLAYLIST_LENGTH]{};
};

#endif // WAMPY_HAGOROMOSTATUS_H
