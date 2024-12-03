#ifndef WAMPY_CASSETTE_H
#define WAMPY_CASSETTE_H

#include "../skinElement.h"
#include "../skinVariant.h"
#include "tape.h"
#include <thread>

namespace Cassette {
    struct ConfigEntry {
        std::string tape;
        std::string reel;
        std::string name;
    };

    typedef std::map<Tape::TapeType, ConfigEntry> configInternal;

    class Config {
      public:
        Config() = default;

        configInternal data{};
        bool randomize{};

        ConfigEntry *Get(const Tape::TapeType t) { return &data.at(t); };

        void Set(const Tape::TapeType t, ConfigEntry v) { data[t] = std::move(v); };

        void Default();

        static std::map<Tape::TapeType, ConfigEntry> GetDefault();

        void SetOrDefault(const Tape::TapeType t, ConfigEntry e) {
            auto def = GetDefault();
            if (e.name.empty()) {
                e.name = def[t].name;
            }

            if (e.reel.empty()) {
                e.reel = def[t].reel;
            }

            if (e.tape.empty()) {
                e.tape = def[t].tape;
            }

            Set(t, e);
        }
    };

    class Cassette : public SkinVariant {
      public:
        Cassette() = default;

        SkinList *reelList{};
        SkinList *tapeList{};

        Config *config{};

        void Draw() override;

        int Load(std::string filename, ImFont *FontRegular) override;

        void WithConfig(Config *c);

        void Unload();

        void UnloadUnused();

        int LoadReel(const std::string &path);

        int LoadTape(const std::string &path);

        void LoadImages();

        int AddFonts(ImFont *fontRegular);

        void SelectTape(bool force = false);

      private:
        std::map<std::string, Tape::Tape> Tapes;
        std::map<std::string, Tape::Reel> Reels;
        int reelID = 0;

        std::string Track; // prevents refreshing bitrate value for vbr tracks (mpd)
        Song song;
        Tape::TapeType tapeType = Tape::MP3_320;
        Tape::Tape *ActiveTape = nullptr;
        Tape::Reel *ActiveReel = nullptr;

        bool reelThreadStop{};

        void drawCodecInfo() const;

        void validateConfig();

        void ReelThread();

        void ReelLoop();

        void randomizeTape();

        void defaultTape();
    };
} // namespace Cassette
#endif // WAMPY_CASSETTE_H