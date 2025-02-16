#ifndef WAMPY_TAPE_H
#define WAMPY_TAPE_H

#include "../connector/connector.h"
#include "../skinElement.h"
#include "imgui.h"
#include <string>

namespace Tape {
    enum Errors {
        ERR_NO_FILES = INT_MIN,
        ERR_OK,
    };

    typedef std::vector<FlatTexture> Reel;

    struct TapeElements {
        FlatTexture Main;
        Button ToggleSettings;
        Button RandomizeTape;
    };

    struct Tape {
        TapeElements Elements;
        void *skin{};
        Connector *connector;
        char *artist;
        char *title;
        char *album;
        ImVec2 titleCoords{83.0f, 117.0f};
        ImVec2 artistCoords{83.0f, 82.0f};
        ImVec2 reelCoords{134.0f, 160.0f};
        ImVec2 albumCoords{-1.0f, -1.0f};
        std::string reel = "other";
        std::string name{};
        std::string formatArtist = "$ARTIST";
        std::string formatTitle = "$TITLE";
        std::string formatAlbum = "$ALBUM";
        std::string formatDuration = "%1$02d:%2$02d";
        float titleWidth = 600.0f;
        ImU32 textColor = IM_COL32_BLACK;
        bool valid{};

        int Load(const std::string &directoryPath);

        int LoadFromJPEGS(const std::string &directoryPath);

        void Draw();

        void DrawSongInfo() const;

        int InitializeButtons();

        void ParseConfig(const std::string &text);

        void Unload();
    };

} // namespace Tape
#endif // WAMPY_TAPE_H
