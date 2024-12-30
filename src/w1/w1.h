#ifndef WAMPY_W1_H
#define WAMPY_W1_H

#include "../util/util.h"
#include "map"

namespace W1 {

    struct W1Options {
        uint deviceColor{};
    };

    extern uint defaultColor;
    extern std::map<std::string, uint> colorByName;
    extern std::map<uint, std::string> colorByValue;

    void SetColor(uint color);

} // namespace W1

#endif // WAMPY_W1_H
