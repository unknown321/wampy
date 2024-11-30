#include "song.h"

void Song::Reset() {
    Artist.clear();
    Album.clear();
    Title.clear();
    Track.clear();
    Date.clear();
    File.clear();
    PlaylistDurationTextSize = 0.0f;
    PlaylistDuration.clear();
    PlaylistTitle.clear();
    Duration = -2;
    PlaylistStringsCalculated = false;
}
