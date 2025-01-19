#ifndef WAMPY_CASSETTE_H
#define WAMPY_CASSETTE_H

#include "../skinElement.h"
#include "../skinVariant.h"
#include "tape.h"
#include <thread>

#define FIELD_SIZE 2048

#define REEL_DELAY_MS 55;

namespace Cassette {
    const float fontSizeTTF = 34.0f;

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

    struct AtlasConfig {
        int delayMS = REEL_DELAY_MS;
    };

    struct ReelAtlas {
        Atlas atlas;
        AtlasConfig config;
    };

    class Cassette : public SkinVariant, public INotifiable {
      public:
        Cassette() = default;

        Cassette(Cassette const &other);
        Cassette &operator=(Cassette const &other);

        SkinList *reelList{};
        SkinList *tapeList{};

        Config *config{};

        void Draw() override;

        int Load(std::string filename, ImFont **FontRegular) override;

        void WithConfig(Config *c);

        void Unload();

        void UnloadUnused();

        int LoadReel(const std::string &path);

        int LoadReelAtlas(const std::string &path);

        static AtlasConfig LoadReelAtlasConfig(const std::string &path);

        int LoadTape(const std::string &path);

        void LoadImages();

        int AddFonts(ImFont **fontRegular);

        void SelectTape();

        void Notify() override;

      private:
        std::map<std::string, Tape::Tape> Tapes;
        std::map<std::string, Tape::Reel> Reels;
        std::map<std::string, ReelAtlas> ReelsAtlas;
        int reelIndex = 0; // TODO mutex

        char previousTrack[FIELD_SIZE]{}; // used to check if song changed
        Tape::TapeType tapeType = Tape::MP3_320;
        Tape::Tape *ActiveTape = nullptr;
        Tape::Reel *ActiveReel = nullptr;
        ReelAtlas *ActiveAtlas = nullptr;

        char artist[FIELD_SIZE]{};
        char title[FIELD_SIZE]{};
        char album[FIELD_SIZE]{};

        bool childThreadsStop{};
        bool reelThreadRunning{};
        bool updateThreadRunning{};

        std::mutex statusUpdatedM;
        bool statusUpdated{}; // set when connector sends an update notification

        void drawCodecInfo() const;

        void validateConfig();

        void ReelLoop();

        void randomizeTape();

        void defaultTape();

        void format();

        void processUpdate();

        void changed(bool *changed);
    };
} // namespace Cassette
#endif // WAMPY_CASSETTE_H