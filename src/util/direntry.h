#ifndef WAMPY_DIRENTRY_H
#define WAMPY_DIRENTRY_H

#include "string"

struct directoryEntry {
    std::string fullPath{};
    std::string name{};
    bool valid = true;
};
#endif // WAMPY_DIRENTRY_H
