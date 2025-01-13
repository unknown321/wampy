#ifndef WAMPY_DAC_H
#define WAMPY_DAC_H

#include "../util/util.h"
#include "cxd3778gf_table.h"
#include "map"
#include "string"
#include "vector"

namespace Dac {
    extern std::map<int, std::string> TableTypeToString;
    extern std::map<int, std::string> MasterVolumeTableTypeToString;
    extern std::map<int, std::string> MasterVolumeValueTypeToString;
    extern std::map<int, std::string> ToneControlTableTypeToString;
    extern std::string volumeTableOutPath;
    extern std::string volumeTableDSDOutPath;
    extern std::string toneControlOutPath;

    void printTableValue(master_volume *t, MASTER_VOLUME_VALUE valType, const std::string &outfile);

    std::string getStatus(std::vector<directoryEntry> *volumeTableFiles, const std::string &outPath);
} // namespace Dac

#endif // WAMPY_DAC_H
