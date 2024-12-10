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

    typedef std::vector<FlatTexture> Reel;

    struct TapeElements {
        FlatTexture Main;
        Button ToggleSettings;
        //        ImVec2 PosArtist;
        //        ImVec2 PosText;
        //        ImVec2 PosReel;
    };

    struct Tape {
        TapeElements Elements;
        void *skin{};
        Connector *connector;
        char *artist;
        char *title;
        ImVec2 titleCoords{83.0f, 117.0f};
        ImVec2 artistCoords{83.0f, 82.0f};
        ImVec2 reelCoords{134.0f, 160.0f};
        std::string reel = "other";
        std::string name{};
        float titleWidth = 600.0f;
        ImU32 textColor = IM_COL32_BLACK;
        bool valid{};

        int Load(const std::string &directoryPath);

        void Draw();

        void DrawSongInfo() const;

        int InitializeButtons();

        void ParseConfig(const std::string &text);

        void Unload();
    };

} // namespace Tape
#endif // WAMPY_TAPE_H
