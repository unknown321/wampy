#ifndef WAMPY_CASSETTE_CONFIG_H
#define WAMPY_CASSETTE_CONFIG_H

#include "map"
#include "string"
#include "tape_type.h"

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
} // namespace Cassette
#endif
