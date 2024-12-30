#ifndef WAMPY_W1_H
#define WAMPY_W1_H

#include "../util/util.h"
#include "map"

namespace W1 {

    struct W1Options {
        uint deviceColor{};
    };

    struct WalkmanOneOptions {
        uint signature;           // SIG
        char region[4];           // REG
        bool remote;              // REM
        uint plusModeVersion;     // PMV, save as uint 1/2
        bool plusModeVersionBOOL; // config file suggests that you can select v1 or v2. boot script has 4 possible values (normal+normal_nt)
        bool plusModeByDefault;   // PMD
        bool gainMode;            // GMD
        bool dacInitializationMode; // DIM
        uint color;                 // COL

        bool configFound;
        bool tuningChanged;

        void Save();
        void Reboot() const;
        void ApplyTuning() const;
    };

    int ParseSettings(WalkmanOneOptions *w);

    extern uint defaultColor;
    extern std::map<std::string, uint> colorByName;
    extern std::map<uint, std::string> colorByValue;
    extern std::map<std::string, uint> colorByNameWalkmanOne;
    extern std::map<uint, std::string> colorByValueWalkmanOne;
    extern std::map<std::string, uint> signatureByNameWalkmanOne;
    extern std::map<uint, std::string> signatureByValueWalkmanOne;
    extern std::vector<std::string> regionWalkmanOne;
    extern std::map<uint, std::string> signatureToPathWalkmanOne;

    void SetColor(uint color);

} // namespace W1

#endif // WAMPY_W1_H
