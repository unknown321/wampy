#ifndef SONG_H
#define SONG_H

#include <atomic>
#include <string>

struct DurationDisplay {
    int Minute1 = 0;
    int Minute2 = 0;
    int Second1 = 0;
    int Second2 = 0;
};

struct Song {
    std::string Artist{};
    std::string Album{};
    std::string Title{};
    std::string Track{};
    std::string Date{};
    std::string File{};
    int Duration = -1;
    DurationDisplay DurDisplay{};
    std::string TitleFormatted{};
    std::string TitleMarquee{};
    std::string ArtistFormatted{};
    std::string PlaylistTitle{};
    std::string PlaylistDuration{};
    bool PlaylistStringsCalculated{};
    float PlaylistDurationTextSize{};
    int songID{};

    void Reset();
};

#endif