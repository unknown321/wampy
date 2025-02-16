#ifndef WAMPY_W1_H
#define WAMPY_W1_H

#include "map"
#include "string"
#include <vector>

namespace W1 {

    struct W1Options {
        unsigned int deviceColor{};
    };

    struct WalkmanOneOptions {
        unsigned int signature;       // SIG
        char region[4];               // REG
        bool remote;                  // REM
        unsigned int plusModeVersion; // PMV, save as unsigned int 1/2
        bool plusModeVersionBOOL; // config file suggests that you can select v1 or v2. boot script has 4 possible values (normal+normal_nt)
        bool plusModeByDefault;   // PMD
        bool gainMode;            // GMD
        bool dacInitializationMode; // DIM
        unsigned int color;         // COL

        bool configFound;
        bool tuningChanged;

        void Save();
        void Reboot() const;
        void ApplyTuning() const;
    };

    int ParseSettings(WalkmanOneOptions *w);

    extern unsigned int defaultColor;
    extern std::map<std::string, unsigned int> colorByName;
    extern std::map<unsigned int, std::string> colorByValue;
    extern std::map<std::string, unsigned int> colorByNameWalkmanOne;
    extern std::map<unsigned int, std::string> colorByValueWalkmanOne;
    extern std::map<std::string, unsigned int> signatureByNameWalkmanOne;
    extern std::map<unsigned int, std::string> signatureByValueWalkmanOne;
    extern std::vector<std::string> regionWalkmanOne;
    extern std::map<unsigned int, std::string> signatureToPathWalkmanOne;

    void SetColor(unsigned int color);

} // namespace W1

#endif // WAMPY_W1_H
